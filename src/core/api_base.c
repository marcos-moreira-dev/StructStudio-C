/*
 * StructStudio C
 * --------------
 * Shared infrastructure for structure operations.
 *
 * This file owns low-level node/edge creation, removal and rebuilding helpers.
 * Higher-level commands live in dedicated files and depend on these primitives.
 */

#include "api_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double SS_NODE_WIDTH = 86.0;
const double SS_NODE_HEIGHT = 44.0;
const double SS_ROUND_NODE_SIZE = 74.0;

/* These helpers keep string assembly explicit and bounded. In C this matters:
 * the rest of the project can safely compose labels/messages without repeating
 * the same manual length checks everywhere. */
static void ss_append_limited_text(char *buffer, size_t capacity, const char *text)
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

const char *ss_node_kind_for_variant(SsVariant variant)
{
    /* kind is a semantic rendering hint. It is not the same thing as variant:
     * several variants can share one kind because they draw similarly. */
    switch (variant) {
        case SS_VARIANT_VECTOR:
            return "vector_cell";
        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            return "list_node";
        case SS_VARIANT_STACK:
            return "stack_node";
        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
        case SS_VARIANT_PRIORITY_QUEUE:
            return "queue_node";
        case SS_VARIANT_BINARY_TREE:
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
            return "tree_node";
        case SS_VARIANT_HEAP:
            return "heap_node";
        case SS_VARIANT_SET:
            return "set_node";
        case SS_VARIANT_MAP:
            return "map_entry";
        default:
            return "graph_vertex";
    }
}

int ss_variant_prefers_round_nodes(SsVariant variant)
{
    /* Visual preferences stay near the shared model helpers because several
     * layers need the same answer: layout, render and insertion defaults. */
    switch (variant) {
        case SS_VARIANT_BINARY_TREE:
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
        case SS_VARIANT_HEAP:
        case SS_VARIANT_SET:
        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            return 1;
        default:
            return 0;
    }
}

int ss_find_node_index(const SsStructure *structure, const char *node_id)
{
    size_t index;

    if (structure == NULL || node_id == NULL) {
        return -1;
    }

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].id, node_id) == 0) {
            return (int) index;
        }
    }
    return -1;
}

const SsNode *ss_structure_find_node_by_token_const(const SsStructure *structure, const char *token, SsError *error)
{
    const SsNode *match = NULL;
    size_t index;

    /* Token resolution is intentionally user-friendly: a field can reference a
     * node by technical ID, visible label or visible value. Ambiguity is
     * reported as an error instead of silently choosing the wrong node. */
    if (error != NULL) {
        ss_error_clear(error);
    }
    if (structure == NULL || token == NULL || token[0] == '\0') {
        return NULL;
    }

    match = ss_structure_find_node_const(structure, token);
    if (match != NULL) {
        return match;
    }

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].label, token) == 0) {
            if (match != NULL) {
                ss_error_set(error, SS_ERROR_DUPLICATE, "La etiqueta del nodo es ambigua.");
                return NULL;
            }
            match = &structure->nodes[index];
        }
    }
    if (match != NULL) {
        return match;
    }

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].value, token) == 0) {
            if (match != NULL) {
                ss_error_set(error, SS_ERROR_DUPLICATE, "El valor del nodo es ambiguo.");
                return NULL;
            }
            match = &structure->nodes[index];
        }
    }

    return match;
}

int ss_find_edge_index(const SsStructure *structure, const char *edge_id)
{
    size_t index;

    if (structure == NULL || edge_id == NULL) {
        return -1;
    }

    for (index = 0; index < structure->edge_count; ++index) {
        if (strcmp(structure->edges[index].id, edge_id) == 0) {
            return (int) index;
        }
    }
    return -1;
}

int ss_reserve_node(SsStructure *structure, size_t required, SsError *error)
{
    /* Reserve helpers centralize allocation failures so command code can stay
     * focused on semantics instead of raw realloc bookkeeping. */
    if (!ss_array_reserve((void **) &structure->nodes, sizeof(*structure->nodes), &structure->node_capacity, required)) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para nodos.");
        return 0;
    }
    return 1;
}

static int ss_reserve_edge(SsStructure *structure, size_t required, SsError *error)
{
    if (!ss_array_reserve((void **) &structure->edges, sizeof(*structure->edges), &structure->edge_capacity, required)) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para aristas.");
        return 0;
    }
    return 1;
}

void ss_assign_node_defaults(SsNode *node, const char *kind, const char *value)
{
    /* Every new node starts from one canonical initialization path. That keeps
     * freshly inserted entities predictable before a variant customizes them. */
    memset(node, 0, sizeof(*node));
    ss_generate_id("node", node->id, sizeof(node->id));
    ss_str_copy(node->kind, sizeof(node->kind), kind);
    ss_str_copy(node->label, sizeof(node->label), value != NULL && value[0] != '\0' ? value : node->id);
    ss_str_copy(node->value, sizeof(node->value), value != NULL ? value : "");
    ss_str_copy(node->value_type, sizeof(node->value_type), "string");
    node->visual.width = SS_NODE_WIDTH;
    node->visual.height = SS_NODE_HEIGHT;
}

void ss_apply_round_node_visual(SsNode *node)
{
    if (node == NULL) {
        return;
    }

    node->visual.width = SS_ROUND_NODE_SIZE;
    node->visual.height = SS_ROUND_NODE_SIZE;
}

void ss_format_priority_label(SsNode *node)
{
    char priority_buffer[24];

    /* Priority queues need the priority visible in the label so the student
     * can compare queue order with the stored payload directly on the canvas. */
    if (node == NULL) {
        return;
    }

    node->label[0] = '\0';
    ss_append_limited_text(node->label, sizeof(node->label), node->value);
    ss_append_limited_text(node->label, sizeof(node->label), " | p=");
    snprintf(priority_buffer, sizeof(priority_buffer), "%d", node->data.priority);
    ss_append_limited_text(node->label, sizeof(node->label), priority_buffer);
}

void ss_format_map_label(SsNode *node)
{
    if (node == NULL) {
        return;
    }

    node->label[0] = '\0';
    ss_append_limited_text(node->label, sizeof(node->label), node->data.key);
    ss_append_limited_text(node->label, sizeof(node->label), " -> ");
    ss_append_limited_text(node->label, sizeof(node->label), node->value);
}

SsNode *ss_insert_node_at(SsStructure *structure, size_t index, const char *value, SsError *error)
{
    SsNode node;

    if (!ss_reserve_node(structure, structure->node_count + 1, error)) {
        return NULL;
    }
    if (index > structure->node_count) {
        index = structure->node_count;
    }

    ss_assign_node_defaults(&node, ss_node_kind_for_variant(structure->variant), value);
    if (ss_variant_prefers_round_nodes(structure->variant)) {
        ss_apply_round_node_visual(&node);
    }
    if (index < structure->node_count) {
        memmove(&structure->nodes[index + 1], &structure->nodes[index], (structure->node_count - index) * sizeof(*structure->nodes));
    }
    structure->nodes[index] = node;
    structure->node_count += 1;
    return &structure->nodes[index];
}

SsNode *ss_append_node_with_id(
    SsStructure *structure,
    const char *id,
    const char *kind,
    const char *label,
    const char *value,
    SsError *error)
{
    SsNode *node;

    if (!ss_reserve_node(structure, structure->node_count + 1, error)) {
        return NULL;
    }

    node = &structure->nodes[structure->node_count];
    ss_assign_node_defaults(node, kind, value);
    if (ss_variant_prefers_round_nodes(structure->variant)) {
        ss_apply_round_node_visual(node);
    }
    ss_str_copy(node->id, sizeof(node->id), id);
    if (node->id[0] == '\0') {
        ss_generate_id("node", node->id, sizeof(node->id));
    }
    ss_str_copy(node->label, sizeof(node->label), label);
    ss_str_copy(node->value, sizeof(node->value), value);
    structure->node_count += 1;
    return node;
}

SsEdge *ss_append_edge(
    SsStructure *structure,
    const char *source_id,
    const char *target_id,
    const char *relation_kind,
    int directed,
    int has_weight,
    double weight,
    SsError *error)
{
    SsEdge *edge;

    if (!ss_reserve_edge(structure, structure->edge_count + 1, error)) {
        return NULL;
    }

    edge = &structure->edges[structure->edge_count];
    memset(edge, 0, sizeof(*edge));
    ss_generate_id("edge", edge->id, sizeof(edge->id));
    ss_str_copy(edge->source_id, sizeof(edge->source_id), source_id);
    ss_str_copy(edge->target_id, sizeof(edge->target_id), target_id);
    ss_str_copy(edge->relation_kind, sizeof(edge->relation_kind), relation_kind);
    edge->is_directed = directed;
    edge->has_weight = has_weight;
    edge->weight = weight;
    edge->visual.show_arrow = directed;
    edge->visual.show_weight = has_weight;
    structure->edge_count += 1;
    return edge;
}

void ss_remove_edge_at(SsStructure *structure, size_t index)
{
    if (index >= structure->edge_count) {
        return;
    }
    if (index + 1 < structure->edge_count) {
        memmove(&structure->edges[index], &structure->edges[index + 1], (structure->edge_count - index - 1) * sizeof(*structure->edges));
    }
    structure->edge_count -= 1;
}

void ss_remove_node_at(SsStructure *structure, size_t index)
{
    size_t edge_index = 0;
    const char *removed_id;

    if (index >= structure->node_count) {
        return;
    }

    removed_id = structure->nodes[index].id;
    while (edge_index < structure->edge_count) {
        SsEdge *edge = &structure->edges[edge_index];
        if (strcmp(edge->source_id, removed_id) == 0 || strcmp(edge->target_id, removed_id) == 0) {
            ss_remove_edge_at(structure, edge_index);
            continue;
        }
        edge_index += 1;
    }

    if (index + 1 < structure->node_count) {
        memmove(&structure->nodes[index], &structure->nodes[index + 1], (structure->node_count - index - 1) * sizeof(*structure->nodes));
    }
    structure->node_count -= 1;
}

static void ss_rebuild_list_edges(SsStructure *structure)
{
    size_t index;
    int use_prev =
        structure->variant == SS_VARIANT_DOUBLY_LINKED_LIST ||
        structure->variant == SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST;
    int is_circular = structure->config.is_circular;

    structure->edge_count = 0;

    for (index = 0; index < structure->node_count; ++index) {
        structure->nodes[index].data.index_hint = (int) index;
        snprintf(structure->nodes[index].label, sizeof(structure->nodes[index].label), "N%zu", index + 1);
        if (index + 1 < structure->node_count) {
            ss_append_edge(structure, structure->nodes[index].id, structure->nodes[index + 1].id, "next", 1, 0, 0.0, NULL);
            if (use_prev) {
                ss_append_edge(structure, structure->nodes[index + 1].id, structure->nodes[index].id, "prev", 1, 0, 0.0, NULL);
            }
        }
    }

    if (is_circular && structure->node_count > 1) {
        ss_append_edge(structure, structure->nodes[structure->node_count - 1].id, structure->nodes[0].id, "next", 1, 0, 0.0, NULL);
        if (use_prev) {
            ss_append_edge(structure, structure->nodes[0].id, structure->nodes[structure->node_count - 1].id, "prev", 1, 0, 0.0, NULL);
        }
    }
}

static void ss_rebuild_queue_edges(SsStructure *structure)
{
    size_t index;

    structure->edge_count = 0;
    for (index = 0; index < structure->node_count; ++index) {
        structure->nodes[index].data.index_hint = (int) index;
        if (index + 1 < structure->node_count) {
            ss_append_edge(structure, structure->nodes[index].id, structure->nodes[index + 1].id, "next", 1, 0, 0.0, NULL);
        }
    }
    if (structure->variant == SS_VARIANT_CIRCULAR_QUEUE && structure->node_count > 1) {
        ss_append_edge(structure, structure->nodes[structure->node_count - 1].id, structure->nodes[0].id, "next", 1, 0, 0.0, NULL);
    }
}

static void ss_rebuild_stack_edges(SsStructure *structure)
{
    size_t index;

    structure->edge_count = 0;
    for (index = 1; index < structure->node_count; ++index) {
        ss_append_edge(structure, structure->nodes[index].id, structure->nodes[index - 1].id, "below", 1, 0, 0.0, NULL);
    }
}

static int ss_compare_priority_desc(const void *left, const void *right)
{
    const SsNode *a = (const SsNode *) left;
    const SsNode *b = (const SsNode *) right;
    return b->data.priority - a->data.priority;
}

void ss_rebuild_priority_queue(SsStructure *structure)
{
    size_t index;

    qsort(structure->nodes, structure->node_count, sizeof(*structure->nodes), ss_compare_priority_desc);
    structure->edge_count = 0;
    for (index = 0; index < structure->node_count; ++index) {
        structure->nodes[index].data.index_hint = (int) index;
        /* Labels are refreshed here because priority is part of the user-facing
         * identity of each queue entry in the current UI. */
        ss_format_priority_label(&structure->nodes[index]);
    }
}

int ss_value_exists(const SsStructure *structure, const char *value)
{
    size_t index;

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].value, value) == 0) {
            return 1;
        }
    }
    return 0;
}

int ss_map_key_exists(const SsStructure *structure, const char *key, const char *ignore_node_id)
{
    size_t index;

    for (index = 0; index < structure->node_count; ++index) {
        const SsNode *node = &structure->nodes[index];
        if (ignore_node_id != NULL && strcmp(node->id, ignore_node_id) == 0) {
            continue;
        }
        if (strcmp(node->data.key, key) == 0) {
            return 1;
        }
    }
    return 0;
}

void ss_format_message(char *buffer, size_t capacity, const char *fmt, const char *label)
{
    if (buffer == NULL || capacity == 0) {
        return;
    }
    snprintf(buffer, capacity, fmt, label);
}

int ss_format_numeric_input(const char *primary, int numeric_value, int *out_value, SsError *error)
{
    if (primary != NULL && ss_parse_int(primary, out_value)) {
        return 1;
    }
    *out_value = numeric_value;
    if (primary == NULL && numeric_value == 0) {
        ss_error_set(error, SS_ERROR_VALIDATION, "Se requiere un valor numerico.");
        return 0;
    }
    return 1;
}

void ss_rebuild_structure_internals(SsStructure *structure)
{
    switch (structure->variant) {
        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            ss_rebuild_list_edges(structure);
            break;
        case SS_VARIANT_STACK:
            ss_rebuild_stack_edges(structure);
            break;
        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
            ss_rebuild_queue_edges(structure);
            break;
        case SS_VARIANT_PRIORITY_QUEUE:
            ss_rebuild_priority_queue(structure);
            break;
        case SS_VARIANT_HEAP:
            ss_rebuild_heap_tree(structure);
            break;
        case SS_VARIANT_BINARY_TREE:
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
            ss_rebuild_tree_edges(structure);
            break;
        default:
            break;
    }
    structure->dirty_layout = 1;
}
