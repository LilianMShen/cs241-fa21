/**
 * teaching_threads
 * CS 241 - Fall 2021
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
struct funcAndInputs {
    reducer reduce_func;
    int numArgs;
    int * args;
    int output;
};

/* You should create a start routine for your threads. */
void * start_routine(void * input) {
    struct funcAndInputs * thread = (struct funcAndInputs *) input;
    // thread->output = malloc(sizeof(int));
    int o = thread->args[0];
    for (int i = 1; i < thread->numArgs; i++) {
        o = thread->reduce_func(o, thread->args[i]);
    }
    thread->output = o;
    return thread;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (num_threads == 0) {
        return base_case;
    }

    // if num_threads > list_len, then the max num of threads is list_len
    // the +1 is because we treat base case like its part of list
    if (num_threads > list_len + 1) num_threads = list_len + 1;


    pthread_t * threads = malloc(sizeof(pthread_t *) * num_threads); // keep track of thread ids
    struct funcAndInputs * threadInfo = malloc(sizeof(struct funcAndInputs) * num_threads); // keep track of all the inputs and input func

    int remainder = (list_len + 1) % num_threads;
    int numArgs = ((list_len + 1) - remainder) / num_threads;
    
    int count = 0;
    int f = 0; // list iterator
    for (size_t i = 0; i < (list_len + 1) - remainder; i += numArgs) {
        threadInfo[count].reduce_func = reduce_func;
        threadInfo[count].numArgs = numArgs;
        threadInfo[count].args=malloc(sizeof(int) * numArgs);
        if (i == 0) {
            threadInfo[count].args[0] = base_case;
            for (int j = 1; j < numArgs; j++) {
                threadInfo[count].args[j] = list[f];
                f++;
            }
            count++;
        } else {
            for (int j = 0; j < numArgs; j++) {
                threadInfo[count].args[j] = list[f];
                f++;
            }
            count++;
        }
    }

    // catch remainders
    threadInfo[num_threads - 1].numArgs = numArgs + remainder; // last one accessed
    threadInfo[num_threads-1].args = realloc(threadInfo[count-1].args, (numArgs + remainder) * sizeof(int));
    f -= numArgs;
    for (int i = 0; i < numArgs + remainder; i++) {
        threadInfo[num_threads - 1].args[i] = list[f];
        f++;
    }

    for (size_t i= 0; i < num_threads; i++) {
        int createErr = pthread_create(&threads[i], NULL, start_routine, &threadInfo[i]);
        if (createErr) {
            printf("error hit at thread %d\n", (int) i);
        }
    }

    // int ** retvals = malloc(sizeof(int*) * num_threads);

    for (size_t i= 0; i < num_threads; i++) {
        int joinErr = pthread_join(threads[i], NULL);
        if (joinErr) {
            printf("error hit at thread %d\n", (int) i);
        }
    }

    int output = threadInfo[0].output;
    for (size_t i = 1; i < num_threads; i++) {
        output = reduce_func(output, threadInfo[i].output);
    }

    for (size_t i = 0; i < num_threads; i++) {
        // free(threads[i]);
        if (threadInfo[i].args != NULL) {
            free(threadInfo[i].args);
        }
        // free(threadInfo[i].output);
        // free(retvals[i]);
    }
    free(threads);
    free(threadInfo);
    // free(retvals);

    return output;
    // return 0;
}
