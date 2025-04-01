#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>

// Global variables
int *buffer;        // Array to hold the random numbers
int arrsz;          // Size of the array
int tnum;           // Number of threads
long long total_sum = 0;  // Variable to hold the total sum (initialized to 0)

// Mutex for thread-safe updates to sumbuff
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototype for the provided array generation function
int generate_array_data(int* buf, int arraysize, int seednum){
  srand(seednum);  // Seed the random number generator
  // for (int i = 0; i < arraysize; i++) {
  //     buf[i] = rand() % 100;  // Fill the array with random numbers (0-99)
  // }
  for (int i = 0; i < arraysize; i++) {
    buf[i] = rand() % 100;  // Generate random numbers between 0 and 99
  }
  printf("Array generation complete. First few values: %d, %d, %d...\n", buf[0], buf[1], buf[2]);
  return 0;  
};

// Structure to define the range each thread will work on
struct range {
  int start;
  int end;
};

void *sum_worker (void *arg) {
  // printf("Print worker from %d to %d\n", idx_range->start, idx_range->end);
  struct range idx_range = *(struct range*)arg;
  long long local_sum = 0;  // Local sum for this thread
  for (int i = idx_range.start; i <= idx_range.end; i++) {
      local_sum += buffer[i];  // Calculate the sum for this thread
  }

  pthread_mutex_lock(&mutex);  // Lock the mutex before updating total_sum
  total_sum += local_sum;  // Update the global total sum
  pthread_mutex_unlock(&mutex);  // Unlock the mutex

  printf("Thread completed range %d to %d with local sum: %lld\n", idx_range.start, idx_range.end, local_sum);
  
  return NULL;  // Return NULL to indicate thread completion
}

int main(int argc, char* argv[]) {
  printf("Program starting...\n");
  // Check if the number of arguments is correct
  if (argc < 3 || argc > 4) {
      printf("usage: aggsum arrsz tnum [seednum]\n");
      printf("Generate randomly integer array size <arrsz> and calculate sum parallelly\n");
      printf("using <tnum> threads. The optional <seednum> value use to control the\n");
      printf("randomization of the generated array.\n");
      return 1;
  }
  
  // Parse command line arguments
  arrsz = atoi(argv[1]); 
  tnum = atoi(argv[2]);  
  int seednum = (argc == 4 && isdigit(argv[3][0])) ? atoi(argv[3]) : time(NULL); 
  
  // Validate arguments
  if (arrsz <= 0 || tnum <= 0) {
      printf("Error: Array size and number of threads must be positive\n");
      return 1;
  }
  
  // Adjust tnum if it's larger than arrsz (no point having more threads than elements)
  if (tnum > arrsz) {
      printf("Warning: Number of threads reduced to match array size\n");
      tnum = arrsz;
  }

  // Allocate memory for the array
  buffer = (int*)malloc(arrsz * sizeof(int));
  if (buffer == NULL) {
      printf("Error: Memory allocation failed\n");
      return 1;
  }
  
  // Generate the array data
  generate_array_data(buffer, arrsz, seednum);
  
  // Initialize the sum to zero
  total_sum = 0;
  
  // Prepare for creating threads
  pthread_t threads[tnum];  // Array to hold thread IDs
  // pthread_t *threads = (pthread_t*)malloc(tnum * sizeof(pthread_t));
  if (threads == NULL) {
      printf("Error: Memory allocation failed for threads\n");
      free(buffer);
      return 1;
  }
  struct range thread_ranges[tnum];  // Array to hold work ranges for each thread
  
  // Calculate the basic number of elements per thread
  int elements_per_thread = arrsz / tnum;
  int remaining_elements = arrsz % tnum;
  
  // Create threads and assign work
  int start_idx = 0;
  for(int i = 0; i < tnum; i++) {
      // Calculate the range for this thread
      thread_ranges[i].start = start_idx;
      
      // Distribute remaining elements among the first 'remaining_elements' threads
      int elements_for_this_thread = elements_per_thread;
      if (i < remaining_elements) {
          elements_for_this_thread++;
      }
      
      thread_ranges[i].end = start_idx + elements_for_this_thread - 1;
      start_idx = thread_ranges[i].end + 1;
      
      // Create the thread
      pthread_create(&threads[i], NULL, sum_worker, &thread_ranges[i]);
  }

  for(int i = 0; i < tnum; i++) {
      pthread_join(threads[i], NULL);  // Wait for each thread to finish
  }

  printf("Total sum: %lld\n", total_sum);  // Print the total sum

  // Calculate sum sequentially to verify
  long long verify_sum = 0;
  for (int i = 0; i < arrsz; i++) {
      verify_sum += buffer[i];
  }
  
  printf("Verification sum: %lld\n", verify_sum);
  if (total_sum != verify_sum) {
      printf("Warning: Sums don't match!\n");
  } else {
      printf("Sums match! Calculation is correct.\n");
  }

  // Free the allocated memory
  free(buffer);

  return 0;
}