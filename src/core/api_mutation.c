/*
 * StructStudio C
 * --------------
 * Direct mutations over existing entities.
 *
 * These functions are shared by the editor and property panel when the user
 * edits, connects or deletes nodes and edges that already exist.
 */

#include "api_internal.h"

#include <stdio.h>
#include <string.h>

int ss_structure_connect(
    SsStructure *structure,
    const char *source_id,
    const char *target_id,
    const char *relation_kind,
    double weight,
    SsError *error)
{
    SsNode *source;
    SsNode *target;

    if (structure == NULL || source_id == NULL || target_id == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Conexion invalida.");
        return 0;
    }

    source = ss_structure_find_node(structure, source_id);
    target = ss_structure_find_node(structure, target_id);
    if (source == NULL || target == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "No se encontraron los nodos para conectar.");
        return 0;
    }

    if (structure->family == SS_FAMILY_GRAPH) {
        if (structure->config.is_weighted && weight == 0.0) {
            ss_error_set(error, SS_ERROR_VALIDATION, "El grafo ponderado requiere un peso.");
            return 0;
        }
        if (ss_append_edge(
                structure,
                source_id,
                target_id,
                relation_kind != NULL ? relation_kind : "graph_link",
                structure->config.is_directed,
                structure->config.is_weighted,
                weight,
                error) == NULL) {
            return 0;
        }
        structure->dirty_layout = 1;
        return 1;
    }

    if (structure->variant == SS_VARIANT_BINARY_TREE) {
        if (strcmp(source->id, target->id) == 0) {
            ss_error_set(error, SS_ERROR_VALIDATION, "Un nodo no puede ser hijo de si mismo.");
            return 0;
        }
        if (relation_kind != NULL && strcmp(relation_kind, "right") == 0) {
            if (source->data.ref_b[0] != '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El nodo ya tiene hijo derecho.");
                return 0;
            }
            ss_str_copy(source->data.ref_b, sizeof(source->data.ref_b), target->id);
        } else {
            if (source->data.ref_a[0] != '\0') {
                ss_error_set(error, SS_ERROR_INVALID_STATE, "El nodo ya tiene hijo izquierdo.");
                return 0;
            }
            ss_str_copy(source->data.ref_a, sizeof(source->data.ref_a), target->id);
        }
        ss_str_copy(target->data.ref_c, sizeof(target->data.ref_c), source->id);
        if (structure->root_id[0] == '\0') {
            ss_str_copy(structure->root_id, sizeof(structure->root_id), source->id);
        }
        ss_rebuild_structure_internals(structure);
        return 1;
    }

    ss_error_set(error, SS_ERROR_INVALID_STATE, "La estructura activa no soporta conexiones manuales.");
    return 0;
}

int ss_structure_delete_node(SsStructure *structure, const char *node_id, SsError *error)
{
    int index = ss_find_node_index(structure, node_id);

    if (index < 0) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo no encontrado.");
        return 0;
    }

    if (structure->variant == SS_VARIANT_BINARY_TREE) {
        SsNode *node = ss_structure_find_node(structure, node_id);
        SsNode *parent = ss_tree_node_by_child(structure, node_id);
        if (parent != NULL) {
            if (strcmp(parent->data.ref_a, node->id) == 0) {
                parent->data.ref_a[0] = '\0';
            } else if (strcmp(parent->data.ref_b, node->id) == 0) {
                parent->data.ref_b[0] = '\0';
            }
        }
        if (strcmp(structure->root_id, node->id) == 0) {
            structure->root_id[0] = '\0';
        }
        /* Binary tree removal is modeled as subtree removal to avoid leaving
         * orphaned descendants with broken parent references. */
        ss_delete_subtree(structure, node_id);
        ss_rebuild_structure_internals(structure);
        return 1;
    }

    ss_remove_node_at(structure, (size_t) index);
    if (structure->variant == SS_VARIANT_HEAP) {
        for (size_t current = structure->node_count / 2; current-- > 0;) {
            ss_heap_sift_down(structure, current);
        }
    }
    ss_rebuild_structure_internals(structure);
    return 1;
}

int ss_structure_delete_edge(SsStructure *structure, const char *edge_id, SsError *error)
{
    int index = ss_find_edge_index(structure, edge_id);

    if (index < 0) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "Arista no encontrada.");
        return 0;
    }

    ss_remove_edge_at(structure, (size_t) index);
    return 1;
}

int ss_structure_update_node(
    SsStructure *structure,
    const char *node_id,
    const char *label,
    const char *value,
    const char *secondary,
    int numeric_value,
    SsError *error)
{
    SsNode *node = ss_structure_find_node(structure, node_id);

    if (node == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "Nodo no encontrado.");
        return 0;
    }

    if (structure->variant == SS_VARIANT_MAP && secondary != NULL && secondary[0] != '\0') {
        if (ss_map_key_exists(structure, secondary, node->id)) {
            ss_error_set(error, SS_ERROR_DUPLICATE, "La clave ya existe.");
            return 0;
        }
        ss_str_copy(node->data.key, sizeof(node->data.key), secondary);
    }
    if (label != NULL && label[0] != '\0') {
        ss_str_copy(node->label, sizeof(node->label), label);
    }
    if (value != NULL) {
        if ((structure->variant == SS_VARIANT_SET) && value[0] != '\0' && ss_value_exists(structure, value) && strcmp(node->value, value) != 0) {
            ss_error_set(error, SS_ERROR_DUPLICATE, "No se permiten duplicados en set.");
            return 0;
        }
        ss_str_copy(node->value, sizeof(node->value), value);
    }
    if (structure->variant == SS_VARIANT_PRIORITY_QUEUE) {
        node->data.priority = numeric_value;
        ss_rebuild_priority_queue(structure);
    }
    if (structure->variant == SS_VARIANT_MAP) {
        ss_format_map_label(node);
    }
    if (structure->variant == SS_VARIANT_BST || structure->variant == SS_VARIANT_AVL || structure->variant == SS_VARIANT_HEAP) {
        int values[256];
        if (!ss_collect_int_values(structure, values, 256)) {
            ss_error_set(error, SS_ERROR_VALIDATION, "La estructura solo admite valores enteros.");
            return 0;
        }
        if (structure->variant == SS_VARIANT_HEAP) {
            for (size_t index = structure->node_count / 2; index-- > 0;) {
                ss_heap_sift_down(structure, index);
            }
            ss_rebuild_heap_tree(structure);
        } else {
            int preserve_avl = structure->variant == SS_VARIANT_AVL;
            if (!ss_rebuild_numeric_tree(structure, values, structure->node_count, preserve_avl, error)) {
                return 0;
            }
            structure->variant = preserve_avl ? SS_VARIANT_AVL : SS_VARIANT_BST;
        }
    }
    structure->dirty_layout = 1;
    return 1;
}

int ss_structure_update_edge(
    SsStructure *structure,
    const char *edge_id,
    const char *relation_kind,
    double weight,
    SsError *error)
{
    SsEdge *edge = ss_structure_find_edge(structure, edge_id);

    if (edge == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "Arista no encontrada.");
        return 0;
    }

    if (relation_kind != NULL && relation_kind[0] != '\0') {
        ss_str_copy(edge->relation_kind, sizeof(edge->relation_kind), relation_kind);
    }
    if (structure->config.is_weighted) {
        edge->weight = weight;
        edge->has_weight = 1;
        edge->visual.show_weight = 1;
    }
    return 1;
}
