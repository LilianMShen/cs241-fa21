/**
 * password_cracker
 * CS 241 - Fall 2021
 */
#include "cracker1.h"
#include "libs/format.h"
#include "libs/utils.h"
#include "includes/queue.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <crypt.h>

static int success;
static int fail;
static queue * users;

struct user {
    char * username;
    char * hash;
    char * password;
};

// create user struct and push it to queue
void user_init(char * str, queue * users) {
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
        // printf("%s\n", token);
        token = strtok(NULL, " ");
    }
    queue_push(users, one);
}

void user_destroy(struct user * u) {
    free(u->username);
    free(u->hash);
    free(u->password);
    free(u);
}

void * routine(int * threadi) {
    // printf("hit\n");
    int i = *threadi;
    // printf("%d", i);
    // printf("test");
    free(threadi);
    struct crypt_data data;
    data.initialized = 0;
    
    struct user * u = queue_pull(users);
    while(strcmp(u->username, "dummy") != 0){
        double start = getTime();
        v1_print_thread_start(i, u->username);
        char * password  = strdup(u->password);
        char * salt = strndup(u->hash, 2);
        // set the password
        int prefixLength = getPrefixLength(password);
        for (int i = prefixLength; i < (int) strlen(password); i++) {
            password[i] = 'a';
        }

        int n = 0;
        int result = 1;
        while(strcmp(crypt_r(password, salt, &data), u->hash) != 0 && incrementString(password + prefixLength) != 0) {
            n++;
            // if (incrementString(password) == 0) {
            //     fail++;
            //     printf("hit fail");
            //     break;
            // }
            // printf("%s", password);
        }
        n++;
        
        if (strcmp(crypt_r(password, salt, &data), u->hash) == 0) {
            result = 0;
            success++;
        } else {
            fail++;
        }

        double end = end = getTime();
        v1_print_thread_result(i, u->username, password, n, (end - start), result);

        free(password);
        free(salt);
        user_destroy(u);
        u = queue_pull(users);
    }
    user_destroy(u);
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    users = queue_create(100);

    char * str = NULL;
    ssize_t nread;
    size_t len = 0;
    while ((nread = getline(&str, &len, stdin)) != -1) {
        if (feof(stdin)) {
            // printf("\nhit feof\n");
            break;
        }
        str[strlen(str) - 1] = '\0';
        user_init(str, users);
    }
    free(str);
    // printf("exited the input space\n");

    // set threadpool
    size_t tc = thread_count;
    if (thread_count == 0) {
        // default 4 threads
        tc = 4;
    }
    // printf("set thread count %ld\n", tc);

    char * dummy = strdup("dummy dummy dummy\0");
    for (size_t i = 0; i < tc; i++) {
        user_init(dummy, users);
    }
    free(dummy);
    // printf("set dummy threads\n");

    // pthread_mutex_t mutex;
    // pthread_mutex_init(&mutex, NULL);

    printf("about to create threads\n");
    pthread_t * threads = malloc(sizeof(threads) * tc);
    for (size_t i = 1; i <= tc; i++) {
        int * threadId = malloc(sizeof(int));
        *threadId = i;
        int createErr = pthread_create(&threads[i], NULL, (void*) routine, threadId);
        if (createErr) {
            printf("create error hit at thread %d\n", (int) i);
        }
    }
    // printf("created threads\n");

    void* retval;
    for (size_t j= 0; j < tc; j++) {
        int joinErr = pthread_join(threads[j], &retval);
        if (joinErr) {
            printf("join error hit at thread %d\n", (int) j);
        }
    }
    // printf("joined threads\n");

    free(threads);
    // pthread_mutex_destroy(&mutex);
    queue_destroy(users);
    v1_print_summary(success,fail);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
