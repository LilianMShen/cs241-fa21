/**
 * utilities_unleashed
 * CS 241 - Fall 2021
 */

#include "format.h"
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_time_usage();
    }

    pid_t pid = fork();

    if (pid < 0) { // fork failure
        print_fork_failed();
    }

    struct timespec before, after;
    clock_gettime(0, &before);

    if (pid == 0) { // do child function
        char * path = "/bin/";
        strcat(path, argv[0]);
        execv(path , &argv[1]);
        // exit(EXIT_FAILURE); // For safety.
        
    }

    if (pid > 0) { // do parent function
        int status;
        waitpid(-1, &status, 0);
        if (WIFEXITED(status)) {
            print_exec_failed();
        }

        clock_gettime(0, &after);
        double time = ((after.tv_sec - before.tv_sec));
        display_results(argv, time);
    }
    return 0;
}
