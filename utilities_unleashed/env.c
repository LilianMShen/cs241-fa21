/**
 * utilities_unleashed
 * CS 241 - Fall 2021
 */

#include "format.h"
#include <unistd.h>
#include <sys/types.h>


int main(int argc, char *argv[]) {
    pid_t childOne = fork();

    if (childOne < 0) {
        print_fork_failed();
    }

    
    return 0;
}
