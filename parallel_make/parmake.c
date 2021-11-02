/**
 * parallel_make
 * CS 241 - Fall 2021
 */

#include "format.h"
#include "includes/graph.h"
#include "parmake.h"
#include "parser.h"
#include "includes/queue.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

static vector * goals;
static graph * dependency_graph;
static size_t * runIndex;
// static bool * flag;

struct info {
    int plausible;
    bool isFinished;
    void * goal;
    queue * rules;
    vector * visited;
    vector * fails;
};

bool contains_cycle(void* cur_node, void* to_match) {
    if (cur_node == NULL && to_match == NULL) return true;
    if (cur_node == NULL || to_match == NULL) return false;
    if (cur_node == to_match) return true;

    // For each neighbor check if there is a path to to_match
    vector *neighbors = graph_neighbors(dependency_graph, cur_node);
    size_t i;
    for (i = 0; i < vector_size(neighbors); i++) {
        if (contains_cycle(vector_get(neighbors, i), to_match)) {
            vector_destroy(neighbors);
            return true;
        }
    }
    vector_destroy(neighbors);
    return false;
}

int checkForCycle(void * target) {
    int has_cycle = 0;
    vector* neighbors = graph_neighbors(dependency_graph, target);
    size_t i;
    for (i = 0; i < vector_size(neighbors); i++) {
        // printf("%zu %zu\n", vector_size(neighbors), i);
        if (contains_cycle(vector_get(neighbors, i), target)) {
            has_cycle = 1;
            break;
        }
    }
    vector_destroy(neighbors);
    return has_cycle;
}

int isVisited(void * target, struct info threadInfo) {
    for (size_t i = 0; i < vector_size(threadInfo.visited); i++) {
        // this target is already in the cycle somewhere
        if (strcmp(vector_get(threadInfo.visited, i), target) == 0) {
            return 1;
        }
    }
    return 0;
}

int executeDependencies(char * target, struct info threadInfo) {
    if (checkForCycle(target)) {
        return -1;
    }

    vector * dependencies = graph_neighbors(dependency_graph, target);

    for (size_t i = 0; i < vector_size(dependencies); i++) {
        void * dep = vector_get(dependencies, i);
        if (executeDependencies(dep, threadInfo) == -1) {
            vector_destroy(dependencies);
            return -1;
        }
    }

    if (isVisited(target, threadInfo) != 1) {
        char * t = malloc(sizeof(target));
        strcpy(t, target); 
        vector_push_back(threadInfo.visited, t);
        queue_push(threadInfo.rules, t);
    }
    vector_destroy(dependencies);

    return 1;
}

int isFailed(void * target, struct info info) {
    vector * dependencies = graph_neighbors(dependency_graph, target);
    
    for (size_t i = 0; i < vector_size(dependencies); i++) {
        void * dep = vector_get(dependencies, i);
        for (size_t j = 0; j < vector_size(info.fails); j++) {
            if (strcmp(dep, vector_get(info.fails, j)) == 0) {
                vector_destroy(dependencies);
                return 1;
            }
        }
    }
    
    vector_destroy(dependencies);
    return 0;
}

void runGoal(void * goal, struct info info) {
    // queue_push(info.rules, "dummy");
    char * t = queue_pull(info.rules);
    // printf("%s\n", t);
    if (strcmp(t, "dummy") == 0) {
        // printf("returning\n");
        return;
    }
    while (strcmp(t, "dummy") != 0) {
        if (isFailed(t, info) == 1) {
            vector_push_back(info.fails, t);
        } else {
            rule_t * rule = (rule_t *) graph_get_vertex_value(dependency_graph, t);
            for (size_t i = 0; i < vector_size(rule->commands); i++) {
                if (system(vector_get(rule->commands, i)) != 0) {
                    vector_push_back(info.fails, t);
                    break;
                }
            }
        }

        free(t);
        t = queue_pull(info.rules);
    }
}

int needToRunGoal(void * target) {
    // struct stat targetStat;
    // stat(target, &targetStat);
    // // targetStat.st_mtime;

    // vector * dependencies = graph_neighbors(dependency_graph, target);

    // if (vector_size(dependencies) == 0) {
    //     struct stat otherStat;
    //     if (stat(t, &otherStat) == -1) {
    //         return 1;
    //     } else if (difftime(goalStat.st_mtime, otherStat.st_mtime) >= 1) {
    //         return 1;
    //     }
    // }
    
    // for (size_t i = 0; i < vector_size(dependencies); i++) {
    //     void * t = vector_get(dependencies, i);

    //     needToRunGoal(t);
    // }

    return 1;
    // return -1;
}

void * routine(struct info * threadInfo) {
    // printf("# of goals is %ld\n", vector_size(goals));
    while (*runIndex < vector_size(goals) ) {
        // printf("current run index is %ld\n", *runIndex);
        if(threadInfo[*runIndex].plausible == -1) {
            print_cycle_failure(threadInfo[*runIndex].goal);
        } else {
            runGoal(threadInfo[*runIndex].goal, threadInfo[*runIndex]);
            // void * t = queue_pull(threadInfo[*runIndex].rules);

            // while (strcmp(t, "dummy") != 0) {
            //     if (isFailed(t, threadInfo[*runIndex]) == 1) {
            //         vector_push_back(threadInfo[*runIndex].fails, t);
            //     } else {
            //         rule_t * rule = (rule_t *) graph_get_vertex_value(dependency_graph, t);
            //         for (size_t i = 0; i < vector_size(rule->commands); i++) {
            //             if (system(vector_get(rule->commands, i)) != 0) {
            //                 vector_push_back(threadInfo[*runIndex].fails, t);
            //                 break;
            //             }
            //         }
            //     }

            //     t = queue_pull(threadInfo[*runIndex].rules);
            // }
        }
        
        if (threadInfo[*runIndex].isFinished == false) {
            threadInfo[*runIndex].isFinished = true;
            // printf("inside here hehehehe \n");
            (*runIndex) += 1;
        }
    }

    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    dependency_graph = parser_parse_makefile(makefile, targets);
    vector * targs = graph_vertices(dependency_graph);
    void * target;
    vector * dependencies;
    goals = string_vector_create();
    size_t index = 0;
    while (index < vector_size(targs)) {
        target = vector_get(targs, index);
        if (strcmp(target, "") == 0) {
            dependencies = graph_neighbors(dependency_graph, target);
            for (size_t i = 0; i < vector_size(dependencies); i++) {
                vector_push_back(goals, vector_get(dependencies, i));
                // printf("%s\n", (char*) vector_get(dependencies, i));
            }
            vector_destroy(dependencies);
            break;
        }
        index++;
    }

    struct info * threadInfo = malloc(sizeof(struct info) * vector_size(goals));

    size_t goalIndex = 0;
    while (goalIndex < vector_size(goals)) {
        threadInfo[goalIndex].rules = queue_create(100);
        threadInfo[goalIndex].visited = string_vector_create();
        threadInfo[goalIndex].fails = string_vector_create();
        
        void * goal = vector_get(goals, goalIndex);
        
        threadInfo[goalIndex].goal = goal;
        threadInfo[goalIndex].plausible = executeDependencies(goal, threadInfo[goalIndex]);
        threadInfo[goalIndex].isFinished = 0;

        // printf("num threads is %ld\n", num_threads);
        for (size_t i = 0; i < num_threads; i++) {
            // printf("hit\n");
            queue_push(threadInfo[goalIndex].rules, "dummy");
        }
        
        // if (executeDependencies(goal) == -1) {
        //     print_cycle_failure(goal);
        //     graph_remove_edge(dependency_graph, "", goal);
        //     vector_erase(goals, goalIndex);
        //     goalIndex--;
        // } 
        // else {
        //     if (needToRunGoal(goal) == 1) runGoal(goal);
        // }

        goalIndex++;
    }

    runIndex = malloc(sizeof(size_t));
    *runIndex = 0;

    pthread_t * threads = malloc(sizeof(pthread_t) * num_threads); // keep track of thread ids

    // routine(threadInfo);

    for (size_t i = 0; i < num_threads; i++) {
        int createErr = pthread_create(&threads[i], NULL, (void *) routine, threadInfo);
        if (createErr) {
            printf("error hit at thread %d\n", (int) i);
        }
    }

    for (size_t j= 0; j < num_threads; j++) {
        int joinErr = pthread_join(threads[j], NULL);
        if (joinErr) {
            printf("join error hit at thread %d\n", (int) j);
        }
    }

    free(threads);
    free(runIndex);
    for (size_t i = 0; i < vector_size(goals); i++) {
        vector_destroy(threadInfo[i].fails);
        queue_destroy(threadInfo[i].rules);
        // if (vector_size(threadInfo[i].visited) > 0) vector_destroy(threadInfo[i].visited);
        // if(threadInfo[i].goal != NULL) free(threadInfo[i].goal);
    }
    free(threadInfo);
    vector_destroy(targs);
    vector_destroy(goals);
    // graph_destroy(dependency_graph);
    return 0;
}
