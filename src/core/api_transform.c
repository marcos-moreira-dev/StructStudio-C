/*
 * StructStudio C
 * --------------
 * Structural transformations derived from existing models.
 *
 * This module keeps pedagogical transformations outside the editor and UI
 * layers: local tree rotations stay in the core, and graph-derived trees are
 * materialized here as new structures without coupling algorithms to widgets.
 */

#include "api_internal.h"

#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SsWeightedEdgeRef {
    size_t source_index;
    size_t target_index;
    double weight;
} SsWeightedEdgeRef;

static int ss_weighted_edge_compare(const void *left, const void *right)
{
    const SsWeightedEdgeRef *a = (const SsWeightedEdgeRef *) left;
    const SsWeightedEdgeRef *b = (const SsWeightedEdgeRef *) right;

    if (a->weight < b->weight) {
        return -1;
    }
    if (a->weight > b->weight) {
        return 1;
    }
    if (a->source_index < b->source_index) {
        return -1;
    }
    if (a->source_index > b->source_index) {
        return 1;
    }
    if (a->target_index < b->target_index) {
        return -1;
    }
    if (a->target_index > b->target_index) {
        return 1;
    }
    return 0;
}

static int ss_dsu_find(int *parent, int index)
{
    if (parent[index] != index) {
        parent[index] = ss_dsu_find(parent, parent[index]);
    }
    return parent[index];
}

static int ss_dsu_union(int *parent, int *rank, int left, int right)
{
    int root_left = ss_dsu_find(parent, left);
    int root_right = ss_dsu_find(parent, right);

    if (root_left == root_right) {
        return 0;
    }

    if (rank[root_left] < rank[root_right]) {
        parent[root_left] = root_right;
    } else if (rank[root_left] > rank[root_right]) {
        parent[root_right] = root_left;
    } else {
        parent[root_right] = root_left;
        rank[root_left] += 1;
    }

    return 1;
}

static double ss_graph_edge_weight(const SsEdge *edge)
{
    return edge != NULL && edge->has_weight ? edge->weight : 1.0;
}

static int ss_structure_can_rotate(const SsStructure *structure)
{
    return structure != NULL &&
        (structure->variant == SS_VARIANT_BST || structure->variant == SS_VARIANT_AVL);
}

static int ss_structure_can_rotate_graph_layout(const SsStructure *structure)
{
    return structure != NULL && structure->family == SS_FAMILY_GRAPH && structure->node_count > 0;
}

static void ss_graph_layout_pivot(const SsStructure *structure, const char *pivot_node_id, double *pivot_x, double *pivot_y)
{
    const SsNode *pivot;
    double sum_center_x = 0.0;
    double sum_center_y = 0.0;

    if (pivot_x != NULL) {
        *pivot_x = 0.0;
    }
    if (pivot_y != NULL) {
        *pivot_y = 0.0;
    }
    if (structure == NULL || structure->node_count == 0) {
        return;
    }

    pivot = pivot_node_id != NULL && pivot_node_id[0] != '\0'
        ? ss_structure_find_node_const(structure, pivot_node_id)
        : NULL;
    if (pivot != NULL) {
        if (pivot_x != NULL) {
            *pivot_x = pivot->visual.x + pivot->visual.width / 2.0;
        }
        if (pivot_y != NULL) {
            *pivot_y = pivot->visual.y + pivot->visual.height / 2.0;
        }
        return;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        const SsNode *node = &structure->nodes[index];
        double center_x = node->visual.x + node->visual.width / 2.0;
        double center_y = node->visual.y + node->visual.height / 2.0;

        sum_center_x += center_x;
        sum_center_y += center_y;
    }

    if (pivot_x != NULL) {
        *pivot_x = sum_center_x / (double) structure->node_count;
    }
    if (pivot_y != NULL) {
        *pivot_y = sum_center_y / (double) structure->node_count;
    }
}

static void ss_graph_layout_shift_to_positive_space(SsStructure *structure)
{
    double min_x = DBL_MAX;
    double min_y = DBL_MAX;
    double shift_x = 0.0;
    double shift_y = 0.0;

    if (structure == NULL || structure->node_count == 0) {
        return;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        if (structure->nodes[index].visual.x < min_x) {
            min_x = structure->nodes[index].visual.x;
        }
        if (structure->nodes[index].visual.y < min_y) {
            min_y = structure->nodes[index].visual.y;
        }
    }

    if (min_x < 60.0) {
        shift_x = 60.0 - min_x;
    }
    if (min_y < 72.0) {
        shift_y = 72.0 - min_y;
    }
    if (shift_x == 0.0 && shift_y == 0.0) {
        return;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        structure->nodes[index].visual.x += shift_x;
        structure->nodes[index].visual.y += shift_y;
    }
}

static void ss_tree_replace_parent_link(SsStructure *structure, SsNode *parent, const char *old_child_id, const char *new_child_id)
{
    if (structure == NULL) {
        return;
    }

    if (parent == NULL) {
        ss_str_copy(structure->root_id, sizeof(structure->root_id), new_child_id != NULL ? new_child_id : "");
        return;
    }

    if (strcmp(parent->data.ref_a, old_child_id) == 0) {
        ss_str_copy(parent->data.ref_a, sizeof(parent->data.ref_a), new_child_id != NULL ? new_child_id : "");
    } else if (strcmp(parent->data.ref_b, old_child_id) == 0) {
        ss_str_copy(parent->data.ref_b, sizeof(parent->data.ref_b), new_child_id != NULL ? new_child_id : "");
    }
}

static int ss_graph_tree_copy_nodes(const SsStructure *source, SsStructure *out_structure, SsError *error)
{
    for (size_t index = 0; index < source->node_count; ++index) {
        const SsNode *source_node = &source->nodes[index];
        SsNode *node = ss_append_node_with_id(
            out_structure,
            source_node->id,
            source_node->kind,
            source_node->label,
            source_node->value,
            error);

        if (node == NULL) {
            return 0;
        }

        ss_str_copy(node->value_type, sizeof(node->value_type), source_node->value_type);
        ss_str_copy(node->data.key, sizeof(node->data.key), source_node->data.key);
        ss_str_copy(node->data.aux_text, sizeof(node->data.aux_text), source_node->data.aux_text);
        node->data.priority = source_node->data.priority;
        node->data.index_hint = source_node->data.index_hint;
    }

    return 1;
}

static int ss_graph_tree_append_edge(
    const SsStructure *source,
    SsStructure *out_structure,
    const char *parent_id,
    const char *child_id,
    double weight,
    SsError *error)
{
    SsNode *child;

    if (ss_append_edge(
            out_structure,
            parent_id,
            child_id,
            "tree_edge",
            source->config.is_directed,
            source->config.is_weighted,
            weight,
            error) == NULL) {
        return 0;
    }

    child = ss_structure_find_node(out_structure, child_id);
    if (child != NULL) {
        ss_str_copy(child->data.ref_c, sizeof(child->data.ref_c), parent_id);
    }
    return 1;
}

static void ss_graph_tree_prune_unreachable(SsStructure *structure)
{
    if (structure == NULL) {
        return;
    }

    for (size_t index = structure->node_count; index-- > 0;) {
        SsNode *node = &structure->nodes[index];

        if (strcmp(node->id, structure->root_id) == 0) {
            continue;
        }
        if (node->data.ref_c[0] == '\0') {
            ss_remove_node_at(structure, index);
        }
    }
}

static int ss_graph_tree_validate_source(const SsStructure *source, SsAnalysisKind kind, SsError *error)
{
    if (source == NULL || source->family != SS_FAMILY_GRAPH) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La transformacion requiere un grafo activo.");
        return 0;
    }

    if (!ss_analysis_supports_tree_generation(kind)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no produce un arbol derivado.");
        return 0;
    }

    if ((kind == SS_ANALYSIS_GRAPH_PRIM || kind == SS_ANALYSIS_GRAPH_KRUSKAL) &&
        (!source->config.is_weighted || source->config.is_directed)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Prim y Kruskal requieren un grafo no dirigido y ponderado.");
        return 0;
    }

    return 1;
}

static int ss_graph_tree_build_bfs(
    const SsStructure *source,
    const char *start_node_id,
    SsStructure *out_structure,
    SsError *error)
{
    int *visited;
    size_t *queue;
    size_t head = 0;
    size_t tail = 0;
    int start_index;

    start_index = ss_find_node_index(source, start_node_id);
    if (start_index < 0) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "El vertice origen para BFS no existe.");
        return 0;
    }

    visited = (int *) calloc(source->node_count, sizeof(*visited));
    queue = (size_t *) calloc(source->node_count, sizeof(*queue));
    if (visited == NULL || queue == NULL) {
        free(visited);
        free(queue);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para generar el arbol BFS.");
        return 0;
    }

    ss_str_copy(out_structure->root_id, sizeof(out_structure->root_id), start_node_id);
    visited[(size_t) start_index] = 1;
    queue[tail++] = (size_t) start_index;
    while (head < tail) {
        size_t current_index = queue[head++];
        const SsNode *current_node = &source->nodes[current_index];

        for (size_t edge_index = 0; edge_index < source->edge_count; ++edge_index) {
            const SsEdge *edge = &source->edges[edge_index];
            int neighbor_index = -1;

            if (strcmp(edge->source_id, current_node->id) == 0) {
                neighbor_index = ss_find_node_index(source, edge->target_id);
            } else if (!edge->is_directed && strcmp(edge->target_id, current_node->id) == 0) {
                neighbor_index = ss_find_node_index(source, edge->source_id);
            }

            if (neighbor_index >= 0 && !visited[(size_t) neighbor_index]) {
                visited[(size_t) neighbor_index] = 1;
                queue[tail++] = (size_t) neighbor_index;
                if (!ss_graph_tree_append_edge(
                        source,
                        out_structure,
                        current_node->id,
                        source->nodes[(size_t) neighbor_index].id,
                        ss_graph_edge_weight(edge),
                        error)) {
                    free(visited);
                    free(queue);
                    return 0;
                }
            }
        }
    }

    free(visited);
    free(queue);
    return 1;
}

static int ss_graph_tree_build_dfs_recursive(
    const SsStructure *source,
    size_t current_index,
    int *visited,
    SsStructure *out_structure,
    SsError *error)
{
    const SsNode *current_node = &source->nodes[current_index];

    visited[current_index] = 1;
    for (size_t edge_index = 0; edge_index < source->edge_count; ++edge_index) {
        const SsEdge *edge = &source->edges[edge_index];
        int neighbor_index = -1;

        if (strcmp(edge->source_id, current_node->id) == 0) {
            neighbor_index = ss_find_node_index(source, edge->target_id);
        } else if (!edge->is_directed && strcmp(edge->target_id, current_node->id) == 0) {
            neighbor_index = ss_find_node_index(source, edge->source_id);
        }

        if (neighbor_index >= 0 && !visited[(size_t) neighbor_index]) {
            if (!ss_graph_tree_append_edge(
                    source,
                    out_structure,
                    current_node->id,
                    source->nodes[(size_t) neighbor_index].id,
                    ss_graph_edge_weight(edge),
                    error)) {
                return 0;
            }
            if (!ss_graph_tree_build_dfs_recursive(source, (size_t) neighbor_index, visited, out_structure, error)) {
                return 0;
            }
        }
    }

    return 1;
}

static int ss_graph_tree_build_dfs(
    const SsStructure *source,
    const char *start_node_id,
    SsStructure *out_structure,
    SsError *error)
{
    int *visited;
    int start_index;
    int result;

    start_index = ss_find_node_index(source, start_node_id);
    if (start_index < 0) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "El vertice origen para DFS no existe.");
        return 0;
    }

    visited = (int *) calloc(source->node_count, sizeof(*visited));
    if (visited == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para generar el arbol DFS.");
        return 0;
    }

    ss_str_copy(out_structure->root_id, sizeof(out_structure->root_id), start_node_id);
    result = ss_graph_tree_build_dfs_recursive(source, (size_t) start_index, visited, out_structure, error);
    free(visited);
    return result;
}

static int ss_graph_tree_build_prim(
    const SsStructure *source,
    const char *start_node_id,
    SsStructure *out_structure,
    SsError *error)
{
    const double inf = DBL_MAX / 8.0;
    int *in_tree;
    int *parent;
    double *best_weight;
    int start_index = 0;

    if (source->node_count == 0) {
        return 1;
    }

    if (start_node_id != NULL && start_node_id[0] != '\0') {
        start_index = ss_find_node_index(source, start_node_id);
        if (start_index < 0) {
            ss_error_set(error, SS_ERROR_NOT_FOUND, "El vertice de inicio para Prim no existe.");
            return 0;
        }
    }

    in_tree = (int *) calloc(source->node_count, sizeof(*in_tree));
    parent = (int *) malloc(source->node_count * sizeof(*parent));
    best_weight = (double *) malloc(source->node_count * sizeof(*best_weight));
    if (in_tree == NULL || parent == NULL || best_weight == NULL) {
        free(in_tree);
        free(parent);
        free(best_weight);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para generar el arbol de Prim.");
        return 0;
    }

    for (size_t index = 0; index < source->node_count; ++index) {
        parent[index] = -1;
        best_weight[index] = inf;
    }

    best_weight[(size_t) start_index] = 0.0;
    for (size_t iteration = 0; iteration < source->node_count; ++iteration) {
        int current = -1;
        double current_weight = inf;

        for (size_t index = 0; index < source->node_count; ++index) {
            if (!in_tree[index] && best_weight[index] < current_weight) {
                current = (int) index;
                current_weight = best_weight[index];
            }
        }
        if (current < 0) {
            free(in_tree);
            free(parent);
            free(best_weight);
            ss_error_set(error, SS_ERROR_VALIDATION, "Prim requiere un grafo conexo.");
            return 0;
        }

        in_tree[(size_t) current] = 1;
        for (size_t edge_index = 0; edge_index < source->edge_count; ++edge_index) {
            const SsEdge *edge = &source->edges[edge_index];
            int neighbor = -1;
            double weight = ss_graph_edge_weight(edge);

            if (strcmp(edge->source_id, source->nodes[(size_t) current].id) == 0) {
                neighbor = ss_find_node_index(source, edge->target_id);
            } else if (!edge->is_directed && strcmp(edge->target_id, source->nodes[(size_t) current].id) == 0) {
                neighbor = ss_find_node_index(source, edge->source_id);
            }

            if (neighbor >= 0 && !in_tree[(size_t) neighbor] && weight < best_weight[(size_t) neighbor]) {
                best_weight[(size_t) neighbor] = weight;
                parent[(size_t) neighbor] = current;
            }
        }
    }

    ss_str_copy(out_structure->root_id, sizeof(out_structure->root_id), source->nodes[(size_t) start_index].id);
    for (size_t index = 0; index < source->node_count; ++index) {
        if ((int) index == start_index) {
            continue;
        }
        if (parent[index] < 0) {
            free(in_tree);
            free(parent);
            free(best_weight);
            ss_error_set(error, SS_ERROR_VALIDATION, "Prim no pudo construir un arbol de expansion valido.");
            return 0;
        }
        if (!ss_graph_tree_append_edge(
                source,
                out_structure,
                source->nodes[(size_t) parent[index]].id,
                source->nodes[index].id,
                best_weight[index],
                error)) {
            free(in_tree);
            free(parent);
            free(best_weight);
            return 0;
        }
    }

    free(in_tree);
    free(parent);
    free(best_weight);
    return 1;
}

static int ss_graph_tree_build_kruskal(
    const SsStructure *source,
    SsStructure *out_structure,
    SsError *error)
{
    SsWeightedEdgeRef *edges;
    SsWeightedEdgeRef *accepted;
    int *parent;
    int *rank;
    int *visited;
    size_t edge_count = 0;
    size_t accepted_count = 0;
    size_t *queue;
    size_t head = 0;
    size_t tail = 0;
    int root_index = 0;

    if (source->node_count == 0) {
        return 1;
    }

    edges = (SsWeightedEdgeRef *) malloc(source->edge_count * sizeof(*edges));
    accepted = (SsWeightedEdgeRef *) malloc(source->edge_count * sizeof(*accepted));
    parent = (int *) malloc(source->node_count * sizeof(*parent));
    rank = (int *) calloc(source->node_count, sizeof(*rank));
    visited = (int *) calloc(source->node_count, sizeof(*visited));
    queue = (size_t *) calloc(source->node_count, sizeof(*queue));
    if (edges == NULL || accepted == NULL || parent == NULL || rank == NULL || visited == NULL || queue == NULL) {
        free(edges);
        free(accepted);
        free(parent);
        free(rank);
        free(visited);
        free(queue);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para generar el arbol de Kruskal.");
        return 0;
    }

    for (size_t index = 0; index < source->node_count; ++index) {
        parent[index] = (int) index;
    }

    for (size_t index = 0; index < source->edge_count; ++index) {
        const SsEdge *edge = &source->edges[index];
        int source_index = ss_find_node_index(source, edge->source_id);
        int target_index = ss_find_node_index(source, edge->target_id);

        if (source_index < 0 || target_index < 0) {
            continue;
        }

        edges[edge_count].source_index = (size_t) source_index;
        edges[edge_count].target_index = (size_t) target_index;
        edges[edge_count].weight = ss_graph_edge_weight(edge);
        edge_count += 1;
    }

    qsort(edges, edge_count, sizeof(*edges), ss_weighted_edge_compare);
    for (size_t index = 0; index < edge_count; ++index) {
        if (ss_dsu_union(parent, rank, (int) edges[index].source_index, (int) edges[index].target_index)) {
            accepted[accepted_count++] = edges[index];
        }
    }

    if (accepted_count + 1 != source->node_count) {
        free(edges);
        free(accepted);
        free(parent);
        free(rank);
        free(visited);
        free(queue);
        ss_error_set(error, SS_ERROR_VALIDATION, "Kruskal requiere un grafo conexo.");
        return 0;
    }

    ss_str_copy(out_structure->root_id, sizeof(out_structure->root_id), source->nodes[(size_t) root_index].id);
    visited[(size_t) root_index] = 1;
    queue[tail++] = (size_t) root_index;
    while (head < tail) {
        size_t current = queue[head++];

        for (size_t index = 0; index < accepted_count; ++index) {
            int neighbor = -1;
            double weight = accepted[index].weight;

            if (accepted[index].source_index == current && !visited[accepted[index].target_index]) {
                neighbor = (int) accepted[index].target_index;
            } else if (accepted[index].target_index == current && !visited[accepted[index].source_index]) {
                neighbor = (int) accepted[index].source_index;
            }

            if (neighbor >= 0) {
                visited[(size_t) neighbor] = 1;
                queue[tail++] = (size_t) neighbor;
                if (!ss_graph_tree_append_edge(
                        source,
                        out_structure,
                        source->nodes[current].id,
                        source->nodes[(size_t) neighbor].id,
                        weight,
                        error)) {
                    free(edges);
                    free(accepted);
                    free(parent);
                    free(rank);
                    free(visited);
                    free(queue);
                    return 0;
                }
            }
        }
    }

    free(edges);
    free(accepted);
    free(parent);
    free(rank);
    free(visited);
    free(queue);
    return 1;
}

int ss_structure_rotate_left(SsStructure *structure, const char *pivot_node_id, SsError *error)
{
    SsNode *pivot;
    SsNode *child;
    SsNode *parent;
    SsNode *transfer;

    if (!ss_structure_can_rotate(structure)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La rotacion izquierda se reserva para BST y AVL.");
        return 0;
    }

    pivot = ss_structure_find_node(structure, pivot_node_id);
    if (pivot == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo pivote inexistente.");
        return 0;
    }

    child = ss_structure_find_node(structure, pivot->data.ref_b);
    if (child == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La rotacion izquierda requiere hijo derecho.");
        return 0;
    }

    parent = ss_structure_find_node(structure, pivot->data.ref_c);
    transfer = ss_structure_find_node(structure, child->data.ref_a);
    ss_tree_replace_parent_link(structure, parent, pivot->id, child->id);
    ss_str_copy(child->data.ref_c, sizeof(child->data.ref_c), parent != NULL ? parent->id : "");
    ss_str_copy(pivot->data.ref_b, sizeof(pivot->data.ref_b), transfer != NULL ? transfer->id : "");
    if (transfer != NULL) {
        ss_str_copy(transfer->data.ref_c, sizeof(transfer->data.ref_c), pivot->id);
    }
    ss_str_copy(child->data.ref_a, sizeof(child->data.ref_a), pivot->id);
    ss_str_copy(pivot->data.ref_c, sizeof(pivot->data.ref_c), child->id);
    ss_rebuild_tree_edges(structure);
    structure->dirty_layout = 1;
    ss_error_clear(error);
    return 1;
}

int ss_structure_rotate_right(SsStructure *structure, const char *pivot_node_id, SsError *error)
{
    SsNode *pivot;
    SsNode *child;
    SsNode *parent;
    SsNode *transfer;

    if (!ss_structure_can_rotate(structure)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La rotacion derecha se reserva para BST y AVL.");
        return 0;
    }

    pivot = ss_structure_find_node(structure, pivot_node_id);
    if (pivot == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo pivote inexistente.");
        return 0;
    }

    child = ss_structure_find_node(structure, pivot->data.ref_a);
    if (child == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La rotacion derecha requiere hijo izquierdo.");
        return 0;
    }

    parent = ss_structure_find_node(structure, pivot->data.ref_c);
    transfer = ss_structure_find_node(structure, child->data.ref_b);
    ss_tree_replace_parent_link(structure, parent, pivot->id, child->id);
    ss_str_copy(child->data.ref_c, sizeof(child->data.ref_c), parent != NULL ? parent->id : "");
    ss_str_copy(pivot->data.ref_a, sizeof(pivot->data.ref_a), transfer != NULL ? transfer->id : "");
    if (transfer != NULL) {
        ss_str_copy(transfer->data.ref_c, sizeof(transfer->data.ref_c), pivot->id);
    }
    ss_str_copy(child->data.ref_b, sizeof(child->data.ref_b), pivot->id);
    ss_str_copy(pivot->data.ref_c, sizeof(pivot->data.ref_c), child->id);
    ss_rebuild_tree_edges(structure);
    structure->dirty_layout = 1;
    ss_error_clear(error);
    return 1;
}

int ss_structure_rotate_graph_layout(SsStructure *structure, double radians, const char *pivot_node_id, SsError *error)
{
    double pivot_x;
    double pivot_y;
    double cosine_value;
    double sine_value;

    if (!ss_structure_can_rotate_graph_layout(structure)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La rotacion visual del grafo requiere una pestana de grafo con vertices.");
        return 0;
    }

    ss_graph_layout_pivot(structure, pivot_node_id, &pivot_x, &pivot_y);
    cosine_value = cos(radians);
    sine_value = sin(radians);

    for (size_t index = 0; index < structure->node_count; ++index) {
        SsNode *node = &structure->nodes[index];
        double center_x = node->visual.x + node->visual.width / 2.0;
        double center_y = node->visual.y + node->visual.height / 2.0;
        double offset_x = center_x - pivot_x;
        double offset_y = center_y - pivot_y;
        double rotated_x = pivot_x + offset_x * cosine_value - offset_y * sine_value;
        double rotated_y = pivot_y + offset_x * sine_value + offset_y * cosine_value;

        node->visual.x = rotated_x - node->visual.width / 2.0;
        node->visual.y = rotated_y - node->visual.height / 2.0;
    }

    ss_graph_layout_shift_to_positive_space(structure);
    structure->dirty_layout = 0;
    ss_error_clear(error);
    return 1;
}

int ss_structure_build_graph_tree(
    const SsStructure *source,
    SsAnalysisKind kind,
    const char *start_node_id,
    SsStructure *out_structure,
    char *out_message,
    size_t out_message_capacity,
    SsError *error)
{
    if (!ss_graph_tree_validate_source(source, kind, error)) {
        return 0;
    }
    if (out_structure == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Salida invalida para el arbol derivado.");
        return 0;
    }

    memset(out_structure, 0, sizeof(*out_structure));
    ss_structure_init(out_structure, source->variant, "");
    if (!ss_graph_tree_copy_nodes(source, out_structure, error)) {
        ss_structure_free(out_structure);
        return 0;
    }

    switch (kind) {
        case SS_ANALYSIS_GRAPH_BFS:
            if (!ss_graph_tree_build_bfs(source, start_node_id, out_structure, error)) {
                ss_structure_free(out_structure);
                return 0;
            }
            ss_graph_tree_prune_unreachable(out_structure);
            ss_str_copy(out_structure->visual_state.layout_mode, sizeof(out_structure->visual_state.layout_mode), "tree_bfs");
            ss_str_copy(out_message, out_message_capacity, "Arbol BFS generado.");
            break;
        case SS_ANALYSIS_GRAPH_DFS:
            if (!ss_graph_tree_build_dfs(source, start_node_id, out_structure, error)) {
                ss_structure_free(out_structure);
                return 0;
            }
            ss_graph_tree_prune_unreachable(out_structure);
            ss_str_copy(out_structure->visual_state.layout_mode, sizeof(out_structure->visual_state.layout_mode), "tree_dfs");
            ss_str_copy(out_message, out_message_capacity, "Arbol DFS generado.");
            break;
        case SS_ANALYSIS_GRAPH_PRIM:
            if (!ss_graph_tree_build_prim(source, start_node_id, out_structure, error)) {
                ss_structure_free(out_structure);
                return 0;
            }
            ss_str_copy(out_structure->visual_state.layout_mode, sizeof(out_structure->visual_state.layout_mode), "tree_prim");
            ss_str_copy(out_message, out_message_capacity, "Arbol de expansion minima generado con Prim.");
            break;
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            if (!ss_graph_tree_build_kruskal(source, out_structure, error)) {
                ss_structure_free(out_structure);
                return 0;
            }
            ss_str_copy(out_structure->visual_state.layout_mode, sizeof(out_structure->visual_state.layout_mode), "tree_kruskal");
            ss_str_copy(out_message, out_message_capacity, "Arbol de expansion minima generado con Kruskal.");
            break;
        default:
            ss_structure_free(out_structure);
            ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no genera arbol derivado.");
            return 0;
    }

    out_structure->dirty_layout = 1;
    ss_error_clear(error);
    return 1;
}
