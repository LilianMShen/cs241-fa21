/**
 * password_cracker
 * CS 241 - Fall 2021
 */
#include "cracker2.h"
#include "libs/format.h"
#include "libs/utils.h"
#include "includes/queue.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <crypt.h>
#include <pthread.h>

static queue * users;
static size_t tc;
static int passwordFound;
static char * finPassword;
static int totalHash;
static double lastCPUTime;
static pthread_barrier_t barrier;

struct user {
    char * username;
    char * hash;
    char * password;
};

static struct user * u;

// create user struct and push it to queue
void user_init(char * str, queue * users) {
    if (str == NULL  || users == NULL) return;
    struct user * one = malloc(sizeof(struct user));
    char * token = strtok(str, " ");

    while (token != NULL) {
        if(one->username == NULL) {
            one->username = strdup(token);
        } else if (one->hash == NULL) {
            one->hash = strdup(token);
        } else if (one->password == NULL) {
            one->password = strdup(token);
        }
        token = strtok(NULL, " ");
    }
    queue_push(users, one);
}

void user_destroy(struct user * u) {
    if (u == NULL) return;
    if (u->username) free(u->username);
    if (u->hash) free(u->hash);
    if (u->password) free(u->password);
    if (u) free(u);
}

void * routine(int * threadi) {
    printf("hit -1\n");
    lastCPUTime = 0;
    int threadID = *threadi;
    free(threadi);
    struct crypt_data data;
    data.initialized = 0;

    pthread_barrier_wait(&barrier);
    if (threadID == 1) u = queue_pull(users);
    pthread_barrier_wait(&barrier);

    while(strcmp(u->username, "dummy") != 0){
        printf("hit 0\n");
        passwordFound = 1;
        totalHash = 0;
        if (threadID == 1) v2_print_start_user(u->username);
        pthread_barrier_wait(&barrier);

        printf("hit 1\n");

        double start = getTime();
        char * password  = strdup(u->password);
        char * salt = strndup(u->hash, 2);

        // set the password
        int prefixLength = getPrefixLength(password);
        for (int i = prefixLength; i < (int) strlen(password); i++) {
            password[i] = 'a';
        }
        int unknownCharacterLength = strlen(password) - prefixLength;

        long startIndex;
        long count;
        getSubrange(unknownCharacterLength, tc, threadID, &startIndex, &count);

        setStringPosition(password, startIndex);

        v2_print_thread_start(threadID, u->username, startIndex, password);

        int n = 0;
        int result = 2;
        for (int i = 0; i < count; i++) {
            if (passwordFound == 0) {
                result = 1;
                break;
            }
            n++;
            if (strcmp(crypt_r(password, salt, &data), u->hash) == 0) {
                passwordFound = 0;
                result = 0;
                finPassword = strdup(password);
                break;
            }
            if (incrementString(password + prefixLength) == 0) {
                break;
            }
        }
        n++;
        totalHash += n;

        pthread_barrier_wait(&barrier);
        
        double end = getTime();
        
        v2_print_thread_result(threadID, n, result);

        pthread_barrier_wait(&barrier);
        
        double cpuTime = getCPUTime();
        if (threadID == 1) v2_print_summary(u->username, finPassword, totalHash, end - start, cpuTime - lastCPUTime, passwordFound);
        lastCPUTime = cpuTime;

        // printf("here 1 %d\n", threadID);
        if (password) free(password);
        if (salt) free(salt);
        if (finPassword) free(finPassword);
        if (threadID == 1) user_destroy(u);
        pthread_barrier_wait(&barrier);
        if (threadID == 1) u = queue_pull(users);
        pthread_barrier_wait(&barrier);
    }
    if (u) user_destroy(u);
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    users = queue_create(100);

    char * str = NULL;
    ssize_t nread;
    size_t len = 0;
    int count = 0;
    while ((nread = getline(&str, &len, stdin)) != -1) {
        if (feof(stdin)) {
            break;
        }
        if (str && strlen(str) > 0) {
            count++;
            str[strlen(str) - 1] = '\0';
            user_init(str, users);
        }
    }
    free(str);

    // set threadpool
    tc = thread_count;
    if (thread_count == 0) {
        // default 4 threads
        tc = 4;
    }
    // printf("set thread count %ld\n", tc);

    char * dummy = strdup("dummy dummy dummy\0");
    // for (size_t i = 0; i < tc; i++) {
    //     user_init(dummy, users);
    // }
    user_init(dummy, users);
    free(dummy);

    

    pthread_t * threads = malloc(sizeof(threads) * tc);
    for (size_t i = 0; i < tc; i++) {
        int * threadId = malloc(sizeof(int));
        *threadId = i+1;
        int createErr = pthread_create(&threads[i], NULL, (void*) routine, threadId);
        
        if (createErr) {
            printf("create error hit at thread %d\n", (int) i);
        }
    };

    pthread_barrier_init(&barrier, NULL, tc);

    for (size_t j= 0; j < tc; j++) {
        int joinErr = pthread_join(threads[j], NULL);
        if (joinErr) {
            printf("join error hit at thread %d\n", (int) j);
        }
    }
    // printf("joined threads\n");

    free(threads);
    queue_destroy(users);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}