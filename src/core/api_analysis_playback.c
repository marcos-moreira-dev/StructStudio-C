/*
 * StructStudio C
 * --------------
 * Step-by-step analysis playback builders.
 *
 * The textual report remains the stable public summary, while this module
 * derives a sequence of pedagogical steps that the editor can replay on the
 * canvas without teaching the render layer how the algorithms work.
 */

#include "api_internal.h"

#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SsPlaybackBuildState {
    SsAnalysisStep *items;
    size_t count;
    size_t capacity;
} SsPlaybackBuildState;

typedef struct SsTreeQueueItem {
    const char *node_id;
    const char *parent_id;
} SsTreeQueueItem;

static void ss_node_display_name(const SsNode *node, char *buffer, size_t capacity)
{
    if (buffer == NULL || capacity == 0) {
        return;
    }

    if (node == NULL) {
        buffer[0] = '\0';
        return;
    }

    if (node->label[0] != '\0') {
        ss_str_copy(buffer, capacity, node->label);
    } else if (node->value[0] != '\0') {
        ss_str_copy(buffer, capacity, node->value);
    } else {
        ss_str_copy(buffer, capacity, node->id);
    }
}

static const SsEdge *ss_find_edge_between(const SsStructure *structure, const char *source_id, const char *target_id)
{
    if (structure == NULL || source_id == NULL || target_id == NULL) {
        return NULL;
    }

    for (size_t index = 0; index < structure->edge_count; ++index) {
        const SsEdge *edge = &structure->edges[index];
        if (strcmp(edge->source_id, source_id) == 0 && strcmp(edge->target_id, target_id) == 0) {
            return edge;
        }
        if (!edge->is_directed && strcmp(edge->source_id, target_id) == 0 && strcmp(edge->target_id, source_id) == 0) {
            return edge;
        }
    }

    return NULL;
}

static double ss_graph_edge_weight(const SsEdge *edge)
{
    return edge != NULL && edge->has_weight ? edge->weight : 1.0;
}

static int ss_step_pushf(
    SsPlaybackBuildState *state,
    SsAnalysisStepKind kind,
    const char *node_id,
    const char *edge_id,
    SsError *error,
    const char *fmt,
    ...)
{
    SsAnalysisStep *step;
    va_list args;

    if (state == NULL || fmt == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Paso de analisis invalido.");
        return 0;
    }

    if (!ss_array_reserve((void **) &state->items, sizeof(*state->items), &state->capacity, state->count + 1)) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para el recorrido guiado.");
        return 0;
    }

    step = &state->items[state->count++];
    memset(step, 0, sizeof(*step));
    step->kind = kind;
    ss_str_copy(step->node_id, sizeof(step->node_id), node_id);
    ss_str_copy(step->edge_id, sizeof(step->edge_id), edge_id);
    va_start(args, fmt);
    vsnprintf(step->message, sizeof(step->message), fmt, args);
    va_end(args);
    return 1;
}

static int ss_build_tree_preorder_steps(
    const SsStructure *structure,
    const char *node_id,
    const char *parent_id,
    SsPlaybackBuildState *state,
    SsError *error)
{
    const SsNode *node = ss_structure_find_node_const(structure, node_id);
    const SsEdge *edge;
    char name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

    if (node == NULL) {
        return 1;
    }

    edge = parent_id != NULL ? ss_find_edge_between(structure, parent_id, node->id) : NULL;
    ss_node_display_name(node, name, sizeof(name));
    if (!ss_step_pushf(state, SS_ANALYSIS_STEP_VISIT, node->id, edge != NULL ? edge->id : "", error, "Visitar %s en preorden.", name)) {
        return 0;
    }
    if (node->data.ref_a[0] != '\0' && !ss_build_tree_preorder_steps(structure, node->data.ref_a, node->id, state, error)) {
        return 0;
    }
    if (node->data.ref_b[0] != '\0' && !ss_build_tree_preorder_steps(structure, node->data.ref_b, node->id, state, error)) {
        return 0;
    }
    return 1;
}

static int ss_build_tree_inorder_steps(
    const SsStructure *structure,
    const char *node_id,
    const char *parent_id,
    SsPlaybackBuildState *state,
    SsError *error)
{
    const SsNode *node = ss_structure_find_node_const(structure, node_id);
    const SsEdge *edge;
    char name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

    if (node == NULL) {
        return 1;
    }

    if (node->data.ref_a[0] != '\0' && !ss_build_tree_inorder_steps(structure, node->data.ref_a, node->id, state, error)) {
        return 0;
    }

    edge = parent_id != NULL ? ss_find_edge_between(structure, parent_id, node->id) : NULL;
    ss_node_display_name(node, name, sizeof(name));
    if (!ss_step_pushf(state, SS_ANALYSIS_STEP_VISIT, node->id, edge != NULL ? edge->id : "", error, "Visitar %s en inorden.", name)) {
        return 0;
    }

    if (node->data.ref_b[0] != '\0' && !ss_build_tree_inorder_steps(structure, node->data.ref_b, node->id, state, error)) {
        return 0;
    }

    return 1;
}

static int ss_build_tree_postorder_steps(
    const SsStructure *structure,
    const char *node_id,
    const char *parent_id,
    SsPlaybackBuildState *state,
    SsError *error)
{
    const SsNode *node = ss_structure_find_node_const(structure, node_id);
    const SsEdge *edge;
    char name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

    if (node == NULL) {
        return 1;
    }

    if (node->data.ref_a[0] != '\0' && !ss_build_tree_postorder_steps(structure, node->data.ref_a, node->id, state, error)) {
        return 0;
    }
    if (node->data.ref_b[0] != '\0' && !ss_build_tree_postorder_steps(structure, node->data.ref_b, node->id, state, error)) {
        return 0;
    }

    edge = parent_id != NULL ? ss_find_edge_between(structure, parent_id, node->id) : NULL;
    ss_node_display_name(node, name, sizeof(name));
    return ss_step_pushf(state, SS_ANALYSIS_STEP_VISIT, node->id, edge != NULL ? edge->id : "", error, "Visitar %s en postorden.", name);
}

static int ss_build_tree_level_steps(
    const SsStructure *structure,
    const char *start_node_id,
    SsPlaybackBuildState *state,
    SsError *error)
{
    size_t capacity = structure->node_count == 0 ? 1 : structure->node_count;
    SsTreeQueueItem *queue = (SsTreeQueueItem *) calloc(capacity, sizeof(*queue));
    size_t head = 0;
    size_t tail = 0;

    if (queue == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para el recorrido por niveles.");
        return 0;
    }

    queue[tail].node_id = start_node_id;
    queue[tail].parent_id = NULL;
    ++tail;
    while (head < tail) {
        const SsNode *node = ss_structure_find_node_const(structure, queue[head].node_id);
        const char *parent_id = queue[head].parent_id;
        const SsEdge *edge;
        char name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];
        ++head;

        if (node == NULL) {
            continue;
        }

        edge = parent_id != NULL ? ss_find_edge_between(structure, parent_id, node->id) : NULL;
        ss_node_display_name(node, name, sizeof(name));
        if (!ss_step_pushf(state, SS_ANALYSIS_STEP_VISIT, node->id, edge != NULL ? edge->id : "", error, "Visitar %s en el recorrido por niveles.", name)) {
            free(queue);
            return 0;
        }

        if (node->data.ref_a[0] != '\0' && tail < capacity) {
            queue[tail].node_id = node->data.ref_a;
            queue[tail].parent_id = node->id;
            ++tail;
        }
        if (node->data.ref_b[0] != '\0' && tail < capacity) {
            queue[tail].node_id = node->data.ref_b;
            queue[tail].parent_id = node->id;
            ++tail;
        }
    }

    free(queue);
    return 1;
}

static int ss_build_graph_bfs_steps(
    const SsStructure *structure,
    size_t start_index,
    SsPlaybackBuildState *state,
    SsError *error)
{
    int *visited = (int *) calloc(structure->node_count, sizeof(*visited));
    size_t *queue = (size_t *) calloc(structure->node_count, sizeof(*queue));
    size_t head = 0;
    size_t tail = 0;

    if (visited == NULL || queue == NULL) {
        free(visited);
        free(queue);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para BFS paso a paso.");
        return 0;
    }

    visited[start_index] = 1;
    queue[tail++] = start_index;
    while (head < tail) {
        size_t current = queue[head++];
        const SsNode *node = &structure->nodes[current];
        char name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

        ss_node_display_name(node, name, sizeof(name));
        if (!ss_step_pushf(state, SS_ANALYSIS_STEP_VISIT, node->id, "", error, "Visitar %s y expandir su nivel.", name)) {
            free(visited);
            free(queue);
            return 0;
        }

        for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
            const SsEdge *edge = &structure->edges[edge_index];
            int neighbor_index = -1;

            if (strcmp(edge->source_id, node->id) == 0) {
                neighbor_index = ss_find_node_index(structure, edge->target_id);
            } else if (!edge->is_directed && strcmp(edge->target_id, node->id) == 0) {
                neighbor_index = ss_find_node_index(structure, edge->source_id);
            }

            if (neighbor_index >= 0 && !visited[neighbor_index]) {
                char neighbor_name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];
                visited[neighbor_index] = 1;
                queue[tail++] = (size_t) neighbor_index;
                ss_node_display_name(&structure->nodes[neighbor_index], neighbor_name, sizeof(neighbor_name));
                if (!ss_step_pushf(
                        state,
                        SS_ANALYSIS_STEP_DISCOVER,
                        structure->nodes[neighbor_index].id,
                        edge->id,
                        error,
                        "Descubrir %s desde %s y encolarlo.",
                        neighbor_name,
                        name)) {
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

static int ss_build_graph_dfs_recursive(
    const SsStructure *structure,
    size_t current_index,
    int *visited,
    SsPlaybackBuildState *state,
    SsError *error)
{
    const SsNode *node = &structure->nodes[current_index];
    char name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

    visited[current_index] = 1;
    ss_node_display_name(node, name, sizeof(name));
    if (!ss_step_pushf(state, SS_ANALYSIS_STEP_VISIT, node->id, "", error, "Visitar %s y profundizar.", name)) {
        return 0;
    }

    for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
        const SsEdge *edge = &structure->edges[edge_index];
        int neighbor_index = -1;

        if (strcmp(edge->source_id, node->id) == 0) {
            neighbor_index = ss_find_node_index(structure, edge->target_id);
        } else if (!edge->is_directed && strcmp(edge->target_id, node->id) == 0) {
            neighbor_index = ss_find_node_index(structure, edge->source_id);
        }

        if (neighbor_index >= 0 && !visited[neighbor_index]) {
            char neighbor_name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];
            ss_node_display_name(&structure->nodes[neighbor_index], neighbor_name, sizeof(neighbor_name));
            if (!ss_step_pushf(
                    state,
                    SS_ANALYSIS_STEP_DISCOVER,
                    structure->nodes[neighbor_index].id,
                    edge->id,
                    error,
                    "Seguir desde %s hacia %s.",
                    name,
                    neighbor_name)) {
                return 0;
            }
            if (!ss_build_graph_dfs_recursive(structure, (size_t) neighbor_index, visited, state, error)) {
                return 0;
            }
        }
    }

    return 1;
}

static int ss_build_graph_dfs_steps(
    const SsStructure *structure,
    size_t start_index,
    SsPlaybackBuildState *state,
    SsError *error)
{
    int *visited = (int *) calloc(structure->node_count, sizeof(*visited));
    int ok;

    if (visited == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para DFS paso a paso.");
        return 0;
    }

    ok = ss_build_graph_dfs_recursive(structure, start_index, visited, state, error);
    free(visited);
    return ok;
}

static int ss_build_graph_dijkstra_steps(
    const SsStructure *structure,
    size_t start_index,
    SsPlaybackBuildState *state,
    SsError *error)
{
    double *distance = (double *) malloc(structure->node_count * sizeof(*distance));
    int *visited = (int *) calloc(structure->node_count, sizeof(*visited));
    int *previous = (int *) malloc(structure->node_count * sizeof(*previous));

    if (distance == NULL || visited == NULL || previous == NULL) {
        free(distance);
        free(visited);
        free(previous);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para Dijkstra paso a paso.");
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
        char current_name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];

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
        ss_node_display_name(&structure->nodes[current], current_name, sizeof(current_name));
        if (!ss_step_pushf(
                state,
                SS_ANALYSIS_STEP_FINALIZE,
                structure->nodes[current].id,
                "",
                error,
                "Fijar %s con distancia %.0f.",
                current_name,
                distance[current])) {
            free(distance);
            free(visited);
            free(previous);
            return 0;
        }

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
                double previous_distance = distance[neighbor];
                double candidate = distance[current] + ss_graph_edge_weight(edge);

                if (candidate < distance[neighbor]) {
                    char neighbor_name[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];
                    char previous_text[32];
                    distance[neighbor] = candidate;
                    previous[neighbor] = (int) current;
                    ss_node_display_name(&structure->nodes[neighbor], neighbor_name, sizeof(neighbor_name));
                    if (previous_distance == DBL_MAX) {
                        ss_str_copy(previous_text, sizeof(previous_text), "INF");
                    } else {
                        snprintf(previous_text, sizeof(previous_text), "%.0f", previous_distance);
                    }
                    if (!ss_step_pushf(
                            state,
                            SS_ANALYSIS_STEP_RELAX,
                            structure->nodes[neighbor].id,
                            edge->id,
                            error,
                            "Relajar %s: %s -> %.0f usando %s.",
                            neighbor_name,
                            previous_text,
                            candidate,
                            current_name)) {
                        free(distance);
                        free(visited);
                        free(previous);
                        return 0;
                    }
                }
            }
        }
    }

    free(distance);
    free(visited);
    free(previous);
    return 1;
}

static int ss_build_tree_steps(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    SsPlaybackBuildState *state,
    SsError *error)
{
    const char *effective_start = start_node_id != NULL && start_node_id[0] != '\0' ? start_node_id : structure->root_id;

    if (structure->node_count == 0 || effective_start[0] == '\0') {
        return 1;
    }
    if (ss_structure_find_node_const(structure, effective_start) == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "El nodo inicial del recorrido no existe.");
        return 0;
    }

    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
            return ss_build_tree_preorder_steps(structure, effective_start, NULL, state, error);
        case SS_ANALYSIS_TREE_INORDER:
            return ss_build_tree_inorder_steps(structure, effective_start, NULL, state, error);
        case SS_ANALYSIS_TREE_POSTORDER:
            return ss_build_tree_postorder_steps(structure, effective_start, NULL, state, error);
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            return ss_build_tree_level_steps(structure, effective_start, state, error);
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "Recorrido de arbol no soportado para playback.");
            return 0;
    }
}

static int ss_build_graph_steps(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    SsPlaybackBuildState *state,
    SsError *error)
{
    int start_index;

    if (start_node_id == NULL || start_node_id[0] == '\0') {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un vertice origen para el recorrido guiado.");
        return 0;
    }

    start_index = ss_find_node_index(structure, start_node_id);
    if (start_index < 0) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "El vertice de origen no existe.");
        return 0;
    }

    switch (kind) {
        case SS_ANALYSIS_GRAPH_BFS:
            return ss_build_graph_bfs_steps(structure, (size_t) start_index, state, error);
        case SS_ANALYSIS_GRAPH_DFS:
            return ss_build_graph_dfs_steps(structure, (size_t) start_index, state, error);
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            return ss_build_graph_dijkstra_steps(structure, (size_t) start_index, state, error);
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "Recorrido de grafo no soportado para playback.");
            return 0;
    }
}

const char *ss_analysis_step_kind_label(SsAnalysisStepKind kind)
{
    switch (kind) {
        case SS_ANALYSIS_STEP_VISIT:
            return "Visita";
        case SS_ANALYSIS_STEP_DISCOVER:
            return "Descubrimiento";
        case SS_ANALYSIS_STEP_RELAX:
            return "Relajacion";
        case SS_ANALYSIS_STEP_FINALIZE:
            return "Cierre";
        default:
            return "Paso";
    }
}

int ss_analysis_supports_playback(SsAnalysisKind kind)
{
    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
        case SS_ANALYSIS_TREE_INORDER:
        case SS_ANALYSIS_TREE_POSTORDER:
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
        case SS_ANALYSIS_GRAPH_BFS:
        case SS_ANALYSIS_GRAPH_DFS:
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            return 1;
        default:
            return 0;
    }
}

int ss_analysis_supports_tree_generation(SsAnalysisKind kind)
{
    switch (kind) {
        case SS_ANALYSIS_GRAPH_BFS:
        case SS_ANALYSIS_GRAPH_DFS:
        case SS_ANALYSIS_GRAPH_PRIM:
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            return 1;
        default:
            return 0;
    }
}

int ss_structure_build_analysis_playback(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    SsAnalysisStep **steps,
    size_t *step_count,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    SsPlaybackBuildState state;
    int ok;

    if (steps == NULL || step_count == NULL || report == NULL || report_capacity == 0) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Salida invalida para el recorrido guiado.");
        return 0;
    }

    *steps = NULL;
    *step_count = 0;
    state.items = NULL;
    state.count = 0;
    state.capacity = 0;
    if (!ss_analysis_supports_playback(kind)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no tiene recorrido guiado.");
        return 0;
    }
    if (!ss_structure_run_analysis(structure, kind, start_node_id, report, report_capacity, error)) {
        return 0;
    }

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    if (structure->family == SS_FAMILY_TREE || structure->family == SS_FAMILY_HEAP) {
        ok = ss_build_tree_steps(structure, kind, start_node_id, &state, error);
    } else if (structure->family == SS_FAMILY_GRAPH) {
        ok = ss_build_graph_steps(structure, kind, start_node_id, &state, error);
    } else {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La estructura activa no soporta recorrido guiado.");
        ok = 0;
    }

    if (!ok) {
        free(state.items);
        return 0;
    }

    *steps = state.items;
    *step_count = state.count;
    ss_error_clear(error);
    return 1;
}
