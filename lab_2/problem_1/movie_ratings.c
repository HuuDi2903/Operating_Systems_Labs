#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_MOVIES 1682  

// Shared memory structure
struct movie_data {
    int counts[MAX_MOVIES + 1];    // Count of ratings per movie
    double sums[MAX_MOVIES + 1];   // Sum of ratings per movie
};

void process_file(const char *filename, struct movie_data *data) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    int userID, movieID, rating;
    long timestamp;
    
    while (fscanf(file, "%d\t%d\t%d\t%ld", &userID, &movieID, &rating, &timestamp) == 4) {
            data->counts[movieID]++;
            data->sums[movieID] += rating;
    }
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    // Calculate required shared memory size
    size_t shm_size = sizeof(struct movie_data);
    // printf("Shared memory size needed: %ld bytes\n", shm_size);

    // Round up to page size (often 4KB)
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t adjusted_size = ((shm_size + page_size - 1) / page_size) * page_size;
    // printf("Adjusted size: %ld bytes (page size: %ld)\n", adjusted_size, page_size);

    // Use ftok to generate a unique key
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    
    // Create shared memory
    int shmid = shmget(key, adjusted_size, IPC_CREAT | 0644);
    if (shmid < 0) {
        printf("shmget failed: %s (errno: %d)\n", strerror(errno), errno);
        }
    // printf("Shared memory created with ID: %d\n", shmid);
    
    // Attach shared memory
    struct movie_data *shared_data = shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    // Initialize shared memory to zeros
    memset(shared_data, 0, shm_size);
    
    // Fork first child process
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid1 == 0) {
        // Child process 1
        printf("Child 1 processing file: %s\n", argv[1]);
        process_file(argv[1], shared_data);
        exit(0);
    }
    
    // Fork second child process
    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid2 == 0) {
        // Child process 2
        printf("Child 2 processing file: %s\n", argv[2]);
        process_file(argv[2], shared_data);
        exit(0);
    }
    
    // Parent process waits for children
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    
    // Print results
    printf("\nMovie ID\tAverage Rating\tRating Count\n");
    printf("-----------------------------------------\n");
    
    int displayed = 0;
    for (int i = 1; i <= MAX_MOVIES; i++) {
        if (shared_data->counts[i] > 0) {
            double avg = shared_data->sums[i] / shared_data->counts[i];
            printf("%d\t\t%.2f\t\t%d\n", i, avg, shared_data->counts[i]);
            displayed++;
            
            // // Limit output for readability
            // if (displayed >= 100) {
            //     printf("... (showing first 100 results only)\n");
            //     break;
            // }
        }
    }
    
    // Detach shared memory
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
        exit(1);
    }
    
    // Remove shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
    
    return 0;
}