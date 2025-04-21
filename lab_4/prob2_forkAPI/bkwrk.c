#include "bktpool.h"
#include <signal.h>
#include <stdio.h>

#define _GNU_SOURCE
#include <linux/sched.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>
#include <sys/mman.h>

// #define DEBUG
#define INFO
// #define WORK_THREAD

// FIFO for available workers
int wrk_fifo[MAX_WORKER];
int wrk_fifo_head = 0, wrk_fifo_tail = 0, wrk_fifo_size = 0;
pthread_mutex_t wrk_fifo_mutex = PTHREAD_MUTEX_INITIALIZER;

void enqueue_worker(int wid) {
  pthread_mutex_lock(&wrk_fifo_mutex);
  if (wrk_fifo_size < MAX_WORKER) {
      wrk_fifo[wrk_fifo_tail] = wid;
      wrk_fifo_tail = (wrk_fifo_tail + 1) % MAX_WORKER;
      wrk_fifo_size++;
  }
  pthread_mutex_unlock(&wrk_fifo_mutex);
}

int dequeue_worker(void) {
  pthread_mutex_lock(&wrk_fifo_mutex);
  int wid = -1;
  if (wrk_fifo_size > 0) {
      wid = wrk_fifo[wrk_fifo_head];
      wrk_fifo_head = (wrk_fifo_head + 1) % MAX_WORKER;
      wrk_fifo_size--;
  }
  pthread_mutex_unlock(&wrk_fifo_mutex);
  return wid;
}

void * bkwrk_worker(void * arg) {
  sigset_t set;
  int sig;
  int s;
  int i = * ((int * ) arg); // Default arg is integer of workid
  struct bkworker_t *wrk = &shared_workers[i];

  /* Taking the mask for waking up */
  sigemptyset( & set);
  sigaddset( & set, SIGUSR1);
  sigaddset( & set, SIGQUIT);

#ifdef DEBUG
  fprintf(stderr, "worker %i start living tid %d \n", i, getpid());
  fflush(stderr);
#endif

  while (1) {
    /* wait for signal */
    s = sigwait( & set, & sig);
    if (s != 0)
      continue;

#ifdef INFO
    fprintf(stderr, "worker wake %d up\n", i);
#endif

    /* Busy running */
    if (wrk -> func != NULL) {
      wrk -> func(wrk -> arg);
    }

    /* Advertise I DONE WORKING */
    shared_busy_flags[i] = 0;
    shared_workers[i].func = NULL;
    shared_workers[i].arg = NULL;
    shared_workers[i].bktaskid = -1;

    enqueue_worker(i); // Mark this worker as available again
  }
}

int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid) {
  if (wrkid < 0 || wrkid > MAX_WORKER)
    return -1;

  struct bktask_t * tsk = bktask_get_byid(bktaskid);

  if (tsk == NULL)
    return -1;

  /* Advertise I AM WORKING */
  shared_busy_flags[wrkid] = 1;
  shared_workers[wrkid].func = tsk->func;
  shared_workers[wrkid].arg = tsk->arg;
  shared_workers[wrkid].bktaskid = bktaskid;

  printf("Assign tsk %d wrk %d \n", tsk -> bktaskid, wrkid);
  return 0;
}

int bkwrk_create_worker() {
  unsigned int i;

  for (i = 0; i < MAX_WORKER; i++) {
#ifdef WORK_THREAD
    void ** child_stack = (void ** ) malloc(STACK_SIZE);
    unsigned int wrkid = i;
    pthread_t threadid;

    sigset_t set;
    int s;

    sigemptyset( & set);
    sigaddset( & set, SIGQUIT);
    sigaddset( & set, SIGUSR1);
    sigprocmask(SIG_BLOCK, & set, NULL);

    /* Stack grow down - start at top*/
    void * stack_top = child_stack + STACK_SIZE;

    wrkid_tid[i] = clone( & bkwrk_worker, stack_top,
      CLONE_VM | CLONE_FILES,
      (void * ) & i);
#ifdef INFO
    fprintf(stderr, "bkwrk_create_worker got worker %u\n", wrkid_tid[i]);
#endif

    usleep(100);

#else
    /* TODO: Implement fork version of create worker */
    int worker_id = i;
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGQUIT);
        sigaddset(&set, SIGUSR1);
        sigprocmask(SIG_BLOCK, &set, NULL);
        
        // Start the worker function
        bkwrk_worker((void*)&worker_id);

        exit(0);
    } else if (pid > 0) {
        // Parent process 
        wrkid_tid[i] = pid;
        
        #ifdef INFO
        fprintf(stderr, "bkwrk_create_worker got worker (fork) %u\n", pid);
        #endif
        
        usleep(100);
    } else {
        // Fork failed
        fprintf(stderr, "Fork failed for worker %d\n", i);
        return -1;
    }
#endif
  }

  // Initialize the worker state 
  wrk_fifo_head = wrk_fifo_tail = wrk_fifo_size = 0;
  for(int i=0; i<MAX_WORKER; ++i)
      enqueue_worker(i);

  return 0;
}

int bkwrk_get_worker() {
  /* TODO Implement the scheduler to select the resource entity 
   * The return value is the ID of the worker which is not currently 
   * busy or wrkid_busy[1] == 0 
   */

  return dequeue_worker();
}

int bkwrk_dispatch_worker(unsigned int wrkid) {

#ifdef WORK_THREAD
  unsigned int tid = wrkid_tid[wrkid];

  /* Invalid task */
  if (worker[wrkid].func == NULL)
    return -1;

  syscall(SYS_tkill, tid, SIG_DISPATCH);

#else
  /* TODO: Implement fork version to signal worker process here */
  pid_t worker_pid = wrkid_tid[wrkid];
    
  /* Invalid task */
  if (shared_workers[wrkid].func == NULL)
    return -1;
  
  /* Send SIGUSR1 signal to the worker process */
  if (kill(worker_pid, SIG_DISPATCH) != 0) {
      perror("Failed to signal worker process");
      return -1;
  }
  
  #ifdef DEBUG
  fprintf(stderr, "brkwrk dispatch wrkid %d - send signal to PID %u\n", 
          wrkid, worker_pid);
  #endif
  
  return 0;
#endif
}