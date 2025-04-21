#include "mytpool.h"

// Fork a new task
int fj_fork(fork_join_pool_t *pool, int parent_id, void *(*function)(void *), void *arg) {
  pthread_mutex_lock(&pool->pool_lock);
  
  // Get a new task ID
  int task_id = pool->task_count++;
  if (task_id >= MAX_TASKS) {
      pthread_mutex_unlock(&pool->pool_lock);
      return -1; // Too many tasks
  }
  
  // Initialize the task
  task_t *task = &pool->tasks[task_id];
  task->function = function;
  task->arg = arg;
  task->result = NULL;
  task->status = TASK_WAITING;
  task->parent_id = parent_id;
  task->child_count = 0;
  task->completed_children = 0;
  
  // Update parent's child count
  if (parent_id >= 0) {
      task_t *parent = &pool->tasks[parent_id];
      pthread_mutex_lock(&parent->lock);
      parent->child_count++;
      pthread_mutex_unlock(&parent->lock);
  }
  
  // Signal that a task is available
  pthread_cond_signal(&pool->task_available);
  pthread_mutex_unlock(&pool->pool_lock);
  
  return task_id;
}

// Join with a task (wait for it to complete)
void *fj_join(fork_join_pool_t *pool, int task_id) {
  task_t *task = &pool->tasks[task_id];
  void *result;
  
  pthread_mutex_lock(&task->lock);
  
  // Wait for the task to complete
  while (task->status != TASK_COMPLETED && task->status != TASK_FAILED) {
      pthread_cond_wait(&task->completion_cond, &task->lock);
  }
  
  result = task->result;
  pthread_mutex_unlock(&task->lock);
  
  return result;
}