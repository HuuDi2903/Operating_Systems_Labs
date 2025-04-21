#include "mytpool.h"

// Worker thread function
void *worker_function(void *arg) {
    fork_join_pool_t *pool = (fork_join_pool_t *)arg;
    task_t *task = NULL;
    
    while (1) {
        // Get a task from the pool
        pthread_mutex_lock(&pool->pool_lock);
        
        // Wait for a task to become available
        while (pool->active && get_next_task(pool, &task) != 0) {
            pthread_cond_wait(&pool->task_available, &pool->pool_lock);
        }
        
        // If pool is shutting down and no tasks, exit
        if (!pool->active && get_next_task(pool, &task) != 0) {
            pthread_mutex_unlock(&pool->pool_lock);
            break;
        }
        
        // We have a task, mark it as running
        pthread_mutex_lock(&task->lock);
        task->status = TASK_RUNNING;
        pthread_mutex_unlock(&task->lock);
        
        pthread_mutex_unlock(&pool->pool_lock);
        
        // Execute the task
        void *result = task->function(task->arg);
        printf("Worker %ld: picked up task %d\n", pthread_self(), task->task_id);
        
        // Update task status and store result
        pthread_mutex_lock(&task->lock);
        task->result = result;
        task->status = TASK_COMPLETED;
        
        // If this task has a parent, update the parent's completed children count
        if (task->parent_id >= 0) {
            task_t *parent = &pool->tasks[task->parent_id];
            pthread_mutex_lock(&parent->lock);
            parent->completed_children++;
            
            // If all children of parent are completed, signal the parent
            if (parent->completed_children == parent->child_count) {
                pthread_cond_signal(&parent->completion_cond);
            }
            pthread_mutex_unlock(&parent->lock);
        }
        
        pthread_mutex_unlock(&task->lock);
    }
    
    return NULL;
}

// Get next available task
int get_next_task(fork_join_pool_t *pool, task_t **task) {
    for (int i = 0; i < pool->task_count; i++) {
        if (pool->tasks[i].status == TASK_WAITING) {
            *task = &pool->tasks[i];
            return 0;
        }
    }
    return -1; // No tasks available
}

  // Initialize the Fork-Join pool
  int fj_pool_init(fork_join_pool_t *pool, int num_workers) {
    if (num_workers > MAX_WORKERS) {
        num_workers = MAX_WORKERS;
    }
    
    // Initialize pool properties
    pool->task_count = 0;
    pool->worker_count = num_workers;
    pool->active = 1;
    
    // Initialize synchronization primitives
    pthread_mutex_init(&pool->pool_lock, NULL);
    pthread_cond_init(&pool->task_available, NULL);
    
    // Initialize tasks
    for (int i = 0; i < MAX_TASKS; i++) {
        pool->tasks[i].task_id = i;
        pool->tasks[i].status = TASK_WAITING;
        pool->tasks[i].parent_id = -1;
        pool->tasks[i].child_count = 0;
        pool->tasks[i].completed_children = 0;
        pthread_mutex_init(&pool->tasks[i].lock, NULL);
        pthread_cond_init(&pool->tasks[i].completion_cond, NULL);
    }
    
    // Create worker threads
    for (int i = 0; i < num_workers; i++) {
        pthread_create(&pool->workers[i], NULL, worker_function, pool);
    }
    
    return 0;
}

// Shutdown the Fork-Join pool
int fj_pool_shutdown(fork_join_pool_t *pool) {
    // Set pool to inactive
    pthread_mutex_lock(&pool->pool_lock);
    pool->active = 0;
    pthread_cond_broadcast(&pool->task_available); // Wake up all workers
    pthread_mutex_unlock(&pool->pool_lock);
    
    // Wait for all workers to finish
    for (int i = 0; i < pool->worker_count; i++) {
        pthread_join(pool->workers[i], NULL);
    }
    
    // Cleanup synchronization primitives
    pthread_mutex_destroy(&pool->pool_lock);
    pthread_cond_destroy(&pool->task_available);
    
    for (int i = 0; i < MAX_TASKS; i++) {
        pthread_mutex_destroy(&pool->tasks[i].lock);
        pthread_cond_destroy(&pool->tasks[i].completion_cond);
    }
    
    return 0;
}