/**
 * utilities_unleashed
 * CS 241 - Fall 2021
 * 
 * lab partner - Justin Wang jlwang5
 */

#include "format.h"
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

extern char** environ;

char *string_slice(char *this, int start, int end) {
    char *ret = malloc(sizeof(char) * (end - start + 1));
    int i;
    for (i = start; i < end; i++) {
        ret[i - start] = this[i];
    }
    ret[end - start] = '\0';
    return ret;
}

int main(int argc, char *argv[]) {
    // Must contain ./env -- [command] at the minimum
    if (argc <= 2) {
        print_env_usage();
    }

    // Fork and execute
    int status;
    pid_t child = fork();
    if (child == -1) {
        print_fork_failed();
        return 1;
    }
    if (child == 0) {
        // Parse variables
        int i;
        for (i = 1; argv[i] != NULL; i++) {
            if (strcmp(argv[i], "--") == 0) {
                i++;
                break;
            } else {
                // Check if variable assignment has '='
                if (!strstr(argv[i], "=")) {
                    print_env_usage();
                    return 1;
                }
                // Set variable
                char *var = strtok(argv[i], "=");
                char *val = strtok(NULL, "=");

                char *trueVal = malloc(sizeof(char) * (strlen(val) + 1));
                trueVal[0] = '\0';
                size_t temp = strlen(val);

                // Loop through the string and populate along with replacement
                size_t j;
                int last = 0;
                for (j = 0; j < strlen(val); j++) {
                    if (val[j] == '%') {
                        // Get substitution slice
                        char *substr = string_slice(val, last, j);
                        
                        if (substr[0] == '%') {
                            // Get environmental variable associated
                            char* subval = getenv(substr + 1);
                            if (subval == NULL) {
                                subval = "";
                            }

                            // Allocate extra memory if needed
                            if (strlen(subval) > strlen(substr)) {

                                trueVal = realloc(trueVal, sizeof(char) * (temp + (strlen(subval) - strlen(substr) + 1)));
                                temp += (strlen(subval) - strlen(substr));
                            }
                            strcat(trueVal, subval);
                        } else {
                            strcat(trueVal, substr);
                        }
                        // Destroy to prevent memory leak
                        free(substr);
                        last = j;
                    }
                }

                // Get substitution slice
                char *substr = string_slice(val, last, j);
                
                if (substr[0] == '%') {
                    // Get environmental variable associated
                    char* subval = getenv(substr + 1);
                    if (subval == NULL) {
                        subval = "";
                    }

                    // Allocate extra memory if needed
                    if (strlen(subval) > strlen(substr)) {
                        trueVal = realloc(trueVal, sizeof(char) * (temp + 1 + (strlen(subval) - strlen(substr))));
                    }
                    strcat(trueVal, subval);
                } else {
                    strcat(trueVal, substr);
                }
                // Destroy to prevent memory leak
                free(substr);
                
                if (setenv(var, trueVal, 1) == -1) {
                    print_environment_change_failed();
                    return 1;
                }

                free(trueVal);
                
            }
            if (argv[i + 1] == NULL) {
                // Did not find "--" thus fail
                print_env_usage();
                return 1;
            }
        }

        // Should have broken out if and only if found a --, thus no command exists if NULL at i
        if (argv[i] == NULL) {
            print_env_usage();
            return 1;
        }

        // Extract out the command
        char com_path[strlen(argv[i]) + strlen("/bin/") + 1];
        strcpy(com_path, "/bin/");
        strcat(com_path, argv[i]);

        // Execute 
        if (execv(com_path, argv + i) == -1) {
            print_exec_failed();
        }
        
    } else {
        // Wait for child
        wait(&status);
        if (WIFEXITED(status)) {

        } else {
            print_exec_failed();
        }
    }
}