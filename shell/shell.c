/**
 * shell
 * CS 241 - Fall 2021
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

// to get rid of implicit declaration
void runCmd(vector*h, vector * pids, char*line);

// handles ctrl c
void handle_sigint(int sig) {
    // @TODO kill child processes
    return;
}

// helper function to get user input
char * getInput(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    linep[strlen(linep) - 1] = '\0';
    return linep;
}

// logic for handling built in functions
int isBuiltIn(vector * v, vector * h, vector * pids) {
    if (vector_empty(v)) {
        return 0;
    }

    char * firstWord = vector_get(v, 0);

    if (strcmp(firstWord, "cd") == 0) {
        if(vector_size(v) < 2) {
            print_no_directory("");
            return -1;
        }
        if(chdir(vector_get(v, 1)) != 0) {
            print_no_directory(vector_get(v, 1));
            return -1;
        }
        return 1;
    } else if (strcmp(firstWord, "!history") == 0) {
        vector_erase(h, vector_size(h) - 1);
        size_t i = 0;
        while (i < vector_size(h)) {
            print_history_line(i, vector_get(h, i));
            i++;
        }
        return 1;
    } else if (firstWord[0] == '#') {
        vector_erase(h, vector_size(h) - 1);
        int index = atoi(firstWord + 1);
        if (index >= (int) vector_size(h)) {
            print_invalid_index();
            return -1;
        }
        char * l = vector_get(h, index);
        print_history_line(index, l);
        vector_push_back(h, l);
        runCmd(h, pids, l);
        return 1;
    } else if (firstWord[0] == '!') {
        vector_erase(h, vector_size(h) - 1);
        char * l = vector_get(h, 0);
        // if prefix is empty
        if (strlen(firstWord) == 1) {
            print_history_line(0, l);
            vector_push_back(h, l);
            runCmd(h, pids, l);
            return 1;
        }
        char prefix = firstWord[1];
        
        for (size_t i = 0; i < vector_size(h); i++) {
            char* l = vector_get(h, i);
            if (l[0] == prefix) {
                print_history_line(i, l);
                vector_push_back(h, l);
                runCmd(h, pids, l);
                return 1;
            }
        }
        print_no_history_match();
        return -1;
    }
    return 0;
}

// logic for handling fork exec wait functions
int isNotBuiltIn(vector* v, vector* h, vector * pids, int isBackground, int redirectFlag, vector * splicedLineTwo) {
    if (vector_empty(v)) {
        return 0;
    }

    pid_t childPid = fork();

    if (childPid < 0) { // fork failure
        print_fork_failed();
    }

    char** argv = (char**) vector_front(v);
    pid_t curpid = getpid();

    int filefd = 0;
    if (childPid == 0) { // do child function
        char path[strlen(argv[0]) + 6];
        strcpy(path, "/bin/");
        strcat(path, argv[0]);
        print_command_executed(curpid);

        if (redirectFlag != 0) {
            char * fileName = vector_get(splicedLineTwo, 0);
            // output
            if (redirectFlag == 2) filefd = open(fileName, O_CREAT|O_WRONLY, 0777);
            // append
            if (redirectFlag == 1) filefd = open(fileName, O_CREAT|O_APPEND|O_WRONLY, 0777);
            // input
            if (redirectFlag == 3) filefd = open(fileName, O_RDONLY, 0777);
            if (filefd == -1) {
                print_redirection_file_error();
                exit(1);
                return -1;
            }
            dup2(filefd, 1);
        }

        if (execvp(path , argv) == -1) {
            print_exec_failed(vector_get(h, vector_size(h) - 1));
            exit(1);
            return -1;
        }
    }

    // aka foreground processes
    if (childPid > 0 && isBackground == 0) { // do parent function
        int status;
        wait(&status);
        // DO NOT UNCOMMENT, THIS IS A FORK BOMB
        // if (redirectFlag != 0) {
        //     close(filefd);
        // }
        if (!WIFEXITED(status)) {
            print_exec_failed(vector_get(h, vector_size(h) - 1));
            exit(1);
            return -1;
        }
    } else if (childPid > 0 && isBackground == 1) {
        // printf("hit background process \n");
        int pgid = setpgid(0, 0);
        vector_push_back(pids, curpid);
    }

    return 0;
}

// logic for finding if user input contains an operator, returns index of operator
int containsOperator(vector* splicedLine) {
    for (int i = 0; i < (int) vector_size(splicedLine); i++) {
        char * word = (vector_get(splicedLine, i));
        if ((strcmp(word, "||") == 0) || (strcmp(word, "&&") == 0)) {
            return i;
        }

        if ((strcmp(word, ">>") == 0) || (strcmp(word, ">") == 0) || (strcmp(word, "<") == 0)) {
            return i;
        }

        if (word[strlen(word) - 1] == ';') {
            word[strlen(word) - 1] = '\0';
            printf("%s", word);
            return i + 1;
        }
    }
    return -1;
}

// splices line where operator is, effectively making it two separate lines
void newSplicedLine(vector* splicedLineTwo, vector* splicedLine, int index) {
    size_t i = (size_t) index;
    while (i < vector_size(splicedLine)) {
        vector_push_back(splicedLineTwo, vector_get(splicedLine, i));
        vector_erase(splicedLine, i);
    }
}

// void overwriteFile(vector * pids) {
//     FILE * file;
// }

// void killChild(vector * v, vector * pids, char * line) {
    
//     if (vector_size(v) < 2) ;
//     pid_t pid = vector_get(v, 1);
//     kill(pid, SIGKILL);
//     if (errno == ESRCH) {
//         print_no_process_found(pid); 
//     } else {
//         print_killed_process(pid, line);
//     }
// }

// runs the command(s) given a line
void runCmd(vector* h, vector * pids, char*line) {
    int orFlag = 0;
    int andFlag = 0;
    int semiFlag = 0;
    int redirectFlag = 0;
    // int outputFlag = 0;
    // int appendFlag = 0;
    // int inputFlag = 0;
    int suc1a = 0;
    int suc2a = 0;

    sstring * cmds = cstr_to_sstring(line);
    vector * splicedLine = sstring_split(cmds, ' ');
    int operatorIndex = containsOperator(splicedLine);
    vector * splicedLineTwo = string_vector_create();
    if (operatorIndex != -1) {
        newSplicedLine(splicedLineTwo, splicedLine, operatorIndex);
        char * operator = vector_get(splicedLineTwo, 0);

        if (strcmp(operator, "||") == 0) {
            orFlag = 1;
            vector_erase(splicedLineTwo, 0);
        } else if (strcmp(operator, "&&") == 0) {
            andFlag = 1;
            vector_erase(splicedLineTwo, 0);
            
        } else if (strcmp(operator, ">>") == 0) {
            // appendFlag = 1;
            redirectFlag = 1;
            vector_erase(splicedLineTwo, 0);
            
        } else if (strcmp(operator, ">") == 0) {
            // outputFlag = 1;
            redirectFlag = 2;
            vector_erase(splicedLineTwo, 0);
            
        } else if (strcmp(operator, "<") == 0) {
            // inputFlag = 1;
            redirectFlag = 3;
            vector_erase(splicedLineTwo, 0);
            
        } else {
            semiFlag = 1;
        }
    }

    // checking for background processes
    int isBackground = 0;
    char * lastWord = vector_get(splicedLine, vector_size(splicedLine) - 1);
    if (lastWord[0] == '&') {
        isBackground = 1;
        vector_erase(splicedLine, vector_size(splicedLine) - 1);
    }

    suc1a = isBuiltIn(splicedLine, h, pids);
    if (suc1a == 0) {
        suc2a = isNotBuiltIn(splicedLine, h, pids, isBackground, redirectFlag, splicedLineTwo);
    }


    int suc1 = 0;
    if (suc1a == 1 || suc2a == 1) {
        suc1 = 1;
    } else if (suc1a == -1 || suc2a == -1) {
        suc1 = -1;
    }

    int suc1b = 0;
    int suc2b = 0;
    if ((suc1 == 1) && (andFlag == 1)) {
        suc1b = isBuiltIn(splicedLineTwo, h, pids);
        if (suc1b == 0) {
            suc2b = isNotBuiltIn(splicedLineTwo, h, pids, isBackground, redirectFlag, splicedLineTwo);
        }
    } else if ((suc1 == -1) && (orFlag == 1)) {
        suc1b = isBuiltIn(splicedLineTwo, h, pids);
        if (suc1b == 0) {
            suc2b = isNotBuiltIn(splicedLineTwo, h, pids, isBackground, redirectFlag, splicedLineTwo);
        }
    } else if (semiFlag == 1) {
        suc1b = isBuiltIn(splicedLineTwo, h, pids);
        if (suc1b == 0) {
            suc2b = isNotBuiltIn(splicedLineTwo, h, pids, isBackground, redirectFlag, splicedLineTwo);
        }
    }
    
    sstring_destroy(cmds);
    vector_destroy(splicedLine);
    vector_destroy(splicedLineTwo);
}

int runUserInput(vector * h, vector * pids) {
    pid_t parentPid = getpid();
    char directory[200];
    char *line;
    int exitFlag = 0;
    // WHILE LOOP THAT PROMPTS FOR USER INPUT
    while (exitFlag == 0) {
        getcwd(directory, 200);
        print_prompt(directory, parentPid);

        // stick this somewhere lmfao
        signal(SIGINT, &handle_sigint);

        // // kill zombie children
        // waitpid(-1, NULL, WNOHANG);

        line = getInput();

        int eofFlag = feof(stdin);
        if (eofFlag != 0) {
            free(line);
            exitFlag = -1;
            break;
        }

        if (strcmp(line, "exit") == 0) {
            free(line);
            exitFlag = -1;
            break;
        }
        
        vector_push_back(h, line);
        
        runCmd(h, pids, line);
        free(line);
    }
    return -1;
}

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    char directory[200];
    getcwd(directory, 200);
    vector * h = string_vector_create();
    vector * pids = int_vector_create();
    int hFlag = 0;
    char* hFile;
    int fFlag = 0;
    char* fFile;

    // do nothing
    if (argc == 0) {
        return 0;
    }

    if ((argc == 1 || argc == 3 || argc == 5) && (strcmp(argv[0], "./shell") == 0 || strcmp(argv[0], "./shell-debug") == 0)) {
        // if it calls /shell -f or ./shell -h
        int opt;
        while ((opt = getopt(argc, argv, "h:f:")) != -1) {
            switch(opt) {
                case 'h':   
                    hFlag = 1;
                    hFile = strdup(optarg);
                    // optarg = textfile
                    break;
                case 'f':
                    fFlag = 1;
                    fFile = strdup(optarg);
                    // pid_t fPid = fork(); 
                    break;
            }
        }
        if (optind < argc)  print_script_file_error();
        // @TODO decide if we exit here?
        // exit(0);
    // } else {
    //     print_usage();
    //     exit(1);
    }

    if (fFlag) {
        FILE * fptr = fopen(fFile, "r");
        char * str = NULL;
        ssize_t nread;
        size_t len = 0;
        if (!fptr) {
            printf("file pointer error");
            exit(1);
        }
        // if (fgets(str, ))
        while ((nread = getline(&str, &len, fptr)) != -1) {
            str[strlen(str) - 1] = '\0';
            // printf("%s\n", str);
            vector_push_back(h, str);
            runCmd(h, pids, str);
        }
        free(str);
        fclose(fptr);
        fFlag = 0;
    } else {
        runUserInput(h, pids);
    }

    // use fopen and fclose to store in history.text
    if (hFlag == 1) {
        chdir(directory);
        FILE *hptr = fopen(hFile, "a");
        // if fopen doesnt work out
        if (!hptr) {
            printf("file pointer error");
            exit(1);
        }

        while (!vector_empty(h)) {
            fprintf(hptr, "%s", vector_get(h, 0));
            fprintf(hptr, "\n");
            vector_erase(h, 0);
        }
        fclose(hptr);
    }


    // kill remaining pids
    pid_t kidpid;
    int status;
    while (vector_size(pids) > 0) {
        int * pid = vector_get(pids, vector_size(pids) - 1);
        kidpid = waitpid(*pid, &status, WNOHANG);
        vector_erase(pids, vector_size(pids) - 1);
    }

    vector_destroy(h);
    vector_destroy(pids);
    exit(0);
    return 0;
}
