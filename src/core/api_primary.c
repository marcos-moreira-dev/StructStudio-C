/*
 * StructStudio C
 * --------------
 * Primary operations for each supported structure variant.
 *
 * This file keeps only the "main action" semantics exposed by the left panel.
 * Shared helpers, tree rebuilding, layout and validation stay in their own
 * modules so this translation unit remains focused on domain commands.
 */

#include "api_internal.h"

#include <stdio.h>
#include <string.h>

int ss_structure_apply_primary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error,
    char *out_message,
    size_t out_message_capacity)
{
    SsNode *node;
    int value;

    /* "Primary" means the main action shown in the left panel for the active
     * variant. The same generic button maps to different semantics here. */
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No hay estructura activa.");
        return 0;
    }

    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            /* Vectors are index-addressed. The command may grow the logical
             * array up to the requested position before storing the new value. */
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            if (value < 0) {
                ss_error_set(error, SS_ERROR_VALIDATION, "El indice debe ser positivo.");
                return 0;
            }
            if (!ss_reserve_node(structure, (size_t) value + 1, error)) {
                return 0;
            }
            while (structure->node_count <= (size_t) value) {
                char empty[16];
                SsNode *cell = ss_insert_node_at(structure, structure->node_count, "", error);
                if (cell == NULL) {
                    return 0;
                }
                snprintf(empty, sizeof(empty), "[%zu]", structure->node_count - 1);
                ss_str_copy(cell->label, sizeof(cell->label), empty);
                cell->data.index_hint = (int) (structure->node_count - 1);
            }
            ss_str_copy(
                structure->nodes[value].value,
                sizeof(structure->nodes[value].value),
                secondary != NULL && secondary[0] != '\0' ? secondary : primary);
            snprintf(structure->nodes[value].label, sizeof(structure->nodes[value].label), "[%d]", value);
            ss_format_message(out_message, out_message_capacity, "Celda %s actualizada.", structure->nodes[value].label);
            structure->dirty_layout = 1;
            return 1;

        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            /* For linked-list variants, primary means insert at the head. */
            node = ss_insert_node_at(structure, 0, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_rebuild_structure_internals(structure);
            ss_format_message(out_message, out_message_capacity, "Nodo %s agregado al inicio.", node->id);
            return 1;

        case SS_VARIANT_STACK:
            /* Stack primary action is push at the visual/semantic top. */
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_rebuild_structure_internals(structure);
            ss_format_message(out_message, out_message_capacity, "Push sobre %s.", node->value);
            return 1;

        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_rebuild_structure_internals(structure);
            ss_format_message(out_message, out_message_capacity, "Enqueue de %s.", node->value);
            return 1;

        case SS_VARIANT_PRIORITY_QUEUE:
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            node->data.priority = numeric_value;
            ss_rebuild_structure_internals(structure);
            ss_format_message(out_message, out_message_capacity, "Elemento %s con prioridad registrada.", node->value);
            return 1;

        case SS_VARIANT_BINARY_TREE:
            /* A plain binary tree only reserves primary for creating the root.
             * Child insertion is handled by secondary/tertiary actions. */
            if (structure->root_id[0] != '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El arbol ya tiene raiz.");
                return 0;
            }
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_str_copy(structure->root_id, sizeof(structure->root_id), node->id);
            ss_rebuild_structure_internals(structure);
            ss_format_message(out_message, out_message_capacity, "Raiz %s creada.", node->id);
            return 1;

        case SS_VARIANT_BST:
            /* BST/AVL/Heap rely on numeric semantics, so the generic text input
             * is normalized into an integer before the core algorithm runs. */
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            if (!ss_bst_insert_value(structure, value, error)) {
                return 0;
            }
            structure->dirty_layout = 1;
            snprintf(out_message, out_message_capacity, "Valor %d insertado en BST.", value);
            return 1;

        case SS_VARIANT_AVL:
        {
            int values[256];
            size_t count;

            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            if (structure->node_count >= 256 || !ss_collect_int_values(structure, values, 256)) {
                ss_error_set(error, SS_ERROR_VALIDATION, "El AVL solo admite valores enteros en esta V1.");
                return 0;
            }
            /* The AVL implementation rebuilds the balanced tree from the value
             * set. That is not the textbook incremental algorithm, but it keeps
             * this V1 smaller and easier to study. */
            for (count = 0; count < structure->node_count; ++count) {
                if (values[count] == value) {
                    ss_error_set(error, SS_ERROR_DUPLICATE, "No se permiten duplicados en AVL.");
                    return 0;
                }
            }
            values[structure->node_count] = value;
            if (!ss_rebuild_numeric_tree(structure, values, structure->node_count + 1, 1, error)) {
                return 0;
            }
            structure->variant = SS_VARIANT_AVL;
            ss_str_copy(out_message, out_message_capacity, "AVL rebalanceado con el valor agregado.");
            return 1;
        }

        case SS_VARIANT_HEAP:
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_heap_sift_up(structure, structure->node_count - 1);
            ss_rebuild_structure_internals(structure);
            snprintf(out_message, out_message_capacity, "Valor %d insertado en heap.", value);
            return 1;

        case SS_VARIANT_SET:
            if (primary == NULL || primary[0] == '\0') {
                ss_error_set(error, SS_ERROR_VALIDATION, "El set requiere un valor.");
                return 0;
            }
            if (ss_value_exists(structure, primary)) {
                ss_error_set(error, SS_ERROR_DUPLICATE, "No se permiten duplicados en set.");
                return 0;
            }
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_format_message(out_message, out_message_capacity, "Elemento %s agregado al set.", node->value);
            structure->dirty_layout = 1;
            return 1;

        case SS_VARIANT_MAP:
            if (primary == NULL || primary[0] == '\0' || secondary == NULL || secondary[0] == '\0') {
                ss_error_set(error, SS_ERROR_VALIDATION, "Map requiere clave y valor.");
                return 0;
            }
            if (ss_map_key_exists(structure, primary, NULL)) {
                ss_error_set(error, SS_ERROR_DUPLICATE, "La clave ya existe en el map.");
                return 0;
            }
            node = ss_insert_node_at(structure, structure->node_count, secondary, error);
            if (node == NULL) {
                return 0;
            }
            ss_str_copy(node->data.key, sizeof(node->data.key), primary);
            ss_format_map_label(node);
            ss_format_message(out_message, out_message_capacity, "Par %s insertado.", primary);
            structure->dirty_layout = 1;
            return 1;

        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            /* In graph variants, primary adds vertices. Edge creation lives in
             * the secondary command so the UI can model source -> target flow. */
            node = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (node == NULL) {
                return 0;
            }
            ss_str_copy(node->label, sizeof(node->label), primary != NULL && primary[0] != '\0' ? primary : node->id);
            ss_apply_round_node_visual(node);
            structure->dirty_layout = 1;
            ss_format_message(out_message, out_message_capacity, "Vertice %s agregado.", node->label);
            return 1;

        default:
            break;
    }

    (void) selected_node_id;
    ss_error_set(error, SS_ERROR_INVALID_STATE, "Operacion primaria no soportada.");
    return 0;
}
