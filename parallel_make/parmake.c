/**
 * parallel_make
 * CS 241 - Fall 2021
 */

#include "format.h"
#include "includes/graph.h"
#include "parmake.h"
#include "parser.h"


int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    vector *targets = graph_vertices(dependency_graph);
    rule_t * rule = (rule_t *) graph_get_vertex_value(dependency_graph, target);
    vector *dependencies = graph_neighbors(dependency_graph, target);
    return 0;
}
