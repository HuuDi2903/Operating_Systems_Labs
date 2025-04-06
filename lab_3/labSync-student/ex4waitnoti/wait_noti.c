#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;  // Number of elements in the buffer
pthread_mutex_t mutex;
pthread_cond_t buffer_not_full, buffer_not_empty; // Condition variables for buffer state

void* producer(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&buffer_not_full, &mutex);  // Wait if buffer is full
        }

        buffer[count++] = i;
        printf("Producer produced: %d\n", i);

        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            pthread_cond_wait(&buffer_not_empty, &mutex); // Wait if buffer is empty
        }

        int item = buffer[--count];
        printf("Consumer consumed: %d\n", item);

        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);

    return 0;
}