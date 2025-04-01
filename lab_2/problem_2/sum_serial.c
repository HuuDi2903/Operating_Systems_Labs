#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long long sum_serial(long long n) {
    long long sum = 0;
    for (long long i = 1; i <= n; i++) {
        sum += i;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    long long n = atoll(argv[1]);
    
    clock_t start = clock();
    long long result = sum_serial(n);
    clock_t end = clock();
    
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Sum of numbers from 1 to %lld = %lld\n", n, result);
    printf("Time taken: %f seconds\n", cpu_time_used);
    
    return 0;
}