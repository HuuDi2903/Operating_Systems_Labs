#include "mytpool.h"
#include <stdio.h>
#include <stdlib.h>

// Argument struct for Fibonacci computation
typedef struct {
    int n;
    fork_join_pool_t *pool;
    int parent_id;
} fib_arg_t;

// Fibonacci function for the Fork-Join Pool
void *fibonacci(void *arg) {
    fib_arg_t *fib_args = (fib_arg_t *)arg;
    int n = fib_args->n;
    fork_join_pool_t *pool = fib_args->pool;
    int parent_id = fib_args->parent_id;
    int *result = malloc(sizeof(int));

    if (!result) return NULL;

    if (n <= 1) {
        *result = n;
    } else {
        // Prepare arguments for child tasks
        fib_arg_t *arg1 = malloc(sizeof(fib_arg_t));
        fib_arg_t *arg2 = malloc(sizeof(fib_arg_t));
        if (!arg1 || !arg2) return NULL;

        arg1->n = n - 1;
        arg1->pool = pool;
        arg1->parent_id = parent_id; // Could use current task's id, if tracked

        arg2->n = n - 2;
        arg2->pool = pool;
        arg2->parent_id = parent_id;

        // Fork two child tasks
        int task1_id = fj_fork(pool, parent_id, fibonacci, arg1);
        int task2_id = fj_fork(pool, parent_id, fibonacci, arg2);

        // Join with child tasks
        int *res1 = (int *)fj_join(pool, task1_id);
        int *res2 = (int *)fj_join(pool, task2_id);

        *result = *res1 + *res2;

        // Clean up
        free(res1);
        free(res2);
        free(arg1);
        free(arg2);
    }
    return result;
}

int main() {
    fork_join_pool_t pool;
    fj_pool_init(&pool, 4); // Start pool with 4 workers

    int n = 10; // Calculate Fibonacci(n)
    fib_arg_t *arg = malloc(sizeof(fib_arg_t));
    arg->n = n;
    arg->pool = &pool;
    arg->parent_id = -1; // Root task has no parent

    int task_id = fj_fork(&pool, -1, fibonacci, arg);
    int *result = (int *)fj_join(&pool, task_id);

    printf("Fibonacci(%d) = %d\n", n, *result);

    free(result);
    free(arg);

    fj_pool_shutdown(&pool);

    return 0;
}