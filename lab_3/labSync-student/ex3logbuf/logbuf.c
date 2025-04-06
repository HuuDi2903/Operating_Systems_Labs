#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LOG_LENGTH 10
#define MAX_BUFFER_SLOT 6
#define MAX_LOOPS 30

char logbuf[MAX_BUFFER_SLOT][MAX_LOG_LENGTH];
int count = 0;
pthread_mutex_t mutex;

void *wrlog(void *data)
{
   char str[MAX_LOG_LENGTH];
   int id = *(int*) data;
   
   // Create log message
   sprintf(str, "%d", id);
   
   // Add some delay to stagger thread execution
   usleep(10000 + (id % 10) * 5000);
   
   pthread_mutex_lock(&mutex);
   
   // Write to the buffer
   strcpy(logbuf[count % MAX_BUFFER_SLOT], str);
   count++;
   
   // Force count to stay within bounds
   if (count >= MAX_BUFFER_SLOT) {
      count = MAX_BUFFER_SLOT;
   }
   
   pthread_mutex_unlock(&mutex);
   
   return NULL;
}

void flushlog()
{
   int i;
   char temp_buffer[MAX_BUFFER_SLOT][MAX_LOG_LENGTH];
   int local_count;
   
   pthread_mutex_lock(&mutex);
   
   // Copy count value safely
   local_count = count;
   
   // Safety check - ensure local_count is within bounds
   if (local_count > MAX_BUFFER_SLOT)
      local_count = MAX_BUFFER_SLOT;
   
   // Only process if we have entries
   if (local_count > 0) {
      // Copy buffer contents to temp buffer
      for (i = 0; i < local_count; i++) {
         strncpy(temp_buffer[i], logbuf[i], MAX_LOG_LENGTH - 1);
         temp_buffer[i][MAX_LOG_LENGTH - 1] = '\0'; // Ensure null-termination
      }
      
      // Reset count
      count = 0;
      
      pthread_mutex_unlock(&mutex);
      
      // Print logs outside the lock
      for (i = 0; i < local_count; i++) {
         printf("Slot %d: %s\n", i, temp_buffer[i]);
      }
      fflush(stdout);
   } else {
      pthread_mutex_unlock(&mutex);
   }
}

void *timer_start(void *args)
{
   struct timespec ts;
   ts.tv_sec = 0;
   ts.tv_nsec = 200 * 1000000; // 200ms
   
   while (1) {
      nanosleep(&ts, NULL);
      flushlog();
   }
   
   return NULL;
}

int main()
{
   int i;
   pthread_t tid[MAX_LOOPS];
   pthread_t timer_tid;
   int id[MAX_LOOPS];
   
   // Initialize mutex
   pthread_mutex_init(&mutex, NULL);
   
   // Clear buffer
   for (i = 0; i < MAX_BUFFER_SLOT; i++) {
      memset(logbuf[i], 0, MAX_LOG_LENGTH);
   }
   
   // Start timer thread
   pthread_create(&timer_tid, NULL, timer_start, NULL);
   
   // Create writer threads
   for (i = 0; i < MAX_LOOPS; i++) {
      id[i] = i;
      pthread_create(&tid[i], NULL, wrlog, &id[i]);
      usleep(5000);  // 5ms between thread creation
   }
   
   // Wait for writer threads
   for (i = 0; i < MAX_LOOPS; i++) {
      pthread_join(tid[i], NULL);
   }
   
   // Sleep to allow final flush
   sleep(1);
   
   // Cancel timer thread (it runs forever)
   pthread_cancel(timer_tid);
   pthread_join(timer_tid, NULL);
   
   // Clean up
   pthread_mutex_destroy(&mutex);
   
   return 0;
}