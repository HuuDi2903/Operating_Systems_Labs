#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

struct thread_data {
    long long start;
    long long end;
    long long partial_sum;
};

void *sum_range(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    data->partial_sum = 0;
    
    for (long long i = data->start; i <= data->end; i++) {
        data->partial_sum += i;
    }
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int num_threads = atoi(argv[1]);
    long long n = atoll(argv[2]);
    
    if (num_threads <= 0 || n <= 0) {
        printf("Both num_threads and n must be positive!\n");
        return 1;
    }

    pthread_t threads[num_threads];
    struct thread_data thread_data_array[num_threads];
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    long long elements_per_thread = n / num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_data_array[i].start = i * elements_per_thread + 1;
        
        if (i == num_threads - 1) {
            // Last thread takes any remaining elements
            thread_data_array[i].end = n;
        } else {
            thread_data_array[i].end = (i + 1) * elements_per_thread;
        }
        
        pthread_create(&threads[i], NULL, sum_range, (void *)&thread_data_array[i]);
    }
    
    // Wait for threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Combine partial sums
    long long total_sum = 0;
    for (int i = 0; i < num_threads; i++) {
        total_sum += thread_data_array[i].partial_sum;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double cpu_time_used = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Sum of numbers from 1 to %lld = %lld\n", n, total_sum);
    printf("Time taken with %d threads: %f seconds\n", num_threads, cpu_time_used);
    
    return 0;
}