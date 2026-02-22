#include "types.h"
#include "stat.h"
#include "user.h"

#define TEST_DURATION 1000  // Run for 1000 ticks (~10 seconds)
#define WORK_UNIT 100       // Iterations per work unit

int main(int argc, char *argv[]) {
  int tickets[] = {10, 20, 30};
  int i, pid;
  int pipes[3][2];  // 3 pipes for 3 children
  int work_counts[3];

  printf(1, "Starting Lottery Test: 3 children with 10, 20, 30 tickets.\n");
  printf(1, "Each child will run for %d ticks and count work units completed.\n\n", TEST_DURATION);

  // Create pipes for each child
  for (i = 0; i < 3; i++) {
    if (pipe(pipes[i]) < 0) {
      printf(1, "Pipe creation failed\n");
      exit();
    }
  }

  for (i = 0; i < 3; i++) {
    pid = fork();
    if (pid < 0) {
      printf(1, "Fork failed\n");
      exit();
    }
    if (pid == 0) {
      // Child process
      int child_id = i;
      int my_tickets = tickets[i];
      
      // Close unnecessary pipe ends
      int j;
      for (j = 0; j < 3; j++) {
        close(pipes[j][0]);  // Close all read ends
        if (j != i) {
          close(pipes[j][1]);  // Close other write ends
        }
      }
      
      // Set tickets
      if (settickets(my_tickets) < 0) {
        printf(1, "settickets failed\n");
        exit();
      }

      // Record start time
      int start_time = uptime();
      int work_count = 0;
      
      // Do work for TEST_DURATION ticks
      while (uptime() - start_time < TEST_DURATION) {
        // One work unit: WORK_UNIT iterations
        volatile int j;
        for (j = 0; j < WORK_UNIT; j++) {
          // CPU-bound work
        }
        work_count++;
      }
      
      int elapsed = uptime() - start_time;
      printf(1, "Child %d (%d tickets): completed %d work units in %d ticks\n", 
             child_id + 1, my_tickets, work_count, elapsed);
      
      // Send work count to parent
      write(pipes[i][1], &work_count, sizeof(work_count));
      close(pipes[i][1]);
      exit();
    }
  }

  // Parent: close all write ends and read results
  for (i = 0; i < 3; i++) {
    close(pipes[i][1]);
  }

  // Wait for all children and read their work counts
  for (i = 0; i < 3; i++) {
    wait();
    read(pipes[i][0], &work_counts[i], sizeof(work_counts[i]));
    close(pipes[i][0]);
  }

  // Calculate and print ratios
  printf(1, "\nTest complete.\n");
  printf(1, "Expected work ratios based on tickets (10:20:30) = 1:2:3\n");
  
  // Normalize ratios to the smallest value
  int min_work = work_counts[0];
  printf(1, "\nActual work unit counts: %d : %d : %d\n", 
         work_counts[0], work_counts[1], work_counts[2]);
  
  // Calculate ratios (multiply by 100 to get 2 decimal places without float)
  int ratio1 = (work_counts[0] * 100) / min_work;
  int ratio2 = (work_counts[1] * 100) / min_work;
  int ratio3 = (work_counts[2] * 100) / min_work;
  
  printf(1, "Normalized ratios: %d.%d : %d.%d : %d.%d\n",
         ratio1 / 100, ratio1 % 100,
         ratio2 / 100, ratio2 % 100,
         ratio3 / 100, ratio3 % 100);
  
  exit();
}