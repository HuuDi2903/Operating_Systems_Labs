#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5

pthread_mutex_t mtx;
pthread_cond_t chopstick[N];

void *philosopher(void*);
void eat(int);
void think(int);

sem_t forks[N];

int main()
{
   int a[N];
   pthread_t tid[N];

   for (int i = 0; i < N; i++) {
      sem_init(&forks[i], 0, 1); 
   }

   for (int i = 0; i < N; i++) {
         a[i] = i;
         pthread_create(&tid[i], NULL, philosopher, &a[i]);
   }

   for (int i = 0; i < N; i++) {
         pthread_join(tid[i], NULL);
   }

   for (int i = 0; i < N; i++) {
         sem_destroy(&forks[i]);
   }

   return 0;
}

void *philosopher(void *num)
{
   int phil = *(int*) num;
   printf("Philosopher %d has entered room\n", phil);
	  
   while (1)
   {
      think(phil);

      sem_wait(&forks[phil]);               // Pick left fork
      sem_wait(&forks[(phil + 1) % N]);    // Pick right fork

      eat(phil);                   

      sem_post(&forks[(phil + 1) % N]);    // Release right fork
      sem_post(&forks[phil]);               // Release left fork

      printf("Philosopher %d has finished eating!\n", phil);
      sleep(3); 
   }

   return NULL;
}

void eat(int phil)
{
   printf("Philosopher %d is eating\n", phil);
   sleep(1);
}

void think(int phil)
{
   printf("Philosopher %d is thinking\n", phil);
   sleep(1);
}