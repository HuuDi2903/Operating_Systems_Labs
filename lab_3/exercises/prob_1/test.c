#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "seqlock.h"

#define NUM_READERS 3
#define NUM_WRITERS 2
#define NUM_OPERATIONS 5

// Shared data protected by the sequence lock
struct shared_data {
    int value1;
    int value2;
};

struct shared_data shared = {0, 0};
pthread_seqlock_t seqlock;

void* reader_thread(void* arg) {
  int id = *(int*)arg;
  int i;
  struct shared_data local_copy;
  unsigned int seq, check_seq;
  
  for (i = 0; i < NUM_OPERATIONS; i++) {
      // Try to read data consistently
      do {
          // Read sequence number before reading data
          seq = seqlock.sequence;
          
          // If writing is in progress (odd sequence), try again
          if (seq & 1) {
              sched_yield();
              continue;
          }
          
          // Read the shared data
          local_copy = shared;
          
          // Check sequence again to ensure data is consistent
          check_seq = seqlock.sequence;
          
          // If sequence changed, data might be inconsistent, try again
          if (seq != check_seq) {
              continue;
          }
          
          // Data is consistent
          printf("Reader %d: Read values: %d, %d (seq: %u)\n", 
                 id, local_copy.value1, local_copy.value2, seq);
          break;
          
      } while (1);
      
      // Sleep a bit before next read
      usleep(200000 + (rand() % 300000)); // 200-500ms
  }
  
  printf("Reader %d: Done.\n", id);
  free(arg);
  return NULL;
}

void* writer_thread(void* arg) {
  int id = *(int*)arg;
  int i;
  
  for (i = 0; i < NUM_OPERATIONS; i++) {
      // Sleep a bit before writing
      usleep(500000 + (rand() % 500000));
      
      // Get sequence before acquiring lock
      unsigned int before_seq = seqlock.sequence;
      
      // Begin write critical section
      pthread_seqlock_wrlock(&seqlock);
      
      // Print sequence after acquiring lock (should be odd)
      printf("Writer %d: Beginning write - seq before: %u, after lock: %u\n", 
             id, before_seq, seqlock.sequence);
      
      // Update values
      shared.value1 = id * 100 + i;
      usleep(200000);
      shared.value2 = id * 100 + i + 1;
      
      // Get sequence before releasing lock
      unsigned int during_seq = seqlock.sequence;
      
      // End write critical section
      pthread_seqlock_wrunlock(&seqlock);
      
      // Print sequence after releasing lock (should be even)
      printf("Writer %d: Completed write - seq during: %u, after unlock: %u, values: %d, %d\n", 
             id, during_seq, seqlock.sequence, shared.value1, shared.value2);

      // Sleep a bit before next write
      usleep(200000 + (rand() % 300000)); // 200-500ms
  }
  
  printf("Writer %d: Done.\n", id);
  free(arg);
  return NULL;
}

int main() {
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int i;
    int *id;
    
    // Seed the random number generator
    srand(time(NULL));
    
    // Initialize the sequence lock
    if (pthread_seqlock_init(&seqlock) != 0) {
        perror("Failed to initialize sequence lock");
        return 1;
    }
    
    printf("Testing sequence lock implementation...\n");
    
    // Create reader threads
    for (i = 0; i < NUM_READERS; i++) {
        id = malloc(sizeof(int));
        *id = i;
        if (pthread_create(&readers[i], NULL, reader_thread, id) != 0) {
            perror("Failed to create reader thread");
            return 1;
        }
    }
    
    // Create writer threads
    for (i = 0; i < NUM_WRITERS; i++) {
        id = malloc(sizeof(int));
        *id = i;
        if (pthread_create(&writers[i], NULL, writer_thread, id) != 0) {
            perror("Failed to create writer thread");
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    // Destroy the sequence lock
    pthread_seqlock_destroy(&seqlock);
    
    printf("All threads completed.\n");
    return 0;
}