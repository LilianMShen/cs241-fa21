// partners: lmshen2, jlwang5, justink6, dbargon2, jaym2, mjakim2
/**
 * savvy_scheduler
 * CS 241 - Fall 2021
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    int priority;

    double arrivalTime;
    double runTime;
    double tempStartTime;
    double remainingTime;
    double startTime;
} job_info;

int numJobs;
double totalWaitingTime;
double totalResponseTime;
double totalTurnAroundTime;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here

    numJobs = 0;
    totalWaitingTime = 0;
    totalResponseTime = 0;
    totalTurnAroundTime = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    job_info* aInfo = ((job*)a)->metadata;
    job_info* bInfo = ((job*)b)->metadata;

    if (aInfo->arrivalTime < bInfo->arrivalTime) {
        return -1;
    } else {
        return 1;
    }
}

int comparer_ppri(const void *a, const void *b) {
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    job_info* aInfo = ((job*)a)->metadata;
    job_info* bInfo = ((job*)b)->metadata;

    int res = aInfo->priority - bInfo->priority;
    if (res == 0) {
        return break_tie(a, b);
    } else if (res < 0) {
        return -1;
    } else {
        return 1;
    }
}

int comparer_psrtf(const void *a, const void *b) {
    job_info* aInfo = ((job*)a)->metadata;
    job_info* bInfo = ((job*)b)->metadata;

    int res = aInfo->remainingTime - bInfo->remainingTime;
    if (res == 0) {
        return break_tie(a, b);
    } else if (res < 0) {
        return -1;
    } else {
        return 1;
    }
}

int comparer_rr(const void *a, const void *b) {
    job_info* aInfo = ((job*)a)->metadata;
    job_info* bInfo = ((job*)b)->metadata;

    if (aInfo->tempStartTime < bInfo->tempStartTime) {
        return -1;
    } else {
        return 1;
    }
}

int comparer_sjf(const void *a, const void *b) {
    job_info* aInfo = ((job*)a)->metadata;
    job_info* bInfo = ((job*)b)->metadata;

    int res = aInfo->runTime - bInfo->runTime;
    if (res == 0) {
        return break_tie(a, b);
    } else if (res < 0) {
        return -1;
    } else {
        return 1;
    }
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    job_info* j = malloc(sizeof(job_info));

    j->id = job_number;
    j->priority = sched_data->priority;

    j->arrivalTime = time;
    j->runTime = sched_data->running_time;
    j->remainingTime = sched_data->running_time;
    j->startTime = -1;
    j->tempStartTime = -1;

    newjob->metadata = j;   
    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    if (job_evicted == NULL) {
        return priqueue_peek(&pqueue);
    } 

    job_info* j = job_evicted->metadata;
    j->tempStartTime = time;
    j->remainingTime -= 1;

    if (j->startTime < 0) {
        j->startTime = time - 1;
    }

    if (pqueue_scheme == RR || pqueue_scheme == PPRI || pqueue_scheme == PSRTF) {
        job* currJob = priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, currJob);
        return priqueue_peek(&pqueue);
    }

    return NULL;
}

void scheduler_job_finished(job *job_done, double time) {
    job_info* j = job_done->metadata;

    numJobs += 1;
    totalWaitingTime += time - j->arrivalTime - j->runTime;
    totalResponseTime += j->startTime - j->arrivalTime;
    totalTurnAroundTime += time - j->arrivalTime;

    free(j);
    priqueue_poll(&pqueue);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    return totalWaitingTime / numJobs;
}

double scheduler_average_turnaround_time() {
    return totalTurnAroundTime / numJobs;
}

double scheduler_average_response_time() {
    return totalResponseTime / numJobs;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}