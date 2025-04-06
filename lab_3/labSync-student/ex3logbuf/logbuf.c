#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_LOG_LENGTH 50     // Max length of a single log message
#define MAX_BUFFER_SLOT 5     // Max number of log slots in the buffer

char log_buffer[MAX_BUFFER_SLOT][MAX_LOG_LENGTH]; // Log buffer
int log_count = 0;                               // Number of logs in the buffer
pthread_mutex_t mutex;
pthread_cond_t buffer_not_full;
pthread_cond_t buffer_not_empty;

void wrlog(char* log) {
   pthread_mutex_lock(&mutex);

   while (log_count == MAX_BUFFER_SLOT) {
      pthread_cond_wait(&buffer_not_full, &mutex);    // Wait if the buffer is full
   }

   snprintf(log_buffer[log_count], MAX_LOG_LENGTH, "%s", log); // Write formatted log to the buffer slot
   printf("Slot %d: %s\n", log_count, log);

   log_count++;

   pthread_cond_signal(&buffer_not_empty);
   pthread_mutex_unlock(&mutex);
}

void flushlog() {
   pthread_mutex_lock(&mutex);

   while (log_count == 0) {
      pthread_cond_wait(&buffer_not_empty, &mutex);   // Wait if the buffer is empty
   }

   // printf("Flushing logs...\n");
   // for (int i = 0; i < log_count; i++) {
   //    printf("Slot %d: %s\n", i, log_buffer[i]);
   // }

   log_count = 0;

   pthread_cond_signal(&buffer_not_full);
   pthread_mutex_unlock(&mutex);
}

// Thread for periodic flushing
void* flush_thread(void* arg) {
   while (1) {
      sleep(5); // Flush every 5 seconds
      flushlog();
   }

   return NULL;
}

int main() {
   pthread_t flusher_thread;

   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&buffer_not_full, NULL);
   pthread_cond_init(&buffer_not_empty, NULL);

   pthread_create(&flusher_thread, NULL, flush_thread, NULL);

   // // Simulate log writing
   // wrlog("Log 1");
   // wrlog("Log 2");
   // wrlog("Log 3");
   // wrlog("Log 4");
   // wrlog("Log 5");
   // wrlog("Log 6");
   // wrlog("Log 7");
   // wrlog("Log 8");
   // wrlog("Log 9");
   // wrlog("Log 10");
   // wrlog("Log 11");
   // wrlog("Log 12");
   // wrlog("Log 13");
   // wrlog("Log 14");
   // wrlog("Log 15");
   // wrlog("Log 16");
   // wrlog("Log 17");
   // wrlog("Log 18");

   for (int i = 0; i < 30; i++) {
      char log[MAX_LOG_LENGTH];
      snprintf(log, MAX_LOG_LENGTH, "Item (%d)", i);
      wrlog(log);
      sleep(1); 
   }


   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&buffer_not_full);
   pthread_cond_destroy(&buffer_not_empty);

   return 0;
}