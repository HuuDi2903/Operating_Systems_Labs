// utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "utils.h"

struct _appconf appconf;  // Actual definition of the variable

int processopts(int argc, char **argv, struct _appconf *conf) {
    // Default values
    conf->seednum = SEEDNO;
    
    if (argc >= 2) {
        if (tonum(argv[1], &conf->arrsz) < 0 || conf->arrsz <= 0) {
            fprintf(stderr, "Error: Invalid array size\n");
            help(EXIT_FAILURE);
        }
    } else {
        help(EXIT_FAILURE);
    }
    
    if (argc >= 3) {
        if (tonum(argv[2], &conf->tnum) < 0 || conf->tnum <= 0) {
            fprintf(stderr, "Error: Invalid thread count\n");
            help(EXIT_FAILURE);
        }
    } else {
        help(EXIT_FAILURE);
    }
    
    if (argc >= 4) {
        if (tonum(argv[3], &conf->seednum) < 0) {
            fprintf(stderr, "Error: Invalid seed number\n");
            help(EXIT_FAILURE);
        }
    }
    
    return 0;
}

int tonum(const char *nptr, int *num) {
    char *endptr;
    long val;
    
    errno = 0;  // Reset errno before the call
    
    val = strtol(nptr, &endptr, 10);
    
    // Check for various possible errors
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || 
        (errno != 0 && val == 0)) {
        return -1;
    }
    
    // No digits were found
    if (endptr == nptr) {
        return -1;
    }
    
    // If we got here, strtol() successfully parsed a number
    *num = (int)val;
    return 0;
}

int validate_and_split_argarray(int arraysize, int num_thread, struct _range* thread_idx_range) {
    if (arraysize < num_thread * THRSL_MIN) {
        return -1; // Array too small to be divided meaningfully
    }
    
    int items_per_thread = arraysize / num_thread;
    int remainder = arraysize % num_thread;
    
    int start = 0;
    for (int i = 0; i < num_thread; i++) {
        thread_idx_range[i].start = start;
        
        // Distribute remainder elements among the first 'remainder' threads
        int extra = (i < remainder) ? 1 : 0;
        
        thread_idx_range[i].end = start + items_per_thread - 1 + extra;
        
        // Set the starting index for the next thread
        start = thread_idx_range[i].end + 1;
    }
    
    return 0;
}

int generate_array_data(int* buf, int arraysize, int seednum) {
    if (buf == NULL || arraysize <= 0) {
        return -1;
    }
    
    // Set the seed for randomization
    srand(seednum);
    
    for (int i = 0; i < arraysize; i++) {
        // Generate a random number between LWBND_DATA_VAL and UPBND_DATA_VAL
        buf[i] = LWBND_DATA_VAL + rand() % (UPBND_DATA_VAL - LWBND_DATA_VAL + 1);
    }
    
    return 0;
}

void help(int xcode) {
    fprintf(stderr, "Usage: %s %s %s [%s]\n", PACKAGE, ARG1, ARG2, ARG3);
    fprintf(stderr, "  %s: Size of the array to process\n", ARG1);
    fprintf(stderr, "  %s: Number of threads to use\n", ARG2);
    fprintf(stderr, "  %s: (Optional) Seed for random number generation (default: %d)\n", ARG3, SEEDNO);
    exit(xcode);
}