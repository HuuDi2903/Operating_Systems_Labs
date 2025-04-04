#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "seqlock.h"  /* TODO implement this header file */

pthread_seqlock_t lock;

int main()
{
   printf("Testing sequence lock...\n");
   int val = 0;

   pthread_seqlock_init(&lock);

   pthread_seqlock_wrlock(&lock);
   val++;
   pthread_seqlock_wrunlock(&lock);


   // if(pthread_seqlock_rdlock(&lock) == 1){
   //    printf("val = %d\n", val); 
   //    pthread_seqlock_rdunlock(&lock);
   // }

   // Get the sequence number before reading
   unsigned int seq_start = pthread_seqlock_rdlock(&lock);

   // Get the sequence number after reading
   unsigned int seq_end = pthread_seqlock_rdunlock(&lock);
   
   // Check if the read was consistent (sequence didn't change and is even)
   if(seq_start == seq_end && !(seq_start & 1)) {
      printf("Read successful! val = %d\n", val);
   } else {
      printf("Read failed!\n");
   }
}
