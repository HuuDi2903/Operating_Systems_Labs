#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>

// ----- Task structure -----
typedef struct task {
    void *(*function)(void *);
    void *arg;
    void *result;
    pthread_t thread;
    int completed;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} task_t;

// ----- Array structure -----
typedef struct {
  int *arr;
  int left, right;
} sum_arg_t;

// Worker function
void *run_task(void *targ) {
    task_t *task = (task_t *)targ;
    task->result = task->function(task->arg);

    pthread_mutex_lock(&task->lock);
    task->completed = 1;
    pthread_cond_signal(&task->cond);
    pthread_mutex_unlock(&task->lock);
    return task->result;
}

// Fork: Create and launch a new task
void fork_task(task_t *task, void *(*function)(void *), void *arg) {
    task->function = function;
    task->arg = arg;
    task->completed = 0;
    pthread_mutex_init(&task->lock, NULL);
    pthread_cond_init(&task->cond, NULL);
    pthread_create(&task->thread, NULL, run_task, task);
}

// Join: Wait for the task to finish and get result
void *join_task(task_t *task) {
    pthread_mutex_lock(&task->lock);
    while (!task->completed) {
        pthread_cond_wait(&task->cond, &task->lock);
    }
    pthread_mutex_unlock(&task->lock);
    return task->result;
}

// Function to sum elements in an array
void *worker_sum(void *arg) {
  sum_arg_t *arr_arg = (sum_arg_t*)arg;
  int *res = malloc(sizeof(int));
  if (arr_arg->left == arr_arg->right) {
      *res = arr_arg->arr[arr_arg->left];
  } else {
      int mid = (arr_arg->left + arr_arg->right) / 2;
      sum_arg_t a1 = { arr_arg->arr, arr_arg->left, mid };
      sum_arg_t a2 = { arr_arg->arr, mid+1, arr_arg->right };
      task_t t1, t2;
      fork_task(&t1, worker_sum, &a1);
      fork_task(&t2, worker_sum, &a2);
      int *v1 = join_task(&t1);
      int *v2 = join_task(&t2);
      *res = *v1 + *v2;
      free(v1); free(v2);
  }

  // printf("Worker %ld is working: Sum from %d to %d is %d\n", pthread_self(), arr_arg->left, arr_arg->right, *res);

  return res;
}

// Function to generate a random array
void random_array(int *arr, int n) {
  srand(time(NULL));
  for (int i = 0; i < n; ++i)
      arr[i] = rand() % 100; 
}

// Function to check the result
void validate_sum(int *arr, int size_array, int expected_sum) {
  int actual_sum = 0;
  for (int i = 0; i < size_array; ++i)
      actual_sum += arr[i];
  if (actual_sum == expected_sum)
      printf("Validation successful: %d == %d\n", actual_sum, expected_sum);
  else
      printf("Validation failed: %d != %d\n", actual_sum, expected_sum);
}

// ----- Example task: Sum of an array -----
int main() {
  int N = 20; // Size of the array
  int arr[N];
  random_array(arr, N);

  printf("Array: ");
  for (int i = 0; i < N; ++i)
      printf("%d ", arr[i]);
  printf("\n");

  sum_arg_t root = { arr, 0, N-1 };
  task_t t;
  fork_task(&t, worker_sum, &root);
  int *sum = join_task(&t);

  validate_sum(arr, N, *sum);

  free(sum);
  pthread_mutex_destroy(&t.lock);
  pthread_cond_destroy(&t.cond);
  return 0;
}