#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

// TODO: Implement cooperative user-level threads.

void thread_init(void){}
tid_t thread_create(void (*fn)(void*), void *arg){ (void)fn; (void)arg; return -1; }
void thread_yield(void){}
int thread_join(tid_t tid){ (void)tid; return -1; }
