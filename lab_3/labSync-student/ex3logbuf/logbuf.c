#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_LOG_LENGTH 10
#define MAX_BUFFER_SLOT 6
#define MAX_LOOPS 30

char logbuf[MAX_BUFFER_SLOT][MAX_LOG_LENGTH];

int count;
void flushlog();

pthread_mutex_t mutex;

struct _args
{
   unsigned int interval;
};

void *wrlog(void *data)
{
   char str[MAX_LOG_LENGTH];
   int id = *(int*) data;

   usleep(20);
   sprintf(str, "%d", id);

   pthread_mutex_lock(&mutex);  // Lock the mutex
   
   strcpy(logbuf[count], str);
   // count = (count > MAX_BUFFER_SLOT)? count :(count + 1); /* Only increase count to size MAX_BUFFER_SLOT*/
   count = (count + 1) % MAX_BUFFER_SLOT; /* Circular buffer */
   // printf("wrlog(): %d \n", id);

   pthread_mutex_unlock(&mutex);  // Unlock the mutex

   return 0;
}

void flushlog()
{
   int i;
   char nullval[MAX_LOG_LENGTH];

   // printf("flushlog()\n");
   sprintf(nullval, "%d", -1);

   pthread_mutex_lock(&mutex);  // Lock the mutex

   for (i = 0; i < count; i++)
   {
      printf("Slot  %i: %s\n", i, logbuf[i]);
      strcpy(logbuf[i], nullval);
   }

   fflush(stdout);

   /*Reset buffer */
   count = 0;

   pthread_mutex_unlock(&mutex);  // Unlock the mutex

   return;
}

void *timer_start(void *args)
{
   while (1)
   {
      flushlog();
      /*Waiting until the next timeout */
      usleep(((struct _args *) args)->interval);
   }
}

int main()
{
   int i;
   count = 0;
   pthread_t tid[MAX_LOOPS];
   pthread_t lgrid;
   int id[MAX_LOOPS];

   pthread_mutex_init(&mutex, NULL);  // Initialize the mutex

   struct _args args;
   args.interval = 500e3;
   /*500 msec ~ 500 * 1000 usec */

   /*Setup periodically invoke flushlog() */
   pthread_create(&lgrid, NULL, &timer_start, (void*) &args);

   /*Asynchronous invoke task writelog */
   for (i = 0; i < MAX_LOOPS; i++)
   {
      id[i] = i;
      pthread_create(&tid[i], NULL, wrlog, (void*) &id[i]);
   }

   for (i = 0; i < MAX_LOOPS; i++)
      pthread_join(tid[i], NULL);

   sleep(5);

   pthread_mutex_destroy(&mutex);  // Destroy the mutex

   return 0;
}
