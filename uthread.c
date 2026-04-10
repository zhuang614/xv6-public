#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

// TODO: Implement cooperative user-level threads.

enum thread_state {
	T_FREE = 0,
	T_RUNNABLE,
	T_RUNNING,
	T_ZOMBIE,
};

struct context {
	uint edi;
	uint esi;
	uint ebx;
	uint ebp;
	uint eip;
};

struct thread {
	tid_t tid;
	enum thread_state state;
	void *stack;
	struct context *context;
	void (*fn)(void *);
	void *arg;
};

#define NTHREAD 16
#define STACK_SIZE 4096

static struct thread threads[NTHREAD];
static struct thread *current_thread;
static tid_t next_tid = 1;

static void
thread_stub(void)
{
	current_thread->fn(current_thread->arg);
	current_thread->state = T_ZOMBIE;
	thread_yield();
	exit();
}

static struct thread*
pick_next(void)
{
	int i;
	int start;

	if(current_thread == 0)
		return 0;

	start = (int)(current_thread - threads);
	for(i = 1; i <= NTHREAD; i++){
		struct thread *t;
		t = &threads[(start + i) % NTHREAD];
		if(t->state == T_RUNNABLE)
			return t;
	}

	if(current_thread->state == T_RUNNING)
		return current_thread;
	return 0;
}

void thread_init(void){
	int i;

	for(i = 0; i < NTHREAD; i++){
		threads[i].tid = -1;
		threads[i].state = T_FREE;
		threads[i].stack = 0;
		threads[i].context = 0;
		threads[i].fn = 0;
		threads[i].arg = 0;
	}

	threads[0].tid = 0;
	threads[0].state = T_RUNNING;
	current_thread = &threads[0];
	next_tid = 1;
}

tid_t thread_create(void (*fn)(void*), void *arg){
	int i;

	if(current_thread == 0)
		thread_init();

	for(i = 0; i < NTHREAD; i++){
		char *sp;
		uint *stk;
		struct thread *t;

		t = &threads[i];
		if(t->state != T_FREE)
			continue;

		t->stack = malloc(STACK_SIZE);
		if(t->stack == 0)
			return -1;

		sp = (char*)t->stack + STACK_SIZE;
		sp -= 5 * sizeof(uint);
		stk = (uint*)sp;
		stk[0] = 0;
		stk[1] = 0;
		stk[2] = 0;
		stk[3] = 0;
		stk[4] = (uint)thread_stub;

		t->tid = next_tid++;
		t->state = T_RUNNABLE;
		t->context = (struct context*)sp;
		t->fn = fn;
		t->arg = arg;
		return t->tid;
	}

	return -1;
}

void thread_yield(void){
	struct thread *old;
	struct thread *next;

	if(current_thread == 0)
		return;

	old = current_thread;
	next = pick_next();
	if(next == 0 || next == old)
		return;

	if(old->state == T_RUNNING)
		old->state = T_RUNNABLE;

	next->state = T_RUNNING;
	current_thread = next;
	uswtch(&old->context, next->context);
}

int thread_join(tid_t tid){
	int i;
	struct thread *t;

	if(current_thread == 0)
		return -1;

	t = 0;
	for(i = 0; i < NTHREAD; i++){
		if(threads[i].tid == tid && threads[i].state != T_FREE){
			t = &threads[i];
			break;
		}
	}
	if(t == 0 || t == current_thread)
		return -1;

	while(t->state != T_ZOMBIE)
		thread_yield();

	free(t->stack);
	t->stack = 0;
	t->context = 0;
	t->fn = 0;
	t->arg = 0;
	t->tid = -1;
	t->state = T_FREE;
	return 0;
}
