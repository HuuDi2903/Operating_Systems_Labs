#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

int finished = 0;
pthread_mutex_t lock;
int recovery_attempted = 0;

int is_safe() {
    // TODO: perform polling checks
    int found = 1; // Assume system is safe by default
    
    // Simulate a condition where the system becomes unsafe sometimes
    int simulated_failure = rand() % 2; 
    if (simulated_failure == 0) {
        found = 0;
    }
    
    if (!found) {
        return -1; // Unsafety detected
    }
    
    return 0; // System is safe
}

// Example of a recovery function
void recover_resources() {
    // TODO: Implement actual recovery logic here

    printf("Recovery action taken.\n");
}

int periodical_detector(void *arg) {
    while (1) {
        sleep(5); // Periodic check every 5 seconds
        pthread_mutex_lock(&lock);
        
        if (!is_safe()) {
            // Abnormal detected! Taking corrective action...
            printf("Abnormal detected! Taking corrective action...\n");
            
            recover_resources();  // Implement this function
            
            if (!finished) {
                break; // Break after terminating
            }
        }
        pthread_mutex_unlock(&lock);
    }
    return 0;
}

int main() {
    // Create the detector thread
    pthread_t detector_thread;
    pthread_mutex_init(&lock, NULL);
    
    if (pthread_create(&detector_thread, NULL, periodical_detector, NULL) != 0) {
        fprintf(stderr, "Error creating detector thread\n");
        return -1;
    }
    
    for (int i = 0; i < 10; ++i) {
        sleep(1);
        printf("Main application running... (%d)\n", i);
    }
    
    // When the main application is done
    finished = 1;
    
    pthread_join(detector_thread, NULL);
    
    pthread_mutex_destroy(&lock);
    
    return 0;
}