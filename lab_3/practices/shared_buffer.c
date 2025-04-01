/* DONE */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int MAXCOUNT = 1e9;
static int count = 0;
pthread_mutex_t mutex; // mutex for critical section

void *f_count(void *sid) {
  int i ;
  for (i = 0; i < MAXCOUNT; i++) {
    pthread_mutex_lock(&mutex);
    count = count + 1;
    pthread_mutex_unlock(&mutex);
  }

  printf ("Thread %s: holding %d \n", (char*) sid , count);
  return NULL; 
}

int main() {
  pthread_t thread1 , thread2;
  pthread_mutex_init(&mutex, NULL); // Initialize the mutex

  /* Create independent threads each of which will execute function */
  pthread_create ( &thread1 , NULL, &f_count , "1");
  pthread_create ( &thread2 , NULL, &f_count , "2");

  // Wait for thread th1 finish
  pthread_join ( thread1 , NULL);
  // Wait for thread th2 finish
  pthread_join ( thread2 , NULL);

  pthread_mutex_destroy(&mutex); // Destroy the mutex

  return 0;
}