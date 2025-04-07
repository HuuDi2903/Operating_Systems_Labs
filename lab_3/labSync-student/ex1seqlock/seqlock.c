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

   if(!(seq_start & 1)) {  // Can only read if the sequence is even
      // Read the value
      printf("Read successful! Value: %d\n", val);
      pthread_seqlock_rdunlock(&lock);
   } else {
      printf("Write in progress, cannot read!\n");
   }

   return 0;
}
