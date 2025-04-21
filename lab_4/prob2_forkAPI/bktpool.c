#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include "bktpool.h"

struct bkworker_t *shared_workers = NULL;
int *shared_busy_flags = NULL;

int bktpool_init()
{
    // Allocate shared memory for workers
    shared_workers = (struct bkworker_t *)mmap(
        NULL,
        sizeof(struct bkworker_t) * MAX_WORKER,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);
    if (shared_workers == MAP_FAILED) {
        perror("mmap shared_workers");
        return -1;
    }

    // Allocate shared memory for busy flags
    shared_busy_flags = (int *)mmap(
        NULL,
        sizeof(int) * MAX_WORKER,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);
    if (shared_busy_flags == MAP_FAILED) {
        perror("mmap shared_busy_flags");
        return -1;
    }

    // Zero-initialize arrays
    memset(shared_workers, 0, sizeof(struct bkworker_t) * MAX_WORKER);
    memset(shared_busy_flags, 0, sizeof(int) * MAX_WORKER);

    return bkwrk_create_worker();
}