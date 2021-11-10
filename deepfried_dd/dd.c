// partners: lmshen2, jlwang5, dbargon2, justink6

/**
 * deepfried_dd
 * CS 241 - Fall 2021
 */
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "format.h"

extern char * optarg;
extern int optind;

static bool read_partial = false;
static bool wrote_partial = false;
static size_t blocks_read = 0;
static size_t blocks_written = 0;
static size_t total_bytes_copied = 0;
static double init_time;

void print_status();

void curr_status(int sig) {
    // signal(sig, SIG_IGN);
    print_status();
}

void print_status() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    double curr_time = t.tv_sec + (double)t.tv_nsec / 1000000000.0;
    print_status_report(blocks_read, read_partial, blocks_written, wrote_partial, total_bytes_copied, (curr_time - init_time));
}

int main(int argc, char **argv) {
    int opt;
    char *inputFile = NULL, *outputFile = NULL, *bSize = NULL, *cCount = NULL, *pCount = NULL, *kCount = NULL;
    // Get initial time
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    init_time = t.tv_sec + (double)t.tv_nsec / 1000000000.0;

    while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch(opt) {
            case 'i':   
                inputFile = strdup(optarg);
                // optarg = textfile
                break;
            case 'o':
                outputFile = strdup(optarg); 
                break;
            case 'b':
                bSize = strdup(optarg); 
                break;
            case 'c':
                cCount = strdup(optarg); 
                break;
            case 'p':
                pCount = strdup(optarg); 
                break;
            case 'k':
                kCount = strdup(optarg); 
                break;
        }
    }
    if (optind < argc) {
        return 1;
    }

    size_t b = 512;
    if(bSize) b = (size_t) atoi(bSize);
    long c = -1; // -1 means go to the end of file, no restricted # of blocks copied
    if (cCount) c = atol(cCount);
    long p = 0;
    if (pCount) p = atol(pCount);
    long k = 0;
    if (kCount) k = atol(kCount);

    FILE * input;
    if (inputFile) {
        input = fopen(inputFile, "r");
        if (input == NULL) {
            print_invalid_input(inputFile);
            return 1;
        }
        fseek(input, b * p, SEEK_SET);
    } else {
        input = stdin;
    }

    FILE * output;
    if (outputFile) { 
        output = fopen(outputFile, "w+");
        if (output == NULL) {
            print_invalid_input(outputFile);
            return 1;
        }
        fseek(output, b * k, SEEK_SET);
    } else {
        output = stdout;
    }

    signal(SIGUSR1, curr_status);
    char buf[b];
    if (c == -1) {
        size_t bytes_read = fread(buf, 1, b, input);
        while (bytes_read) {
            signal(SIGUSR1, curr_status);
            if (bytes_read == 0) {
                break;
            }
            
            if (bytes_read != b) {
                read_partial = true;
            } else {
                blocks_read++;
            }

            fwrite(buf, 1, bytes_read, output);
            total_bytes_copied += bytes_read;
            if (bytes_read != b) {
                wrote_partial = true;
            } else {
                blocks_written++;
            }
            bytes_read = fread(buf, 1, b, input);
        }
    } else {
        long i;
        size_t bytes_read = fread(buf, 1, b, input);
        for (i = 0; i < c; i++) {
            signal(SIGUSR1, curr_status);
            if (bytes_read == 0) {
                break;
            }

            if (bytes_read != b) {
                read_partial = true;
            } else {
                blocks_read++;
            }

            fwrite(buf, 1, bytes_read, output);
            total_bytes_copied += bytes_read;
            if (bytes_read != b) {
                wrote_partial = true;
            } else {
                blocks_written++;
            }
            bytes_read = fread(buf, 1, b, input);
        }
    }
    print_status();

}