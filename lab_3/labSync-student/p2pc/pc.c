/* DONE */

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define BUFSIZE 2
#define THREADS 1 // 1 producer and 1 consumer
#define LOOPS 3 * BUFSIZE // variable

// Initiate shared buffer
int buffer[BUFSIZE];
int fill = 0;
int use = 0;

/* TODO: Fill in the synchronization stuff */
sem_t empty; // semaphore for empty slots
sem_t full; // semaphore for full slots
pthread_mutex_t mutex; // mutex for critical section

void put(int value); // put data into buffer
int get(); // get data from buffer

void *producer(void *arg) {
    int i;
    int tid = (int)arg;
    for (i = 0; i < LOOPS; i++) {
        /* TODO: Fill in the synchronization stuff */
        sem_wait(&empty); // wait for empty slot
        pthread_mutex_lock(&mutex); 

        put(i); // line P2
        printf("Producer%d put data%d\n", tid, i);
        sleep(1);

        /* TODO: Fill in the synchronization stuff */
        pthread_mutex_unlock(&mutex);
        sem_post(&full); // signal that there is a new item
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    int i, tmp = 0;
    int tid = (int)arg;
    while (tmp != -1) {
        /* TODO: Fill in the synchronization stuff */
        sem_wait(&full); // wait for full slot
        pthread_mutex_lock(&mutex);

        tmp = get(); // line C2
        printf("Consumer%d get data%d\n", tid, tmp);
        sleep(1);

        /* TODO: Fill in the synchronization stuff */
        pthread_mutex_unlock(&mutex);
        sem_post(&empty); // signal that there is an empty slot
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int i, j;
    int tid[THREADS];
    pthread_t producers[THREADS];
    pthread_t consumers[THREADS];
    
    /* TODO: Fill in the synchronization stuff */
    sem_init(&empty, 0, BUFSIZE); // Initialize empty semaphore
    sem_init(&full, 0, 0); // Initialize full semaphore
    pthread_mutex_init(&mutex, NULL); // Initialize mutex
    
    for (i = 0; i < THREADS; i++) {
        tid[i] = i;
        // Create producer thread
        pthread_create(&producers[i], NULL, producer, (void *)tid[i]);
        // Create consumer thread
        pthread_create(&consumers[i], NULL, consumer, (void *)tid[i]);
    }
    
    for (i = 0; i < THREADS; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    
    /* TODO: Fill in the synchronization stuff destroy (if needed) */
    sem_destroy(&empty); // Destroy empty semaphore
    sem_destroy(&full); // Destroy full semaphore
    pthread_mutex_destroy(&mutex); // Destroy mutex
    
    return 0;
}

void put(int value) {
    buffer[fill] = value; // line f1
    fill = (fill + 1) % BUFSIZE; // line f2
}

int get() {
    int tmp = buffer[use]; // line g1
    buffer[use] = -1; //clean the item
    use = (use + 1) % BUFSIZE; // line g2
    return tmp;
}