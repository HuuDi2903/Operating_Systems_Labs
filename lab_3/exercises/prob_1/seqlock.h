#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
  pthread_mutex_t mutex;      // Protects sequence counter
  unsigned int sequence;      // Sequence counter
} pthread_seqlock_t;

int pthread_seqlock_init(pthread_seqlock_t *seqlock);
int pthread_seqlock_destroy(pthread_seqlock_t *seqlock);
/*--------------------------------------------------------*/
int pthread_seqlock_rdlock(pthread_seqlock_t *seqlock);
int pthread_seqlock_rdunlock(pthread_seqlock_t *seqlock);
int pthread_seqlock_wrlock(pthread_seqlock_t *seqlock);
int pthread_seqlock_wrunlock(pthread_seqlock_t *seqlock);



