#include "seqlock.h"

/*
- Sequence is odd --> Writer is active
- Sequence is even --> Many readers can enter
- When readers want to read, they check the sequence number, and wait for writer to finish
*/

pthread_seqlock_t lock;

int pthread_seqlock_init(pthread_seqlock_t *seqlock) {
  if (seqlock == NULL)  return -1;
  
  int ret = pthread_mutex_init(&seqlock->mutex, NULL);
  if (ret == 0) {
      seqlock->sequence = 1;  // Initialize sequence to even number (0)
  }
  return ret;
}

int pthread_seqlock_destroy(pthread_seqlock_t *seqlock) {
  if (seqlock == NULL)  return -1;
  
  int ret = pthread_mutex_destroy(&seqlock->mutex);
  return ret;
}

int pthread_seqlock_rdlock(pthread_seqlock_t *seqlock) {
  if (seqlock == NULL)
      return -1;
  
  unsigned int seq;
  do {
      /* Read sequence before accessing data */
      seq = seqlock->sequence;
      
      /* If sequence is odd, a writer has the lock - we must wait */
      if (seq & 1) {
          /* Wait and try again */
          sched_yield();  // Give up CPU, let writer finish
          continue;
      }      
  } while (0);  // We just need to check once, readers don't actually lock
  
  return 0;
}

int pthread_seqlock_rdunlock(pthread_seqlock_t *seqlock) {
  if (seqlock == NULL)  return -1;
  return 0;
}

int pthread_seqlock_wrlock(pthread_seqlock_t *seqlock) {
  if (seqlock == NULL)
      return -1;
  
  /* Lock the mutex to ensure only one writer */
  int ret = pthread_mutex_lock(&seqlock->mutex);
  if (ret == 0) {
      /* Increment sequence to odd number, indicating write in progress */
      seqlock->sequence++;

      if (!(seqlock->sequence & 1)) {
          /* If sequence is even, increment to make it odd */
          seqlock->sequence++;
      }
  }
  return ret;
}

int pthread_seqlock_wrunlock(pthread_seqlock_t *seqlock) {
  if (seqlock == NULL)  return -1;
  
    /* Ensure sequence is even (no write in progress) */
  if (seqlock->sequence & 1) {
      seqlock->sequence++;
  }
  
  pthread_mutex_unlock(&seqlock->mutex); // Unlock the mutex
  return 0;
}


