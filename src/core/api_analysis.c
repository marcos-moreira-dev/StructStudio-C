/*
 * StructStudio C
 * --------------
 * Structural analysis and traversal helpers.
 *
 * This module adds educational read-only algorithms over the in-memory
 * structures without coupling them to the UI. The textual report remains the
 * stable summary contract; guided step playback lives in a companion module so
 * the summary path stays simple and reusable.
 */

#include "api_internal.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SsAnalysisAppendState {
    char *buffer;
    size_t capacity;
    int first;
} SsAnalysisAppendState;

/* The textual report is still a first-class educational output, so these
 * helpers assemble human-readable traversals without exposing raw buffers to
 * every algorithm implementation. */
static void ss_analysis_append_text(char *buffer, size_t capacity, const char *text)
{
    size_t length;

    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    length = strlen(buffer);
    if (length >= capacity - 1) {
        return;
    }

    ss_str_copy(buffer + length, capacity - length, text);
}

static void ss_analysis_append_line(char *buffer, size_t capacity, const char *text)
{
    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    if (buffer[0] != '\0') {
        ss_analysis_append_text(buffer, capacity, "\r\n");
    }
    ss_analysis_append_text(buffer, capacity, text);
}

static void ss_analysis_append_node_display(SsAnalysisAppendState *state, const SsNode *node)
{
    char text[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

    /* Reports prefer a visible label, then a value, and only fall back to the
     * technical node ID if there is no better human-facing token. */
    if (state == NULL || node == NULL) {
        return;
    }

    if (!state->first) {
        ss_analysis_append_text(state->buffer, state->capacity, " -> ");
    }

    if (node->label[0] != '\0') {
        ss_str_copy(text, sizeof(text), node->label);
    } else if (node->value[0] != '\0') {
        ss_str_copy(text, sizeof(text), node->value);
    } else {
        ss_str_copy(text, sizeof(text), node->id);
    }

    ss_analysis_append_text(state->buffer, state->capacity, text);
    state->first = 0;
}

static void ss_tree_preorder(
    const SsStructure *structure,
    const char *node_id,
    SsAnalysisAppendState *state)
{
    const SsNode *node = ss_structure_find_node_const(structure, node_id);

    /* Recursive traversals are kept explicit instead of compressed into one
     * generic walker because that is easier to study in a data-structures
     * course. */
    if (node == NULL) {
        return;
    }

    ss_analysis_append_node_display(state, node);
    if (node->data.ref_a[0] != '\0') {
        ss_tree_preorder(structure, node->data.ref_a, state);
    }
    if (node->data.ref_b[0] != '\0') {
        ss_tree_preorder(structure, node->data.ref_b, state);
    }
}

static void ss_tree_inorder(
    const SsStructure *structure,
    const char *node_id,
    SsAnalysisAppendState *state)
{
    const SsNode *node = ss_structure_find_node_const(structure, node_id);

    if (node == NULL) {
        return;
    }

    if (node->data.ref_a[0] != '\0') {
        ss_tree_inorder(structure, node->data.ref_a, state);
    }
    ss_analysis_append_node_display(state, node);
    if (node->data.ref_b[0] != '\0') {
        ss_tree_inorder(structure, node->data.ref_b, state);
    }
}

static void ss_tree_postorder(
    const SsStructure *structure,
    const char *node_id,
    SsAnalysisAppendState *state)
{
    const SsNode *node = ss_structure_find_node_const(structure, node_id);

    if (node == NULL) {
        return;
    }

    if (node->data.ref_a[0] != '\0') {
        ss_tree_postorder(structure, node->data.ref_a, state);
    }
    if (node->data.ref_b[0] != '\0') {
        ss_tree_postorder(structure, node->data.ref_b, state);
    }
    ss_analysis_append_node_display(state, node);
}

static void ss_tree_level_order(
    const SsStructure *structure,
    const char *start_node_id,
    SsAnalysisAppendState *state)
{
    char **queue;
    size_t head = 0;
    size_t tail = 0;
    size_t capacity = structure->node_count == 0 ? 1 : structure->node_count;

    /* Level-order uses a simple queue over node IDs. This mirrors the standard
     * breadth-first idea without introducing a separate queue abstraction. */
    queue = (char **) calloc(capacity, sizeof(*queue));
    if (queue == NULL) {
        return;
    }

    queue[tail++] = (char *) start_node_id;
    while (head < tail) {
        const SsNode *node = ss_structure_find_node_const(structure, queue[head++]);
        if (node == NULL) {
            continue;
        }

        ss_analysis_append_node_display(state, node);
        if (node->data.ref_a[0] != '\0' && tail < capacity) {
            queue[tail++] = (char *) node->data.ref_a;
        }
        if (node->data.ref_b[0] != '\0' && tail < capacity) {
            queue[tail++] = (char *) node->data.ref_b;
        }
    }

    free(queue);
}

static void ss_graph_neighbors(
    const SsStructure *structure,
    size_t node_index,
    size_t *neighbors,
    size_t *neighbor_count)
{
    size_t count = 0;
    const char *node_id = structure->nodes[node_index].id;

    /* Graph traversals repeatedly need neighbor lists. Building them through
     * edges here keeps BFS/DFS code smaller and more readable. */
    for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
        const SsEdge *edge = &structure->edges[edge_index];

        if (strcmp(edge->source_id, node_id) == 0) {
            int neighbor_index = ss_find_node_index(structure, edge->target_id);
            if (neighbor_index >= 0 && count < structure->node_count) {
                neighbors[count++] = (size_t) neighbor_index;
            }
        } else if (!edge->is_directed && strcmp(edge->target_id, node_id) == 0) {
            int neighbor_index = ss_find_node_index(structure, edge->source_id);
            if (neighbor_index >= 0 && count < structure->node_count) {
                neighbors[count++] = (size_t) neighbor_index;
            }
        }
    }

    *neighbor_count = count;
}

static void ss_graph_dfs_recursive(
    const SsStructure *structure,
    size_t node_index,
    int *visited,
    SsAnalysisAppendState *state)
{
    size_t *neighbors;
    size_t neighbor_count = 0;

    /* DFS stays recursive because the goal is pedagogical readability, not
     * squeezing every last byte of stack usage in this classroom-scale app. */
    visited[node_index] = 1;
    ss_analysis_append_node_display(state, &structure->nodes[node_index]);

    neighbors = (size_t *) calloc(structure->node_count == 0 ? 1 : structure->node_count, sizeof(*neighbors));
    if (neighbors == NULL) {
        return;
    }
    ss_graph_neighbors(structure, node_index, neighbors, &neighbor_count);
    for (size_t index = 0; index < neighbor_count; ++index) {
        if (!visited[neighbors[index]]) {
            ss_graph_dfs_recursive(structure, neighbors[index], visited, state);
        }
    }
    free(neighbors);
}

static int ss_run_tree_analysis(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    SsAnalysisAppendState state;
    const char *root_id = start_node_id != NULL && start_node_id[0] != '\0' ? start_node_id : structure->root_id;

    /* Tree analyses share one dispatcher that selects the traversal flavor and
     * emits a stable textual summary. */
    if (structure->node_count == 0 || root_id[0] == '\0') {
        ss_str_copy(report, report_capacity, "La estructura arborea esta vacia.");
        ss_error_clear(error);
        return 1;
    }

    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
            ss_str_copy(report, report_capacity, "Preorden: ");
            break;
        case SS_ANALYSIS_TREE_INORDER:
            ss_str_copy(report, report_capacity, "Inorden: ");
            break;
        case SS_ANALYSIS_TREE_POSTORDER:
            ss_str_copy(report, report_capacity, "Postorden: ");
            break;
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            ss_str_copy(report, report_capacity, "Por niveles: ");
            break;
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "Analisis de arbol no soportado.");
            return 0;
    }

    state.buffer = report;
    state.capacity = report_capacity;
    state.first = 1;
    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
            ss_tree_preorder(structure, root_id, &state);
            break;
        case SS_ANALYSIS_TREE_INORDER:
            ss_tree_inorder(structure, root_id, &state);
            break;
        case SS_ANALYSIS_TREE_POSTORDER:
            ss_tree_postorder(structure, root_id, &state);
            break;
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            ss_tree_level_order(structure, root_id, &state);
            break;
        default:
            break;
    }

    ss_error_clear(error);
    return 1;
}

static int ss_run_graph_bfs(
    const SsStructure *structure,
    size_t start_index,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    int *visited;
    size_t *queue;
    size_t *neighbors;
    size_t head = 0;
    size_t tail = 0;
    SsAnalysisAppendState state;

    visited = (int *) calloc(structure->node_count, sizeof(*visited));
    queue = (size_t *) calloc(structure->node_count, sizeof(*queue));
    neighbors = (size_t *) calloc(structure->node_count, sizeof(*neighbors));
    if (visited == NULL || queue == NULL || neighbors == NULL) {
        free(visited);
        free(queue);
        free(neighbors);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para BFS.");
        return 0;
    }

    ss_str_copy(report, report_capacity, "BFS: ");
    state.buffer = report;
    state.capacity = report_capacity;
    state.first = 1;
    queue[tail++] = start_index;
    visited[start_index] = 1;

    while (head < tail) {
        size_t current = queue[head++];
        size_t neighbor_count = 0;

        ss_analysis_append_node_display(&state, &structure->nodes[current]);
        ss_graph_neighbors(structure, current, neighbors, &neighbor_count);
        for (size_t index = 0; index < neighbor_count; ++index) {
            if (!visited[neighbors[index]]) {
                visited[neighbors[index]] = 1;
                queue[tail++] = neighbors[index];
            }
        }
    }

    free(visited);
    free(queue);
    free(neighbors);
    ss_error_clear(error);
    return 1;
}

static int ss_run_graph_dfs(
    const SsStructure *structure,
    size_t start_index,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    int *visited = (int *) calloc(structure->node_count, sizeof(*visited));
    SsAnalysisAppendState state;

    if (visited == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para DFS.");
        return 0;
    }

    ss_str_copy(report, report_capacity, "DFS: ");
    state.buffer = report;
    state.capacity = report_capacity;
    state.first = 1;
    ss_graph_dfs_recursive(structure, start_index, visited, &state);
    free(visited);
    ss_error_clear(error);
    return 1;
}

static double ss_graph_edge_weight(const SsEdge *edge)
{
    return edge->has_weight ? edge->weight : 1.0;
}

static int ss_graph_has_negative_weights(const SsStructure *structure)
{
    for (size_t index = 0; index < structure->edge_count; ++index) {
        if (ss_graph_edge_weight(&structure->edges[index]) < 0.0) {
            return 1;
        }
    }

    return 0;
}

static void ss_append_path_recursive(
    const SsStructure *structure,
    const int *previous,
    size_t index,
    SsAnalysisAppendState *state)
{
    if (previous[index] >= 0) {
        ss_append_path_recursive(structure, previous, (size_t) previous[index], state);
    }
    ss_analysis_append_node_display(state, &structure->nodes[index]);
}

static int ss_run_graph_dijkstra(
    const SsStructure *structure,
    size_t start_index,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    double *distance;
    int *visited;
    int *previous;
    char line[512];

    distance = (double *) malloc(structure->node_count * sizeof(*distance));
    visited = (int *) calloc(structure->node_count, sizeof(*visited));
    previous = (int *) malloc(structure->node_count * sizeof(*previous));
    if (distance == NULL || visited == NULL || previous == NULL) {
        free(distance);
        free(visited);
        free(previous);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para Dijkstra.");
        return 0;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        distance[index] = DBL_MAX;
        previous[index] = -1;
    }
    distance[start_index] = 0.0;

    for (;;) {
        size_t current = structure->node_count;
        double best = DBL_MAX;

        for (size_t index = 0; index < structure->node_count; ++index) {
            if (!visited[index] && distance[index] < best) {
                best = distance[index];
                current = index;
            }
        }
        if (current == structure->node_count) {
            break;
        }

        visited[current] = 1;
        for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
            const SsEdge *edge = &structure->edges[edge_index];
            size_t neighbor = structure->node_count;

            if (strcmp(edge->source_id, structure->nodes[current].id) == 0) {
                int target_index = ss_find_node_index(structure, edge->target_id);
                if (target_index >= 0) {
                    neighbor = (size_t) target_index;
                }
            } else if (!edge->is_directed && strcmp(edge->target_id, structure->nodes[current].id) == 0) {
                int source_index = ss_find_node_index(structure, edge->source_id);
                if (source_index >= 0) {
                    neighbor = (size_t) source_index;
                }
            }

            if (neighbor != structure->node_count && !visited[neighbor]) {
                double candidate = distance[current] + ss_graph_edge_weight(edge);
                if (candidate < distance[neighbor]) {
                    distance[neighbor] = candidate;
                    previous[neighbor] = (int) current;
                }
            }
        }
    }

    report[0] = '\0';
    snprintf(line, sizeof(line), "Dijkstra desde %s", structure->nodes[start_index].label[0] != '\0' ? structure->nodes[start_index].label : structure->nodes[start_index].id);
    ss_analysis_append_line(report, report_capacity, line);
    for (size_t index = 0; index < structure->node_count; ++index) {
        if (distance[index] == DBL_MAX) {
            snprintf(line, sizeof(line), "%s = INF", structure->nodes[index].label[0] != '\0' ? structure->nodes[index].label : structure->nodes[index].id);
            ss_analysis_append_line(report, report_capacity, line);
        } else {
            char path[512] = "";
            SsAnalysisAppendState path_state;
            const char *node_name = structure->nodes[index].label[0] != '\0'
                ? structure->nodes[index].label
                : structure->nodes[index].id;
            char distance_text[32];

            path_state.buffer = path;
            path_state.capacity = sizeof(path);
            path_state.first = 1;
            ss_append_path_recursive(structure, previous, index, &path_state);
            snprintf(distance_text, sizeof(distance_text), "%.0f", distance[index]);
            line[0] = '\0';
            ss_analysis_append_text(line, sizeof(line), node_name);
            ss_analysis_append_text(line, sizeof(line), " = ");
            ss_analysis_append_text(line, sizeof(line), distance_text);
            ss_analysis_append_text(line, sizeof(line), " | camino: ");
            ss_analysis_append_text(line, sizeof(line), path);
            ss_analysis_append_line(report, report_capacity, line);
        }
    }

    free(distance);
    free(visited);
    free(previous);
    ss_error_clear(error);
    return 1;
}

static int ss_run_graph_analysis(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    int start_index;

    if (start_node_id == NULL || start_node_id[0] == '\0') {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un vertice origen o indique uno en el campo de analisis.");
        return 0;
    }

    start_index = ss_find_node_index(structure, start_node_id);
    if (start_index < 0) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "El vertice de inicio no existe.");
        return 0;
    }

    if (kind == SS_ANALYSIS_GRAPH_DIJKSTRA && !structure->config.is_weighted) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Dijkstra requiere un grafo ponderado.");
        return 0;
    }
    if (kind == SS_ANALYSIS_GRAPH_DIJKSTRA && ss_graph_has_negative_weights(structure)) {
        ss_error_set(error, SS_ERROR_VALIDATION, "Dijkstra no admite pesos negativos.");
        return 0;
    }

    switch (kind) {
        case SS_ANALYSIS_GRAPH_BFS:
            return ss_run_graph_bfs(structure, (size_t) start_index, report, report_capacity, error);
        case SS_ANALYSIS_GRAPH_DFS:
            return ss_run_graph_dfs(structure, (size_t) start_index, report, report_capacity, error);
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            return ss_run_graph_dijkstra(structure, (size_t) start_index, report, report_capacity, error);
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "Analisis de grafo no soportado.");
            return 0;
    }
}

const char *ss_analysis_kind_label(SsAnalysisKind kind)
{
    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
            return "Preorden";
        case SS_ANALYSIS_TREE_INORDER:
            return "Inorden";
        case SS_ANALYSIS_TREE_POSTORDER:
            return "Postorden";
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            return "Por niveles";
        case SS_ANALYSIS_GRAPH_BFS:
            return "BFS";
        case SS_ANALYSIS_GRAPH_DFS:
            return "DFS";
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            return "Dijkstra";
        case SS_ANALYSIS_GRAPH_FLOYD_WARSHALL:
            return "Floyd-Warshall";
        case SS_ANALYSIS_GRAPH_PRIM:
            return "Prim";
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            return "Kruskal";
        default:
            return "Analisis";
    }
}

size_t ss_analysis_kinds_for_variant(SsVariant variant, SsAnalysisKind *items, size_t capacity)
{
    size_t count = 0;

    if (items == NULL || capacity == 0) {
        return 0;
    }

    switch (variant) {
        case SS_VARIANT_BINARY_TREE:
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
            if (count < capacity) items[count++] = SS_ANALYSIS_TREE_PREORDER;
            if (count < capacity) items[count++] = SS_ANALYSIS_TREE_INORDER;
            if (count < capacity) items[count++] = SS_ANALYSIS_TREE_POSTORDER;
            if (count < capacity) items[count++] = SS_ANALYSIS_TREE_LEVEL_ORDER;
            break;
        case SS_VARIANT_HEAP:
            if (count < capacity) items[count++] = SS_ANALYSIS_TREE_LEVEL_ORDER;
            break;
        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
            if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_BFS;
            if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_DFS;
            break;
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_BFS;
            if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_DFS;
            if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_DIJKSTRA;
            if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_FLOYD_WARSHALL;
            if (variant == SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH) {
                if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_PRIM;
                if (count < capacity) items[count++] = SS_ANALYSIS_GRAPH_KRUSKAL;
            }
            break;
        default:
            break;
    }

    return count;
}

SsAnalysisStartMode ss_analysis_start_mode(SsAnalysisKind kind)
{
    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
        case SS_ANALYSIS_TREE_INORDER:
        case SS_ANALYSIS_TREE_POSTORDER:
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
        case SS_ANALYSIS_GRAPH_PRIM:
            return SS_ANALYSIS_START_OPTIONAL;
        case SS_ANALYSIS_GRAPH_BFS:
        case SS_ANALYSIS_GRAPH_DFS:
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            return SS_ANALYSIS_START_REQUIRED;
        case SS_ANALYSIS_GRAPH_FLOYD_WARSHALL:
        case SS_ANALYSIS_GRAPH_KRUSKAL:
        default:
            return SS_ANALYSIS_START_NONE;
    }
}

int ss_analysis_requires_start_node(SsAnalysisKind kind)
{
    return ss_analysis_start_mode(kind) == SS_ANALYSIS_START_REQUIRED;
}

int ss_structure_run_analysis(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    if (report == NULL || report_capacity == 0) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Buffer de analisis invalido.");
        return 0;
    }

    report[0] = '\0';
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
        case SS_ANALYSIS_TREE_INORDER:
        case SS_ANALYSIS_TREE_POSTORDER:
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            if (structure->family != SS_FAMILY_TREE && structure->family != SS_FAMILY_HEAP) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no aplica a la estructura activa.");
                return 0;
            }
            return ss_run_tree_analysis(structure, kind, start_node_id, report, report_capacity, error);

        case SS_ANALYSIS_GRAPH_BFS:
        case SS_ANALYSIS_GRAPH_DFS:
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            if (structure->family != SS_FAMILY_GRAPH) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no aplica a la estructura activa.");
                return 0;
            }
            return ss_run_graph_analysis(structure, kind, start_node_id, report, report_capacity, error);

        case SS_ANALYSIS_GRAPH_FLOYD_WARSHALL:
        case SS_ANALYSIS_GRAPH_PRIM:
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            if (structure->family != SS_FAMILY_GRAPH) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no aplica a la estructura activa.");
                return 0;
            }
            return ss_run_graph_advanced_analysis(structure, kind, start_node_id, report, report_capacity, error);

        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "Analisis no soportado.");
            return 0;
    }
}
