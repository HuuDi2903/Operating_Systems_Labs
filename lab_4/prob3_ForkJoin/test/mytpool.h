/*
ForkJoinPool
  ├── Task Queue
  ├── Worker Threads
  ├── Task Stealing Mechanism
  └── Synchronization Primitives
  */

  #ifndef MYTPOOL_H
#define MYTPOOL_H

  #include <stdio.h>
  #include <stdlib.h>
  #include <pthread.h>
  #include <signal.h>
  #include <unistd.h>
  
  #define MAX_WORKERS 10
  #define MAX_TASKS 100
  
  // Task status
  typedef enum {
      TASK_WAITING,    // Task is waiting to be executed
      TASK_RUNNING,    // Task is currently running
      TASK_COMPLETED,  // Task has completed execution
      TASK_FAILED      // Task execution failed
  } task_status_t;
  
  // Task structure
  typedef struct task {
      int task_id;                     // Unique task ID
      void *(*function)(void *);       // Function to execute
      void *arg;                       // Arguments for the function
      void *result;                    // Result of the task
      task_status_t status;            // Status of the task
      int parent_id;                   // Parent task ID (-1 if root task)
      int child_count;                 // Number of child tasks
      int completed_children;          // Number of completed children
      pthread_mutex_t lock;            // Lock for task manipulation
      pthread_cond_t completion_cond;  // Condition variable for join
  } task_t;
  
  // Fork-Join Pool
  typedef struct {
      task_t tasks[MAX_TASKS];         // Task array
      int task_count;                  // Number of tasks in the pool
      pthread_t workers[MAX_WORKERS];  // Worker threads
      int worker_count;                // Number of worker threads
      int active;                      // Pool status (1=active, 0=shutdown)
      pthread_mutex_t pool_lock;       // Lock for pool manipulation
      pthread_cond_t task_available;   // Condition for task availability
  } fork_join_pool_t;

  int fj_pool_init(fork_join_pool_t *pool, int num_workers);
  int fj_pool_shutdown(fork_join_pool_t *pool);
  void *worker_function(void *arg);
  int get_next_task(fork_join_pool_t *pool, task_t **task);
  
#endif