/*
 * StructStudio C
 * --------------
 * Secondary operations for each variant.
 *
 * Secondary actions typically represent the complementary command from the UI:
 * pop/dequeue, append/search, or connect depending on the active structure.
 */

#include "api_internal.h"

#include <stdio.h>
#include <string.h>

int ss_structure_apply_secondary(
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
            ss_str_copy(structure->nodes[value].value, sizeof(structure->nodes[value].value), secondary != NULL ? secondary : "");
            snprintf(out_message, out_message_capacity, "Celda [%d] reemplazada.", value);
            return 1;

        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
        {
            SsNode *tail = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (tail == NULL) {
                return 0;
            }
            ss_rebuild_structure_internals(structure);
            ss_format_message(out_message, out_message_capacity, "Nodo %s agregado al final.", tail->id);
            return 1;
        }

        case SS_VARIANT_STACK:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "La pila esta vacia.");
                return 0;
            }
            ss_remove_node_at(structure, structure->node_count - 1);
            ss_rebuild_structure_internals(structure);
            ss_str_copy(out_message, out_message_capacity, "Pop ejecutado.");
            return 1;

        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "La cola esta vacia.");
                return 0;
            }
            ss_remove_node_at(structure, 0);
            ss_rebuild_structure_internals(structure);
            ss_str_copy(out_message, out_message_capacity, "Dequeue ejecutado.");
            return 1;

        case SS_VARIANT_PRIORITY_QUEUE:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "La priority queue esta vacia.");
                return 0;
            }
            ss_remove_node_at(structure, 0);
            ss_rebuild_structure_internals(structure);
            ss_str_copy(out_message, out_message_capacity, "Elemento prioritario extraido.");
            return 1;

        case SS_VARIANT_BINARY_TREE:
        {
            SsNode *parent;
            SsNode *child;

            if (selected_node_id == NULL || selected_node_id[0] == '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo padre.");
                return 0;
            }
            parent = ss_structure_find_node(structure, selected_node_id);
            if (parent == NULL) {
                ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo padre inexistente.");
                return 0;
            }
            if (parent->data.ref_a[0] != '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El nodo ya tiene hijo izquierdo.");
                return 0;
            }
            child = ss_insert_node_at(structure, structure->node_count, primary, error);
            if (child == NULL) {
                return 0;
            }
            ss_str_copy(parent->data.ref_a, sizeof(parent->data.ref_a), child->id);
            ss_str_copy(child->data.ref_c, sizeof(child->data.ref_c), parent->id);
            ss_rebuild_structure_internals(structure);
            ss_str_copy(out_message, out_message_capacity, "Hijo izquierdo agregado.");
            return 1;
        }

        case SS_VARIANT_BST:
        {
            SsNode *current;
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            current = ss_structure_find_node(structure, structure->root_id);
            while (current != NULL) {
                int current_value;
                if (!ss_node_int_value(current, &current_value)) {
                    ss_error_set(error, SS_ERROR_VALIDATION, "BST contiene valores no numericos.");
                    return 0;
                }
                if (value == current_value) {
                    snprintf(out_message, out_message_capacity, "Valor %d encontrado en %s.", value, current->id);
                    return 1;
                }
                current = value < current_value
                    ? (current->data.ref_a[0] != '\0' ? ss_structure_find_node(structure, current->data.ref_a) : NULL)
                    : (current->data.ref_b[0] != '\0' ? ss_structure_find_node(structure, current->data.ref_b) : NULL);
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "El valor no existe en el BST.");
            return 0;
        }

        case SS_VARIANT_AVL:
            if (structure->node_count == 0 || !ss_collect_int_values(structure, values, 256)) {
                ss_error_set(error, SS_ERROR_VALIDATION, "AVL contiene valores invalidos.");
                return 0;
            }
            if (!ss_format_numeric_input(primary, numeric_value, &value, error)) {
                return 0;
            }
            count = 0;
            for (size_t i = 0; i < structure->node_count; ++i) {
                if (values[i] != value) {
                    values[count++] = values[i];
                }
            }
            if (count == structure->node_count) {
                ss_error_set(error, SS_ERROR_NOT_FOUND, "El valor no existe en AVL.");
                return 0;
            }
            if (!ss_rebuild_numeric_tree(structure, values, count, 1, error)) {
                return 0;
            }
            structure->variant = SS_VARIANT_AVL;
            ss_str_copy(out_message, out_message_capacity, "AVL actualizado y balanceado.");
            return 1;

        case SS_VARIANT_HEAP:
            if (structure->node_count == 0) {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El heap esta vacio.");
                return 0;
            }
            structure->nodes[0] = structure->nodes[structure->node_count - 1];
            structure->node_count -= 1;
            if (structure->node_count > 0) {
                ss_heap_sift_down(structure, 0);
            }
            ss_rebuild_structure_internals(structure);
            ss_str_copy(out_message, out_message_capacity, "Raiz extraida del heap.");
            return 1;

        case SS_VARIANT_SET:
            for (count = 0; count < structure->node_count; ++count) {
                if (strcmp(structure->nodes[count].value, primary) == 0) {
                    ss_remove_node_at(structure, count);
                    ss_str_copy(out_message, out_message_capacity, "Elemento eliminado del set.");
                    return 1;
                }
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "Elemento no encontrado en set.");
            return 0;

        case SS_VARIANT_MAP:
            for (count = 0; count < structure->node_count; ++count) {
                if (strcmp(structure->nodes[count].data.key, primary) == 0) {
                    ss_str_copy(structure->nodes[count].value, sizeof(structure->nodes[count].value), secondary != NULL ? secondary : "");
                    ss_format_map_label(&structure->nodes[count]);
                    ss_str_copy(out_message, out_message_capacity, "Map actualizado.");
                    return 1;
                }
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "Clave no encontrada en map.");
            return 0;

        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
        {
            const SsNode *target;

            if (selected_node_id == NULL || selected_node_id[0] == '\0' || secondary == NULL || secondary[0] == '\0') {
                ss_error_set(error, SS_ERROR_VALIDATION, "Seleccione un vertice y escriba un destino por ID, etiqueta o valor en el campo secundario.");
                return 0;
            }

            target = ss_structure_find_node_by_token_const(structure, secondary, error);
            if (target == NULL) {
                if (!ss_error_is_ok(error)) {
                    return 0;
                }
                ss_error_set(error, SS_ERROR_NOT_FOUND, "No se encontro el vertice destino indicado.");
                return 0;
            }

            if (!ss_structure_connect(structure, selected_node_id, target->id, "graph_link", (double) numeric_value, error)) {
                return 0;
            }
            ss_str_copy(out_message, out_message_capacity, "Arista agregada.");
            return 1;
        }

        default:
            break;
    }
    ss_error_set(error, SS_ERROR_INVALID_STATE, "Operacion secundaria no soportada.");
    return 0;
}
