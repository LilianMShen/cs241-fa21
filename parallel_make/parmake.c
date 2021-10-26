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

static queue * rules;
static vector * goals;
static vector * visited;
static vector * fails;
static graph * dependency_graph;

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

int isVisited(void * target) {
    for (size_t i = 0; i < vector_size(visited); i++) {
        // this target is already in the cycle somewhere
        if (strcmp(vector_get(visited, i), target) == 0) {
            return 1;
        }
    }
    return 0;
}

int executeDependencies(void * target) {
    if (checkForCycle(target)) {
        return -1;
    }

    vector * dependencies = graph_neighbors(dependency_graph, target);

    for (size_t i = 0; i < vector_size(dependencies); i++) {
        void * dep = vector_get(dependencies, i);
        if (executeDependencies(dep) == -1) {
            vector_destroy(dependencies);
            return -1;
        }
    }

    if (isVisited(target) != 1) {
        vector_push_back(visited, target);
        queue_push(rules, target);
    }
    vector_destroy(dependencies);

    return 1;
}

int isFailed(void * target) {
    vector * dependencies = graph_neighbors(dependency_graph, target);
    
    for (size_t i = 0; i < vector_size(dependencies); i++) {
        void * dep = vector_get(dependencies, i);
        for (size_t j = 0; j < vector_size(fails); j++) {
            if (strcmp(dep, vector_get(fails, j)) == 0) {
                vector_destroy(dependencies);
                return 1;
            }
        }
    }
    
    vector_destroy(dependencies);
    return 0;
}

void runGoal(void * goal) {
    queue_push(rules, "dummy");
    void * t = queue_pull(rules);

    while (strcmp(t, "dummy") != 0) {
        if (isFailed(t) == 1) {
            vector_push_back(fails, t);
        } else {
            rule_t * rule = (rule_t *) graph_get_vertex_value(dependency_graph, t);
            for (size_t i = 0; i < vector_size(rule->commands); i++) {
                if (system(vector_get(rule->commands, i)) != 0) {
                    vector_push_back(fails, t);
                    break;
                }
            }
        }

        t = queue_pull(rules);
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

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    // printf("please....");
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

    size_t goalIndex = 0;
    while (goalIndex < vector_size(goals)) {
        rules = queue_create(100);
        visited = string_vector_create();
        fails = string_vector_create();

        void * goal = vector_get(goals, goalIndex);
        if (executeDependencies(goal) == -1) {
            print_cycle_failure(goal);
            graph_remove_edge(dependency_graph, "", goal);
            vector_erase(goals, goalIndex);
            goalIndex--;
        } else {
            if (needToRunGoal(goal) == 1) runGoal(goal);
        }

        queue_destroy(rules);
        vector_destroy(visited);
        vector_destroy(fails);

        goalIndex++;
    }

    vector_destroy(targs);
    vector_destroy(goals);
    // vector_destroy(visited);
    graph_destroy(dependency_graph);
    
    // queue_destroy(rules);


    return 0;
}
