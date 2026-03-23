/*
 * StructStudio C
 * --------------
 * Advanced graph analyses.
 *
 * This module groups graph algorithms that are slightly heavier or more
 * specialized than BFS/DFS/Dijkstra. Keeping them here avoids turning the
 * main analysis file into a single oversized translation unit.
 */

#include "api_internal.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SsWeightedEdgeRef {
    size_t source_index;
    size_t target_index;
    double weight;
} SsWeightedEdgeRef;

static void ss_graph_append_text(char *buffer, size_t capacity, const char *text)
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

static void ss_graph_append_line(char *buffer, size_t capacity, const char *text)
{
    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    if (buffer[0] != '\0') {
        ss_graph_append_text(buffer, capacity, "\r\n");
    }
    ss_graph_append_text(buffer, capacity, text);
}

static const char *ss_graph_node_name(const SsNode *node)
{
    if (node == NULL) {
        return "";
    }
    if (node->label[0] != '\0') {
        return node->label;
    }
    if (node->value[0] != '\0') {
        return node->value;
    }
    return node->id;
}

static double ss_graph_edge_weight(const SsEdge *edge)
{
    return edge->has_weight ? edge->weight : 1.0;
}

static void ss_graph_format_weight(double value, char *buffer, size_t capacity)
{
    if (buffer == NULL || capacity == 0) {
        return;
    }

    snprintf(buffer, capacity, "%.0f", value);
}

static int ss_graph_validate_mst_requirements(const SsStructure *structure, SsError *error)
{
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay una estructura de grafo activa.");
        return 0;
    }
    if (structure->family != SS_FAMILY_GRAPH || !structure->config.is_weighted || structure->config.is_directed) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado requiere un grafo no dirigido y ponderado.");
        return 0;
    }

    return 1;
}

static int ss_graph_qsort_edges(const void *left, const void *right)
{
    const SsWeightedEdgeRef *left_edge = (const SsWeightedEdgeRef *) left;
    const SsWeightedEdgeRef *right_edge = (const SsWeightedEdgeRef *) right;

    if (left_edge->weight < right_edge->weight) {
        return -1;
    }
    if (left_edge->weight > right_edge->weight) {
        return 1;
    }
    if (left_edge->source_index < right_edge->source_index) {
        return -1;
    }
    if (left_edge->source_index > right_edge->source_index) {
        return 1;
    }
    if (left_edge->target_index < right_edge->target_index) {
        return -1;
    }
    if (left_edge->target_index > right_edge->target_index) {
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
        rank[root_left]++;
    }

    return 1;
}

static int ss_run_graph_floyd_warshall(
    const SsStructure *structure,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    const double inf = DBL_MAX / 8.0;
    double *distance;
    char line[1024];
    size_t matrix_size;

    if (structure->node_count == 0) {
        ss_str_copy(report, report_capacity, "El grafo esta vacio.");
        ss_error_clear(error);
        return 1;
    }

    matrix_size = structure->node_count * structure->node_count;
    distance = (double *) malloc(matrix_size * sizeof(*distance));
    if (distance == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para Floyd-Warshall.");
        return 0;
    }

    for (size_t row = 0; row < structure->node_count; ++row) {
        for (size_t column = 0; column < structure->node_count; ++column) {
            distance[row * structure->node_count + column] = row == column ? 0.0 : inf;
        }
    }

    for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
        const SsEdge *edge = &structure->edges[edge_index];
        int source_index = ss_find_node_index(structure, edge->source_id);
        int target_index = ss_find_node_index(structure, edge->target_id);
        double weight = ss_graph_edge_weight(edge);

        if (source_index < 0 || target_index < 0) {
            continue;
        }
        if (weight < distance[(size_t) source_index * structure->node_count + (size_t) target_index]) {
            distance[(size_t) source_index * structure->node_count + (size_t) target_index] = weight;
        }
        if (!edge->is_directed &&
            weight < distance[(size_t) target_index * structure->node_count + (size_t) source_index]) {
            distance[(size_t) target_index * structure->node_count + (size_t) source_index] = weight;
        }
    }

    for (size_t pivot = 0; pivot < structure->node_count; ++pivot) {
        for (size_t row = 0; row < structure->node_count; ++row) {
            for (size_t column = 0; column < structure->node_count; ++column) {
                double via_left = distance[row * structure->node_count + pivot];
                double via_right = distance[pivot * structure->node_count + column];
                double *current = &distance[row * structure->node_count + column];

                if (via_left >= inf || via_right >= inf) {
                    continue;
                }
                if (via_left + via_right < *current) {
                    *current = via_left + via_right;
                }
            }
        }
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        if (distance[index * structure->node_count + index] < 0.0) {
            free(distance);
            ss_error_set(error, SS_ERROR_VALIDATION, "Floyd-Warshall detecto un ciclo negativo.");
            return 0;
        }
    }

    report[0] = '\0';
    ss_graph_append_line(report, report_capacity, "Floyd-Warshall");
    for (size_t row = 0; row < structure->node_count; ++row) {
        ss_str_copy(line, sizeof(line), ss_graph_node_name(&structure->nodes[row]));
        ss_graph_append_text(line, sizeof(line), ": ");

        for (size_t column = 0; column < structure->node_count; ++column) {
            char weight_text[32];

            if (column > 0) {
                ss_graph_append_text(line, sizeof(line), " | ");
            }
            ss_graph_append_text(line, sizeof(line), ss_graph_node_name(&structure->nodes[column]));
            ss_graph_append_text(line, sizeof(line), "=");
            if (distance[row * structure->node_count + column] >= inf) {
                ss_graph_append_text(line, sizeof(line), "INF");
            } else {
                ss_graph_format_weight(distance[row * structure->node_count + column], weight_text, sizeof(weight_text));
                ss_graph_append_text(line, sizeof(line), weight_text);
            }
        }

        ss_graph_append_line(report, report_capacity, line);
    }

    free(distance);
    ss_error_clear(error);
    return 1;
}

static int ss_run_graph_prim(
    const SsStructure *structure,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    const double inf = DBL_MAX / 8.0;
    int *in_mst;
    int *parent;
    double *key;
    char line[256];
    int start_index;
    double total_cost = 0.0;
    size_t visited_count = 0;

    if (!ss_graph_validate_mst_requirements(structure, error)) {
        return 0;
    }
    if (structure->node_count == 0) {
        ss_str_copy(report, report_capacity, "El grafo esta vacio.");
        ss_error_clear(error);
        return 1;
    }

    start_index = 0;
    if (start_node_id != NULL && start_node_id[0] != '\0') {
        start_index = ss_find_node_index(structure, start_node_id);
        if (start_index < 0) {
            ss_error_set(error, SS_ERROR_NOT_FOUND, "El vertice de inicio para Prim no existe.");
            return 0;
        }
    }

    in_mst = (int *) calloc(structure->node_count, sizeof(*in_mst));
    parent = (int *) malloc(structure->node_count * sizeof(*parent));
    key = (double *) malloc(structure->node_count * sizeof(*key));
    if (in_mst == NULL || parent == NULL || key == NULL) {
        free(in_mst);
        free(parent);
        free(key);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para Prim.");
        return 0;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        parent[index] = -1;
        key[index] = inf;
    }
    key[(size_t) start_index] = 0.0;

    for (size_t count = 0; count < structure->node_count; ++count) {
        size_t current = structure->node_count;
        double best = inf;

        for (size_t index = 0; index < structure->node_count; ++index) {
            if (!in_mst[index] && key[index] < best) {
                best = key[index];
                current = index;
            }
        }
        if (current == structure->node_count) {
            break;
        }

        in_mst[current] = 1;
        visited_count++;
        total_cost += key[current];

        for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
            const SsEdge *edge = &structure->edges[edge_index];
            int neighbor = -1;
            double weight = ss_graph_edge_weight(edge);

            if (strcmp(edge->source_id, structure->nodes[current].id) == 0) {
                neighbor = ss_find_node_index(structure, edge->target_id);
            } else if (strcmp(edge->target_id, structure->nodes[current].id) == 0) {
                neighbor = ss_find_node_index(structure, edge->source_id);
            }

            if (neighbor >= 0 && !in_mst[(size_t) neighbor] && weight < key[(size_t) neighbor]) {
                key[(size_t) neighbor] = weight;
                parent[(size_t) neighbor] = (int) current;
            }
        }
    }

    if (visited_count != structure->node_count) {
        free(in_mst);
        free(parent);
        free(key);
        ss_error_set(error, SS_ERROR_VALIDATION, "Prim requiere un grafo conexo para construir el MST.");
        return 0;
    }

    report[0] = '\0';
    snprintf(line, sizeof(line), "Prim desde %s", ss_graph_node_name(&structure->nodes[(size_t) start_index]));
    ss_graph_append_line(report, report_capacity, line);
    for (size_t index = 0; index < structure->node_count; ++index) {
        char weight_text[32];

        if ((int) index == start_index) {
            continue;
        }
        ss_graph_format_weight(key[index], weight_text, sizeof(weight_text));
        snprintf(
            line,
            sizeof(line),
            "%s - %s = %s",
            ss_graph_node_name(&structure->nodes[(size_t) parent[index]]),
            ss_graph_node_name(&structure->nodes[index]),
            weight_text);
        ss_graph_append_line(report, report_capacity, line);
    }
    {
        char total_text[32];
        ss_graph_format_weight(total_cost, total_text, sizeof(total_text));
        snprintf(line, sizeof(line), "Costo total = %s", total_text);
        ss_graph_append_line(report, report_capacity, line);
    }

    free(in_mst);
    free(parent);
    free(key);
    ss_error_clear(error);
    return 1;
}

static int ss_run_graph_kruskal(
    const SsStructure *structure,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    SsWeightedEdgeRef *edges;
    SsWeightedEdgeRef *mst_edges;
    int *parent;
    int *rank;
    char line[256];
    size_t edge_count = 0;
    size_t mst_count = 0;
    double total_cost = 0.0;

    if (!ss_graph_validate_mst_requirements(structure, error)) {
        return 0;
    }
    if (structure->node_count == 0) {
        ss_str_copy(report, report_capacity, "El grafo esta vacio.");
        ss_error_clear(error);
        return 1;
    }

    edges = (SsWeightedEdgeRef *) malloc((structure->edge_count == 0 ? 1 : structure->edge_count) * sizeof(*edges));
    mst_edges = (SsWeightedEdgeRef *) malloc((structure->node_count == 0 ? 1 : structure->node_count) * sizeof(*mst_edges));
    parent = (int *) malloc((structure->node_count == 0 ? 1 : structure->node_count) * sizeof(*parent));
    rank = (int *) calloc(structure->node_count == 0 ? 1 : structure->node_count, sizeof(*rank));
    if (edges == NULL || mst_edges == NULL || parent == NULL || rank == NULL) {
        free(edges);
        free(mst_edges);
        free(parent);
        free(rank);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para Kruskal.");
        return 0;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        parent[index] = (int) index;
    }

    for (size_t index = 0; index < structure->edge_count; ++index) {
        const SsEdge *edge = &structure->edges[index];
        int source_index = ss_find_node_index(structure, edge->source_id);
        int target_index = ss_find_node_index(structure, edge->target_id);

        if (source_index < 0 || target_index < 0) {
            continue;
        }

        edges[edge_count].source_index = (size_t) source_index;
        edges[edge_count].target_index = (size_t) target_index;
        edges[edge_count].weight = ss_graph_edge_weight(edge);
        edge_count++;
    }

    qsort(edges, edge_count, sizeof(*edges), ss_graph_qsort_edges);
    for (size_t index = 0; index < edge_count; ++index) {
        if (ss_dsu_union(
                parent,
                rank,
                (int) edges[index].source_index,
                (int) edges[index].target_index)) {
            mst_edges[mst_count++] = edges[index];
            total_cost += edges[index].weight;
            if (mst_count == structure->node_count - 1) {
                break;
            }
        }
    }

    if (structure->node_count > 0 && mst_count != structure->node_count - 1) {
        free(edges);
        free(mst_edges);
        free(parent);
        free(rank);
        ss_error_set(error, SS_ERROR_VALIDATION, "Kruskal requiere un grafo conexo para construir el MST.");
        return 0;
    }

    report[0] = '\0';
    ss_graph_append_line(report, report_capacity, "Kruskal");
    for (size_t index = 0; index < mst_count; ++index) {
        char weight_text[32];

        ss_graph_format_weight(mst_edges[index].weight, weight_text, sizeof(weight_text));
        snprintf(
            line,
            sizeof(line),
            "%s - %s = %s",
            ss_graph_node_name(&structure->nodes[mst_edges[index].source_index]),
            ss_graph_node_name(&structure->nodes[mst_edges[index].target_index]),
            weight_text);
        ss_graph_append_line(report, report_capacity, line);
    }
    {
        char total_text[32];
        ss_graph_format_weight(total_cost, total_text, sizeof(total_text));
        snprintf(line, sizeof(line), "Costo total = %s", total_text);
        ss_graph_append_line(report, report_capacity, line);
    }

    free(edges);
    free(mst_edges);
    free(parent);
    free(rank);
    ss_error_clear(error);
    return 1;
}

int ss_run_graph_advanced_analysis(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error)
{
    switch (kind) {
        case SS_ANALYSIS_GRAPH_FLOYD_WARSHALL:
            return ss_run_graph_floyd_warshall(structure, report, report_capacity, error);
        case SS_ANALYSIS_GRAPH_PRIM:
            return ss_run_graph_prim(structure, start_node_id, report, report_capacity, error);
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            return ss_run_graph_kruskal(structure, report, report_capacity, error);
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "Analisis avanzado de grafo no soportado.");
            return 0;
    }
}
