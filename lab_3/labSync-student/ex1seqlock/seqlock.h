#ifndef PTHREAD_H
#include <pthread.h>
#endif

typedef struct pthread_seqlock { /* TODO: implement the structure */
   pthread_mutex_t mutex;      // Protects sequence counter
   unsigned int sequence;      // Sequence counter
} pthread_seqlock_t;

static inline void pthread_seqlock_init(pthread_seqlock_t *rw)
{
   if (rw == NULL)  return;
  
   int ret = pthread_mutex_init(&rw->mutex, NULL);
   if (ret == 0) {
       rw->sequence = 0;  // Initialize sequence to even number (0)
   }
}

static inline void pthread_seqlock_wrlock(pthread_seqlock_t *rw)
{
   if (rw == NULL) return;
    
   pthread_mutex_lock(&rw->mutex);
   
   // Increment sequence to odd value, indicating write in progress
   rw->sequence++;
}

static inline void pthread_seqlock_wrunlock(pthread_seqlock_t *rw)
{
   if (rw == NULL) return;
    
   // Increment sequence to even value, indicating write completed
   rw->sequence++;
   
   pthread_mutex_unlock(&rw->mutex);
}

static inline unsigned pthread_seqlock_rdlock(pthread_seqlock_t *rw)
{
   if (rw == NULL)   return -1;

   unsigned int seq;
   do {
      /* Read sequence before accessing data */
      seq = rw->sequence;
      
      /* If sequence is odd, the writing is happening - we must wait */
      if (seq & 1) {
         /* Wait and try again */
         sched_yield();  
         continue;
      }      

      return seq;  // Return the sequence number

   } while (1);  
}


static inline unsigned pthread_seqlock_rdunlock(pthread_seqlock_t *rw)
{
    if (rw == NULL) return -1;
    return rw->sequence;
}

