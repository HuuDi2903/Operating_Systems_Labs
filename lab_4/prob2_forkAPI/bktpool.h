#ifndef BKTPOOL_H
#define BKTPOOL_H

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// For shared memory
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_WORKER 10
#define WRK_THREAD 1
#define STACK_SIZE 4096
#define SIG_DISPATCH SIGUSR1

// FIFO queue for available workers
extern int wrk_fifo[MAX_WORKER];
extern int wrk_fifo_head; // dequeue here
extern int wrk_fifo_tail; // enqueue here
extern int wrk_fifo_size;
extern pthread_mutex_t wrk_fifo_mutex;

// Shared memory
extern struct bkworker_t *shared_workers;
extern int *shared_busy_flags;

/* Task ID is unique non-decreasing integer */
int taskid_seed;

int wrkid_tid[MAX_WORKER];
int wrkid_busy[MAX_WORKER];
int wrkid_cur;

struct bktask_t {
  void( * func)(void * arg);
  void * arg;
  unsigned int bktaskid;
  struct bktask_t * tnext;
}* bktask;

int bktask_sz;

struct bkworker_t {
  void( * func)(void * arg);
  void * arg;
  unsigned int wrkid;
  unsigned int bktaskid;
};

struct bkworker_t worker[MAX_WORKER];

/* Prototype API */

/* bktpool module */
int bktpool_init();

/* bktask module */
struct bktask_t * bktask_get_byid(unsigned int bktaskid);
int bktask_init(unsigned int * bktaskid, void * func, void * arg);
int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid);

/* bkwrk module */
void * bkwrk_worker(void * arg);
int bkwrk_create_worker();
int bkwrk_dispatch_worker(unsigned int wrkid);
int bkwrk_get_worker();

#endif