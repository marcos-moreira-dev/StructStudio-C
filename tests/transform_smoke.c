/*
 * StructStudio C
 * --------------
 * Smoke tests for structural transformations.
 *
 * These checks cover two pedagogical features added in this iteration:
 * explicit BST/AVL rotations and graph-to-tree derivations from traversals.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "core/api.h"

static int require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "%s\n", message);
        return 0;
    }
    return 1;
}

static int test_bst_rotation(void)
{
    SsStructure structure;
    SsError error;
    char message[SS_MESSAGE_CAPACITY];
    const SsNode *root;

    ss_structure_init(&structure, SS_VARIANT_BST, "rotation_bst");
    ss_error_clear(&error);

    if (!ss_structure_apply_primary(&structure, "30", "", 30, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&structure, "20", "", 20, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&structure, "40", "", 40, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&structure, "35", "", 35, "", &error, message, sizeof(message))) return 0;

    root = ss_structure_find_node_const(&structure, structure.root_id);
    if (!require(root != NULL && strcmp(root->value, "30") == 0, "BST base invalido antes de rotar.")) {
        ss_structure_free(&structure);
        return 0;
    }

    if (!ss_structure_rotate_left(&structure, structure.root_id, &error)) {
        fprintf(stderr, "Rotation failed: %s\n", error.message);
        ss_structure_free(&structure);
        return 0;
    }

    root = ss_structure_find_node_const(&structure, structure.root_id);
    if (!require(root != NULL && strcmp(root->value, "40") == 0, "La rotacion izquierda no actualizo la raiz.")) {
        ss_structure_free(&structure);
        return 0;
    }
    if (!require(strcmp(root->data.ref_a, "") != 0, "La nueva raiz no conserva el subarbol izquierdo.")) {
        ss_structure_free(&structure);
        return 0;
    }

    ss_structure_free(&structure);
    return 1;
}

static int test_graph_bfs_tree(void)
{
    SsStructure graph;
    SsStructure derived;
    SsError error;
    char message[SS_MESSAGE_CAPACITY];
    const SsNode *a;
    const SsNode *b;
    const SsNode *c;
    const SsNode *d;

    memset(&derived, 0, sizeof(derived));
    ss_structure_init(&graph, SS_VARIANT_DIRECTED_GRAPH, "graph_bfs");
    ss_error_clear(&error);

    if (!ss_structure_apply_primary(&graph, "A", "", 0, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&graph, "B", "", 0, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&graph, "C", "", 0, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&graph, "D", "", 0, "", &error, message, sizeof(message))) return 0;

    a = ss_structure_find_node_by_token_const(&graph, "A", &error);
    b = ss_structure_find_node_by_token_const(&graph, "B", &error);
    c = ss_structure_find_node_by_token_const(&graph, "C", &error);
    d = ss_structure_find_node_by_token_const(&graph, "D", &error);
    if (!require(a != NULL && b != NULL && c != NULL && d != NULL, "No se pudieron resolver los vertices del grafo.")) {
        ss_structure_free(&graph);
        return 0;
    }

    if (!ss_structure_connect(&graph, a->id, b->id, "graph_link", 0.0, &error)) return 0;
    if (!ss_structure_connect(&graph, a->id, c->id, "graph_link", 0.0, &error)) return 0;
    if (!ss_structure_connect(&graph, b->id, d->id, "graph_link", 0.0, &error)) return 0;

    if (!ss_structure_build_graph_tree(&graph, SS_ANALYSIS_GRAPH_BFS, a->id, &derived, message, sizeof(message), &error)) {
        fprintf(stderr, "Graph tree failed: %s\n", error.message);
        ss_structure_free(&graph);
        return 0;
    }

    if (!require(derived.node_count == 4, "El arbol BFS debe conservar los vertices alcanzados.")) {
        ss_structure_free(&graph);
        ss_structure_free(&derived);
        return 0;
    }
    if (!require(derived.edge_count == 3, "El arbol BFS debe contener n-1 aristas.")) {
        ss_structure_free(&graph);
        ss_structure_free(&derived);
        return 0;
    }
    if (!require(strcmp(derived.visual_state.layout_mode, "tree_bfs") == 0, "El layout derivado no quedo marcado como arbol BFS.")) {
        ss_structure_free(&graph);
        ss_structure_free(&derived);
        return 0;
    }

    ss_structure_free(&graph);
    ss_structure_free(&derived);
    return 1;
}

static int test_graph_layout_rotation(void)
{
    SsStructure graph;
    SsError error;
    char message[SS_MESSAGE_CAPACITY];
    double before_x[3];
    double before_y[3];

    ss_structure_init(&graph, SS_VARIANT_UNDIRECTED_GRAPH, "graph_rotation");
    ss_error_clear(&error);

    if (!ss_structure_apply_primary(&graph, "A", "", 0, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&graph, "B", "", 0, "", &error, message, sizeof(message))) return 0;
    if (!ss_structure_apply_primary(&graph, "C", "", 0, "", &error, message, sizeof(message))) return 0;
    ss_structure_auto_layout(&graph, 720.0);

    for (size_t index = 0; index < graph.node_count; ++index) {
        before_x[index] = graph.nodes[index].visual.x;
        before_y[index] = graph.nodes[index].visual.y;
    }

    if (!ss_structure_rotate_graph_layout(&graph, 0.2617993877991494, "", &error)) {
        fprintf(stderr, "Graph rotation failed: %s\n", error.message);
        ss_structure_free(&graph);
        return 0;
    }

    if (!require(
            fabs(graph.nodes[0].visual.x - before_x[0]) > 0.5 ||
            fabs(graph.nodes[0].visual.y - before_y[0]) > 0.5,
            "La rotacion del grafo no modifico la geometria visible.")) {
        ss_structure_free(&graph);
        return 0;
    }

    if (!ss_structure_rotate_graph_layout(&graph, -0.2617993877991494, "", &error)) {
        fprintf(stderr, "Graph reverse rotation failed: %s\n", error.message);
        ss_structure_free(&graph);
        return 0;
    }

    for (size_t index = 0; index < graph.node_count; ++index) {
        if (!require(
                fabs(graph.nodes[index].visual.x - before_x[index]) < 0.6 &&
                fabs(graph.nodes[index].visual.y - before_y[index]) < 0.6,
                "La rotacion inversa del grafo no recupero la distribucion original.")) {
            ss_structure_free(&graph);
            return 0;
        }
    }

    ss_structure_free(&graph);
    return 1;
}

int main(void)
{
    if (!test_bst_rotation()) {
        return 1;
    }
    if (!test_graph_bfs_tree()) {
        return 1;
    }
    if (!test_graph_layout_rotation()) {
        return 1;
    }
    return 0;
}
