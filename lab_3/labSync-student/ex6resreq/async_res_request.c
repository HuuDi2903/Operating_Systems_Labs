#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_RESOURCES 3

typedef struct {
    int id;                    // Process ID
    int requested_resources;   // Resources requested
    void (*callback)(int);     // Callback function for resource allocation
} process_request_t;

int available_resources = NUM_RESOURCES;
pthread_mutex_t resource_lock;
pthread_cond_t resource_cond;

void resource_callback(int process_id) {
    printf("Resource allocated to process %d\n", process_id);
}

void* resource_manager(void* arg) {
    process_request_t* request = (process_request_t*)arg;

    pthread_mutex_lock(&resource_lock);

    while (request->requested_resources > available_resources) {
        printf("Process (%d) is waiting for: %d\n", request->id, request->requested_resources);
        pthread_cond_wait(&resource_cond, &resource_lock);
    }

    available_resources -= request->requested_resources;
    request->callback(request->id);
    pthread_mutex_unlock(&resource_lock);

    // TODO: Perform process
    // Simulate some processing time
    printf("Process (%d) is processing: %d\n", request->id, request->requested_resources);
    sleep(3);

    pthread_mutex_lock(&resource_lock);
    available_resources += request->requested_resources;
    printf("Process (%d) released: %d\n", request->id, request->requested_resources);
    sleep(3);

    pthread_cond_broadcast(&resource_cond);
    pthread_mutex_unlock(&resource_lock);
    return NULL;
}

int main() {
  pthread_mutex_init(&resource_lock, NULL);
  pthread_cond_init(&resource_cond, NULL);

  process_request_t requests[] = {
    {1, 2, resource_callback},
    {2, 3, resource_callback},
    {3, 2, resource_callback},
    {4, 1, resource_callback},
    {5, 3, resource_callback}, 
  };

  pthread_t threads[sizeof(requests)/sizeof(requests[0])];

  for (int i = 0; i < sizeof(requests)/sizeof(requests[0]); ++i) {
      pthread_create(&threads[i], NULL, resource_manager, &requests[i]);
  }

  for (int i = 0; i < sizeof(requests)/sizeof(requests[0]); ++i) {
      pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&resource_lock);
  pthread_cond_destroy(&resource_cond);

  return 0;
}