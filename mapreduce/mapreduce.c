/**
 * mapreduce
 * CS 241 - Fall 2021
 */

#include "utils.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    // Create an input pipe for each mapper.
    int numMaps = atoi(argv[5]);

    // Let fds[i < num_maps] be for child i
    // Let fds[num_maps] be pipe of reducer
    int fds[numMaps + 1][2];

    for (int i = 0; i < numMaps + 1; i++) {
        pipe(fds[i]);
    }

    // Open the output file.
    int out = open(argv[2], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

    // Start a splitter process for each mapper.
    pid_t splitterChild[numMaps];
    for (int i = 0; i < numMaps; i++) {
        // Fork
        pid_t child = fork();
        splitterChild[i] = child;

        if (child == 0) {
            dup2(fds[i][1], 1);

            char num[10];
            sprintf(num, "%d", i);

            execl("./splitter", "./splitter", argv[1], argv[5], num, NULL);
            exit(1);
        }
    }

    // Start all the mapper processes.
    pid_t mapChild[numMaps];
    for (int i = 0; i < numMaps; i++) {
        close(fds[i][1]);

        // Fork
        pid_t child = fork();
        mapChild[i] = child;

        if (child == 0) {
            dup2(fds[i][0], 0);
            dup2(fds[numMaps][1], 1);

            execl(argv[3], argv[3], NULL);
            exit(1);
        }
    }

    // Start the fds[numMaps]r process.
    close(fds[numMaps][1]);

    pid_t child = fork();
    if (child == 0) {
        dup2(fds[numMaps][0], 0);
        dup2(out, 1);

        execl(argv[4], argv[4], NULL);
        exit(1);
    }

    // Wait for the reducer to finish.

    // Print nonzero subprocess exit codes.

    // Count the number of lines in the output file.

    return 0;
}