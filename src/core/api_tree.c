/*
 * StructStudio C
 * --------------
 * Tree, BST, AVL and heap helpers.
 *
 * The algorithms here stay close to the domain model and are reused by several
 * command files. Keeping them isolated avoids mixing traversal logic with UI
 * command routing.
 */

#include "api_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ss_compare_ints(const void *left, const void *right)
{
    const int *a = (const int *) left;
    const int *b = (const int *) right;
    return *a - *b;
}

int ss_node_int_value(const SsNode *node, int *out_value)
{
    return ss_parse_int(node->value, out_value);
}

static void ss_clear_tree_links(SsStructure *structure)
{
    size_t index;

    for (index = 0; index < structure->node_count; ++index) {
        structure->nodes[index].data.ref_a[0] = '\0';
        structure->nodes[index].data.ref_b[0] = '\0';
        structure->nodes[index].data.ref_c[0] = '\0';
    }
    structure->root_id[0] = '\0';
    structure->edge_count = 0;
}

void ss_rebuild_tree_edges(SsStructure *structure)
{
    size_t index;

    structure->edge_count = 0;
    for (index = 0; index < structure->node_count; ++index) {
        SsNode *node = &structure->nodes[index];
        if (node->data.ref_a[0] != '\0') {
            ss_append_edge(structure, node->id, node->data.ref_a, "left", 1, 0, 0.0, NULL);
        }
        if (node->data.ref_b[0] != '\0') {
            ss_append_edge(structure, node->id, node->data.ref_b, "right", 1, 0, 0.0, NULL);
        }
    }
}

void ss_rebuild_heap_tree(SsStructure *structure)
{
    size_t index;

    ss_clear_tree_links(structure);
    if (structure->node_count == 0) {
        return;
    }

    ss_str_copy(structure->root_id, sizeof(structure->root_id), structure->nodes[0].id);
    for (index = 0; index < structure->node_count; ++index) {
        size_t left = index * 2 + 1;
        size_t right = left + 1;
        structure->nodes[index].data.index_hint = (int) index;
        if (left < structure->node_count) {
            ss_str_copy(structure->nodes[index].data.ref_a, sizeof(structure->nodes[index].data.ref_a), structure->nodes[left].id);
            ss_str_copy(structure->nodes[left].data.ref_c, sizeof(structure->nodes[left].data.ref_c), structure->nodes[index].id);
        }
        if (right < structure->node_count) {
            ss_str_copy(structure->nodes[index].data.ref_b, sizeof(structure->nodes[index].data.ref_b), structure->nodes[right].id);
            ss_str_copy(structure->nodes[right].data.ref_c, sizeof(structure->nodes[right].data.ref_c), structure->nodes[index].id);
        }
    }
    ss_rebuild_tree_edges(structure);
}

void ss_heap_sift_up(SsStructure *structure, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        int current_value;
        int parent_value;

        if (!ss_node_int_value(&structure->nodes[index], &current_value) || !ss_node_int_value(&structure->nodes[parent], &parent_value)) {
            return;
        }
        if (current_value <= parent_value) {
            return;
        }

        {
            SsNode temp = structure->nodes[parent];
            structure->nodes[parent] = structure->nodes[index];
            structure->nodes[index] = temp;
        }
        index = parent;
    }
}

void ss_heap_sift_down(SsStructure *structure, size_t index)
{
    for (;;) {
        size_t left = index * 2 + 1;
        size_t right = left + 1;
        size_t largest = index;
        int largest_value;
        int left_value;
        int right_value;

        if (!ss_node_int_value(&structure->nodes[largest], &largest_value)) {
            return;
        }
        if (left < structure->node_count && ss_node_int_value(&structure->nodes[left], &left_value) && left_value > largest_value) {
            largest = left;
            largest_value = left_value;
        }
        if (right < structure->node_count && ss_node_int_value(&structure->nodes[right], &right_value) && right_value > largest_value) {
            largest = right;
        }
        if (largest == index) {
            return;
        }

        {
            SsNode temp = structure->nodes[index];
            structure->nodes[index] = structure->nodes[largest];
            structure->nodes[largest] = temp;
        }
        index = largest;
    }
}

SsNode *ss_tree_node_by_child(SsStructure *structure, const char *child_id)
{
    size_t index;

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].data.ref_a, child_id) == 0 || strcmp(structure->nodes[index].data.ref_b, child_id) == 0) {
            return &structure->nodes[index];
        }
    }
    return NULL;
}

static void ss_collect_subtree_ids(const SsStructure *structure, const char *node_id, char ids[][SS_ID_CAPACITY], size_t *count)
{
    const SsNode *node;

    node = ss_structure_find_node_const(structure, node_id);
    if (node == NULL || *count >= SS_VALIDATION_CAPACITY) {
        return;
    }

    ss_str_copy(ids[*count], SS_ID_CAPACITY, node->id);
    *count += 1;
    if (node->data.ref_a[0] != '\0') {
        ss_collect_subtree_ids(structure, node->data.ref_a, ids, count);
    }
    if (node->data.ref_b[0] != '\0') {
        ss_collect_subtree_ids(structure, node->data.ref_b, ids, count);
    }
}

void ss_delete_subtree(SsStructure *structure, const char *node_id)
{
    char ids[SS_VALIDATION_CAPACITY][SS_ID_CAPACITY];
    size_t count = 0;
    size_t index;

    ss_collect_subtree_ids(structure, node_id, ids, &count);
    for (index = 0; index < count; ++index) {
        int node_index = ss_find_node_index(structure, ids[index]);
        if (node_index >= 0) {
            ss_remove_node_at(structure, (size_t) node_index);
        }
    }
}

int ss_collect_int_values(const SsStructure *structure, int *values, size_t capacity)
{
    size_t index;

    if (structure->node_count > capacity) {
        return 0;
    }
    for (index = 0; index < structure->node_count; ++index) {
        if (!ss_node_int_value(&structure->nodes[index], &values[index])) {
            return 0;
        }
    }
    return 1;
}

int ss_bst_insert_value(SsStructure *structure, int value, SsError *error)
{
    char value_buffer[32];
    SsNode *current;
    SsNode *parent = NULL;
    int current_value;
    int go_left = 0;

    snprintf(value_buffer, sizeof(value_buffer), "%d", value);
    if (structure->root_id[0] == '\0') {
        SsNode *root = ss_append_node_with_id(structure, "", ss_node_kind_for_variant(structure->variant), value_buffer, value_buffer, error);
        if (root == NULL) {
            return 0;
        }
        ss_str_copy(structure->root_id, sizeof(structure->root_id), root->id);
        return 1;
    }

    current = ss_structure_find_node(structure, structure->root_id);
    while (current != NULL) {
        if (!ss_node_int_value(current, &current_value)) {
            ss_error_set(error, SS_ERROR_VALIDATION, "El arbol contiene valores no numericos.");
            return 0;
        }
        if (value == current_value) {
            ss_error_set(error, SS_ERROR_DUPLICATE, "No se permiten duplicados en el arbol.");
            return 0;
        }
        parent = current;
        if (value < current_value) {
            go_left = 1;
            current = current->data.ref_a[0] != '\0' ? ss_structure_find_node(structure, current->data.ref_a) : NULL;
        } else {
            go_left = 0;
            current = current->data.ref_b[0] != '\0' ? ss_structure_find_node(structure, current->data.ref_b) : NULL;
        }
    }

    {
        SsNode *node = ss_append_node_with_id(structure, "", ss_node_kind_for_variant(structure->variant), value_buffer, value_buffer, error);
        if (node == NULL) {
            return 0;
        }
        ss_str_copy(node->data.ref_c, sizeof(node->data.ref_c), parent->id);
        if (go_left) {
            ss_str_copy(parent->data.ref_a, sizeof(parent->data.ref_a), node->id);
        } else {
            ss_str_copy(parent->data.ref_b, sizeof(parent->data.ref_b), node->id);
        }
    }

    ss_rebuild_tree_edges(structure);
    return 1;
}

static int ss_avl_build_balanced(
    SsStructure *structure,
    const int *values,
    int start,
    int end,
    const char *parent_id,
    char *out_root_id,
    SsError *error)
{
    int mid;
    char value_buffer[32];
    SsNode *node;

    if (start > end) {
        out_root_id[0] = '\0';
        return 1;
    }

    mid = (start + end) / 2;
    snprintf(value_buffer, sizeof(value_buffer), "%d", values[mid]);
    node = ss_append_node_with_id(structure, "", ss_node_kind_for_variant(structure->variant), value_buffer, value_buffer, error);
    if (node == NULL) {
        return 0;
    }
    if (parent_id != NULL) {
        ss_str_copy(node->data.ref_c, sizeof(node->data.ref_c), parent_id);
    }
    ss_str_copy(out_root_id, SS_ID_CAPACITY, node->id);

    if (!ss_avl_build_balanced(structure, values, start, mid - 1, node->id, node->data.ref_a, error)) {
        return 0;
    }
    if (!ss_avl_build_balanced(structure, values, mid + 1, end, node->id, node->data.ref_b, error)) {
        return 0;
    }
    return 1;
}

int ss_rebuild_numeric_tree(SsStructure *structure, int *values, size_t count, int balanced, SsError *error)
{
    size_t index;
    char root_id[SS_ID_CAPACITY] = "";

    ss_structure_clear(structure);
    if (count == 0) {
        return 1;
    }

    if (balanced) {
        /* AVL rebuilding is implemented as "sort + rebuild balanced" to keep
         * the code approachable for study and easy to validate in tests. */
        qsort(values, count, sizeof(*values), ss_compare_ints);
        if (!ss_avl_build_balanced(structure, values, 0, (int) count - 1, NULL, root_id, error)) {
            return 0;
        }
        ss_str_copy(structure->root_id, sizeof(structure->root_id), root_id);
        ss_rebuild_tree_edges(structure);
        return 1;
    }

    for (index = 0; index < count; ++index) {
        if (!ss_bst_insert_value(structure, values[index], error)) {
            return 0;
        }
    }
    return 1;
}
