// justink6, jlwang5, dbargon2, lmshen2

/**
 * deadlock_demolition
 * CS 241 - Fall 2021
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
    pthread_mutex_t pm;
};

// Resource allocation graph
static graph* g;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


drm_t *drm_init() {
    // Create graph if it doesn't exist
    if (g == NULL) {
        g = shallow_graph_create();
    }

    // Initialize drm_t
    drm_t* result = malloc(sizeof(drm_t));
    pthread_mutex_init(&result->pm, NULL);

    // Lock thread
    pthread_mutex_lock(&lock);

    // Add to graph
    graph_add_vertex(g, result);

    // Unlock thread
    pthread_mutex_unlock(&lock);

    return result;
}

bool contains_cycle(void* cur_node, void* to_match) {
    if (cur_node == to_match) {
        return true;
    }
    // For each neighbor check if there is a path to to_match
    vector *neighbors = graph_neighbors(g, cur_node);
    size_t i;
    for (i = 0; i < vector_size(neighbors); i++) {
        if (contains_cycle(vector_get(neighbors, i), to_match)) {
            return true;
        }
    }
    return false;
}

int checkForCycle(pthread_t *thread_id) {
    int has_cycle = 0;
    vector* neighbors = graph_neighbors(g, thread_id);
    size_t i;
    for (i = 0; i < vector_size(neighbors); i++) {
        // printf("%zu %zu\n", vector_size(neighbors), i);
        if (contains_cycle(vector_get(neighbors, i), thread_id)) {
            has_cycle = 1;
            break;
        }
    }

    return has_cycle;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&lock);

    // Check to see if graph contains edges
    if (!graph_contains_vertex(g, drm) || !graph_contains_vertex(g, thread_id)) {
        pthread_mutex_unlock(&lock);
        return 0;
    }

    // Check for edge
    if (!graph_adjacent(g, drm, thread_id)) {
        pthread_mutex_unlock(&lock);
        return 0;
    }

    // Remove edge
    graph_remove_edge(g, drm, thread_id);
    
    // Unlock mutex
    pthread_mutex_unlock(&drm->pm);
    pthread_mutex_unlock(&lock);

    return 1;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&lock);

    // Add thread to resource allocation graph
    graph_add_vertex(g, thread_id);

    // Check to see if edge already exists
    if (!graph_adjacent(g, drm, thread_id)) {
        // Add edge from thread TO drm - process requesting resource
        graph_add_edge(g, thread_id, drm);

        // Check for cycle
        if (checkForCycle(thread_id)) {
            // Cycle found, remove request edge
            graph_remove_edge(g, thread_id, drm);
            pthread_mutex_unlock(&lock);
            return 0;
        } else {
            // Reverse edge to point from drm to thread
            pthread_mutex_unlock(&lock);
            pthread_mutex_lock(&drm->pm);

            pthread_mutex_lock(&lock);

            graph_remove_edge(g, thread_id, drm);
            graph_add_edge(g, drm, thread_id);

            pthread_mutex_unlock(&lock);
            return 1;
        }
    } else {
        pthread_mutex_unlock(&lock);
        return 0;
    }
}

void drm_destroy(drm_t *drm) {
    // Remove vertex from graph
    graph_remove_vertex(g, drm);

    // Destroy mutex
    pthread_mutex_destroy(&drm->pm);
    pthread_mutex_destroy(&lock);

    free(drm);
    return;
}