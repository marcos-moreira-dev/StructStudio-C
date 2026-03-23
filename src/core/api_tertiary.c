/*
 * StructStudio C
 * --------------
 * Tertiary operations for each variant.
 *
 * These commands are intentionally separated because their semantics differ
 * more widely across structures: peek, delete, rebalance or contextual insert.
 */

#include "api_internal.h"

#include <stdio.h>
#include <string.h>

int ss_structure_apply_tertiary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error,
    char *out_message,
    size_t out_message_capacity)
{
    int value;
    int values[256];
    size_t count;
    int found = 0;

    (void) secondary;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No hay estructura activa.");
        return 0;
    }

    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            if (value < 0 || (size_t) value >= structure->node_count) {
                ss_error_set(error, SS_ERROR_NOT_FOUND, "Indice fuera de rango.");
                return 0;
            }
            structure->nodes[value].value[0] = '\0';
            snprintf(out_message, out_message_capacity, "Celda [%d] vaciada.", value);
            return 1;

        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
            if (selected_node_id == NULL || selected_node_id[0] == '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo de referencia.");
                return 0;
            }
            {
                int selected_index = ss_find_node_index(structure, selected_node_id);
                SsNode *node;
                if (selected_index < 0) {
                    ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo de referencia inexistente.");
                    return 0;
                }
                node = ss_insert_node_at(structure, (size_t) selected_index + 1, primary, error);
                if (node == NULL) {
                    return 0;
                }
                ss_rebuild_structure_internals(structure);
                ss_str_copy(out_message, out_message_capacity, "Nodo insertado despues del seleccionado.");
                return 1;
            }

        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            if (selected_node_id == NULL || selected_node_id[0] == '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo de referencia.");
                return 0;
            }
            {
                int selected_index = ss_find_node_index(structure, selected_node_id);
                SsNode *node;
                if (selected_index < 0) {
                    ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo de referencia inexistente.");
                    return 0;
                }
                node = ss_insert_node_at(structure, (size_t) selected_index, primary, error);
                if (node == NULL) {
                    return 0;
                }
                ss_rebuild_structure_internals(structure);
                ss_str_copy(out_message, out_message_capacity, "Nodo insertado antes del seleccionado.");
                return 1;
            }

        case SS_VARIANT_STACK:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "La pila esta vacia.");
                return 0;
            }
            snprintf(out_message, out_message_capacity, "Tope actual: %s", structure->nodes[structure->node_count - 1].value);
            return 1;

        case SS_VARIANT_QUEUE:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "La cola esta vacia.");
                return 0;
            }
            snprintf(out_message, out_message_capacity, "Frente actual: %s", structure->nodes[0].value);
            return 1;

        case SS_VARIANT_CIRCULAR_QUEUE:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "La cola esta vacia.");
                return 0;
            }
            snprintf(out_message, out_message_capacity, "Rear actual: %s", structure->nodes[structure->node_count - 1].value);
            return 1;

        case SS_VARIANT_PRIORITY_QUEUE:
            if (selected_node_id == NULL || selected_node_id[0] == '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un elemento para editar prioridad.");
                return 0;
            }
            {
                SsNode *node = ss_structure_find_node(structure, selected_node_id);
                if (node == NULL) {
                    ss_error_set(error, SS_ERROR_NOT_FOUND, "Elemento inexistente.");
                    return 0;
                }
                node->data.priority = numeric_value;
                ss_rebuild_structure_internals(structure);
                snprintf(out_message, out_message_capacity, "Prioridad actualizada a %d.", numeric_value);
                return 1;
            }

        case SS_VARIANT_BINARY_TREE:
            if (selected_node_id == NULL || selected_node_id[0] == '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo padre.");
                return 0;
            }
            {
                SsNode *parent = ss_structure_find_node(structure, selected_node_id);
                SsNode *child;
                if (parent == NULL) {
                    ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo padre inexistente.");
                    return 0;
                }
                if (parent->data.ref_b[0] != '\0') {
                    ss_error_set(error, SS_ERROR_INVALID_STATE, "El nodo ya tiene hijo derecho.");
                    return 0;
                }
                child = ss_insert_node_at(structure, structure->node_count, primary, error);
                if (child == NULL) {
                    return 0;
                }
                ss_str_copy(parent->data.ref_b, sizeof(parent->data.ref_b), child->id);
                ss_str_copy(child->data.ref_c, sizeof(child->data.ref_c), parent->id);
                ss_rebuild_structure_internals(structure);
                ss_str_copy(out_message, out_message_capacity, "Hijo derecho agregado.");
                return 1;
            }

        case SS_VARIANT_BST:
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            if (structure->node_count == 0 || !ss_collect_int_values(structure, values, 256)) {
                ss_error_set(error, SS_ERROR_VALIDATION, "BST invalido.");
                return 0;
            }
            count = 0;
            for (size_t index = 0; index < structure->node_count; ++index) {
                if (values[index] == value && !found) {
                    found = 1;
                    continue;
                }
                values[count++] = values[index];
            }
            if (!found) {
                ss_error_set(error, SS_ERROR_NOT_FOUND, "El valor no existe en BST.");
                return 0;
            }
            if (!ss_rebuild_numeric_tree(structure, values, count, 0, error)) {
                return 0;
            }
            ss_str_copy(out_message, out_message_capacity, "Valor eliminado del BST.");
            return 1;

        case SS_VARIANT_AVL:
            if (structure->node_count == 0 || !ss_collect_int_values(structure, values, 256)) {
                ss_error_set(error, SS_ERROR_VALIDATION, "AVL invalido.");
                return 0;
            }
            if (!ss_rebuild_numeric_tree(structure, values, structure->node_count, 1, error)) {
                return 0;
            }
            structure->variant = SS_VARIANT_AVL;
            ss_str_copy(out_message, out_message_capacity, "AVL rebalanceado.");
            return 1;

        case SS_VARIANT_HEAP:
            ss_rebuild_heap_tree(structure);
            ss_str_copy(out_message, out_message_capacity, "Heapify aplicado.");
            return 1;

        case SS_VARIANT_SET:
            if (ss_value_exists(structure, primary)) {
                snprintf(out_message, out_message_capacity, "El valor %s existe en el set.", primary);
                return 1;
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "El valor no existe en el set.");
            return 0;

        case SS_VARIANT_MAP:
            for (count = 0; count < structure->node_count; ++count) {
                if (strcmp(structure->nodes[count].data.key, primary) == 0) {
                    snprintf(
                        out_message,
                        out_message_capacity,
                        "Clave %s encontrada con valor %s.",
                        primary,
                        structure->nodes[count].value);
                    return 1;
                }
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "Clave no encontrada en map.");
            return 0;

        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            if (selected_node_id != NULL && selected_node_id[0] != '\0') {
                int edge_index = ss_find_edge_index(structure, selected_node_id);
                if (edge_index >= 0) {
                    ss_remove_edge_at(structure, (size_t) edge_index);
                    ss_str_copy(out_message, out_message_capacity, "Arista eliminada.");
                    return 1;
                }
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "Seleccione una arista para eliminar.");
            return 0;

        default:
            break;
    }

    ss_error_set(error, SS_ERROR_INVALID_STATE, "Operacion terciaria no soportada.");
    return 0;
}
