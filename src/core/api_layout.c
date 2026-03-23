/*
 * StructStudio C
 * --------------
 * Automatic layout strategies per structure family.
 *
 * The goal is not to be mathematically optimal, but deterministic and easy to
 * reason about for teaching and for PNG export reproducibility.
 */

#include "api_internal.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct SsTreeLayoutCursor {
    double next_x;
} SsTreeLayoutCursor;

static void ss_layout_graph_tree(SsStructure *structure, double canvas_width)
{
    int *depths;
    size_t *queue;
    size_t *level_counts;
    size_t *level_used;
    size_t head = 0;
    size_t tail = 0;
    size_t reachable = 0;
    int root_index = 0;
    int max_depth = 0;

    if (structure->node_count == 0) {
        return;
    }

    if (structure->root_id[0] != '\0') {
        int found = ss_find_node_index(structure, structure->root_id);
        if (found >= 0) {
            root_index = found;
        }
    }

    depths = (int *) malloc(structure->node_count * sizeof(*depths));
    queue = (size_t *) malloc(structure->node_count * sizeof(*queue));
    if (depths == NULL || queue == NULL) {
        free(depths);
        free(queue);
        return;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        depths[index] = -1;
    }

    depths[(size_t) root_index] = 0;
    queue[tail++] = (size_t) root_index;
    while (head < tail) {
        size_t current = queue[head++];
        const char *current_id = structure->nodes[current].id;

        reachable = head;
        if (depths[current] > max_depth) {
            max_depth = depths[current];
        }

        for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
            const SsEdge *edge = &structure->edges[edge_index];
            int child_index;

            if (strcmp(edge->source_id, current_id) != 0) {
                continue;
            }

            child_index = ss_find_node_index(structure, edge->target_id);
            if (child_index < 0 || depths[(size_t) child_index] >= 0) {
                continue;
            }

            depths[(size_t) child_index] = depths[current] + 1;
            queue[tail++] = (size_t) child_index;
        }
    }

    level_counts = (size_t *) calloc((size_t) max_depth + 2, sizeof(*level_counts));
    level_used = (size_t *) calloc((size_t) max_depth + 2, sizeof(*level_used));
    if (level_counts == NULL || level_used == NULL) {
        free(depths);
        free(queue);
        free(level_counts);
        free(level_used);
        return;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        if (depths[index] < 0) {
            depths[index] = max_depth + 1;
        }
        level_counts[(size_t) depths[index]] += 1;
    }

    for (size_t order = 0; order < structure->node_count; ++order) {
        size_t node_index = order < reachable ? queue[order] : order;
        size_t level;
        size_t position;
        double total_width;
        double start_x;

        if (order >= reachable && depths[node_index] <= max_depth) {
            continue;
        }

        structure->nodes[node_index].visual.width = SS_ROUND_NODE_SIZE;
        structure->nodes[node_index].visual.height = SS_ROUND_NODE_SIZE;
        level = (size_t) depths[node_index];
        position = level_used[level]++;
        total_width = level_counts[level] > 0 ? (double) (level_counts[level] - 1) * 128.0 : 0.0;
        start_x = canvas_width > total_width + 160.0
            ? (canvas_width - total_width) / 2.0
            : 70.0;
        structure->nodes[node_index].visual.x = start_x + (double) position * 128.0;
        structure->nodes[node_index].visual.y = 92.0 + (double) level * 108.0;
    }

    free(depths);
    free(queue);
    free(level_counts);
    free(level_used);
}

static void ss_layout_linear(SsStructure *structure, double start_x, double start_y, double gap_x)
{
    for (size_t index = 0; index < structure->node_count; ++index) {
        SsNode *node = &structure->nodes[index];
        node->visual.width = structure->variant == SS_VARIANT_MAP ? 150.0 : SS_NODE_WIDTH;
        node->visual.height = SS_NODE_HEIGHT;
        node->visual.x = start_x + (double) index * gap_x;
        node->visual.y = start_y;
    }
}

static void ss_layout_wrapped(
    SsStructure *structure,
    double start_x,
    double start_y,
    double gap_x,
    double gap_y,
    size_t columns,
    double node_width,
    double node_height)
{
    for (size_t index = 0; index < structure->node_count; ++index) {
        size_t row = index / columns;
        size_t column = index % columns;
        SsNode *node = &structure->nodes[index];

        node->visual.width = node_width;
        node->visual.height = node_height;
        node->visual.x = start_x + (double) column * gap_x;
        node->visual.y = start_y + (double) row * gap_y;
    }
}

static void ss_layout_tree_recursive(SsStructure *structure, const char *node_id, int depth, SsTreeLayoutCursor *cursor)
{
    SsNode *node = ss_structure_find_node(structure, node_id);

    if (node == NULL) {
        return;
    }

    if (node->data.ref_a[0] != '\0') {
        ss_layout_tree_recursive(structure, node->data.ref_a, depth + 1, cursor);
    }

    ss_apply_round_node_visual(node);
    node->visual.x = cursor->next_x;
    node->visual.y = 70.0 + depth * 96.0;
    cursor->next_x += 112.0;

    if (node->data.ref_b[0] != '\0') {
        ss_layout_tree_recursive(structure, node->data.ref_b, depth + 1, cursor);
    }
}

void ss_structure_auto_layout(SsStructure *structure, double canvas_width)
{
    SsTreeLayoutCursor cursor;

    if (structure == NULL) {
        return;
    }

    switch (structure->family) {
        case SS_FAMILY_VECTOR:
            ss_layout_linear(structure, 60.0, 120.0, 96.0);
            break;
        case SS_FAMILY_LIST:
        case SS_FAMILY_QUEUE:
            ss_layout_linear(structure, 70.0, 140.0, 128.0);
            break;
        case SS_FAMILY_STACK:
            for (size_t index = 0; index < structure->node_count; ++index) {
                structure->nodes[index].visual.width = SS_NODE_WIDTH;
                structure->nodes[index].visual.height = SS_NODE_HEIGHT;
                structure->nodes[index].visual.x = 160.0;
                structure->nodes[index].visual.y = 340.0 - (double) index * 64.0;
            }
            break;
        case SS_FAMILY_SET:
            ss_layout_wrapped(
                structure,
                70.0,
                120.0,
                120.0,
                88.0,
                4,
                SS_ROUND_NODE_SIZE,
                SS_ROUND_NODE_SIZE);
            break;
        case SS_FAMILY_MAP:
            ss_layout_wrapped(
                structure,
                70.0,
                120.0,
                190.0,
                80.0,
                3,
                150.0,
                SS_NODE_HEIGHT);
            break;
        case SS_FAMILY_TREE:
        case SS_FAMILY_HEAP:
            cursor.next_x = 70.0;
            if (structure->root_id[0] != '\0') {
                ss_layout_tree_recursive(structure, structure->root_id, 0, &cursor);
            }
            break;
        case SS_FAMILY_GRAPH:
            if (strncmp(structure->visual_state.layout_mode, "tree_", 5) == 0) {
                ss_layout_graph_tree(structure, canvas_width);
            } else {
                for (size_t index = 0; index < structure->node_count; ++index) {
                    double angle = ((double) index / (double) (structure->node_count == 0 ? 1 : structure->node_count)) * 6.28318530718;
                    double center_x = canvas_width > 320.0 ? canvas_width / 2.0 : 320.0;
                    double center_y = 240.0;
                    double radius = structure->node_count > 2 ? 140.0 : 90.0;
                    structure->nodes[index].visual.width = 74.0;
                    structure->nodes[index].visual.height = 74.0;
                    structure->nodes[index].visual.x = center_x + cos(angle) * radius;
                    structure->nodes[index].visual.y = center_y + sin(angle) * radius;
                }
            }
            break;
        default:
            break;
    }

    structure->dirty_layout = 0;
}
