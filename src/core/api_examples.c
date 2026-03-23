/*
 * StructStudio C
 * --------------
 * Example dataset builders for the supported TDAs.
 *
 * The objective is educational: the UI can request a representative example
 * without hardcoding structure-specific widget behavior. By keeping the sample
 * recipes here, tests and future import/export features can reuse them too.
 */

#include "api_internal.h"

#include <stdio.h>
#include <string.h>

static int ss_example_apply_primary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    SsError *error)
{
    char message[SS_MESSAGE_CAPACITY];

    return ss_structure_apply_primary(
        structure,
        primary,
        secondary,
        numeric_value,
        "",
        error,
        message,
        sizeof(message));
}

static int ss_example_apply_secondary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error)
{
    char message[SS_MESSAGE_CAPACITY];

    return ss_structure_apply_secondary(
        structure,
        primary,
        secondary,
        numeric_value,
        selected_node_id,
        error,
        message,
        sizeof(message));
}

static int ss_example_apply_tertiary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error)
{
    char message[SS_MESSAGE_CAPACITY];

    return ss_structure_apply_tertiary(
        structure,
        primary,
        secondary,
        numeric_value,
        selected_node_id,
        error,
        message,
        sizeof(message));
}

static SsNode *ss_example_find_node_by_value(const SsStructure *structure, const char *value)
{
    size_t index;

    if (structure == NULL || value == NULL) {
        return NULL;
    }

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].value, value) == 0 || strcmp(structure->nodes[index].label, value) == 0) {
            return &structure->nodes[index];
        }
    }

    return NULL;
}

static int ss_example_connect(
    SsStructure *structure,
    const char *source_value,
    const char *target_value,
    const char *relation_kind,
    double weight,
    SsError *error)
{
    SsNode *source = ss_example_find_node_by_value(structure, source_value);
    SsNode *target = ss_example_find_node_by_value(structure, target_value);

    if (source == NULL || target == NULL) {
        ss_error_set(error, SS_ERROR_NOT_FOUND, "No se encontraron nodos para el ejemplo.");
        return 0;
    }

    return ss_structure_connect(structure, source->id, target->id, relation_kind, weight, error);
}

static int ss_load_binary_tree_example(SsStructure *structure, SsError *error)
{
    SsNode *root;
    SsNode *left;
    SsNode *right;

    if (!ss_example_apply_primary(structure, "40", "", 0, error)) {
        return 0;
    }

    root = ss_structure_find_node(structure, structure->root_id);
    if (root == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No se pudo crear la raiz del ejemplo.");
        return 0;
    }

    if (!ss_example_apply_secondary(structure, "20", "", 0, root->id, error) ||
        !ss_example_apply_tertiary(structure, "60", "", 0, root->id, error)) {
        return 0;
    }

    left = ss_example_find_node_by_value(structure, "20");
    right = ss_example_find_node_by_value(structure, "60");
    if (left == NULL || right == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No se pudieron ubicar hijos del ejemplo.");
        return 0;
    }

    if (!ss_example_apply_secondary(structure, "10", "", 0, left->id, error) ||
        !ss_example_apply_tertiary(structure, "30", "", 0, left->id, error) ||
        !ss_example_apply_secondary(structure, "50", "", 0, right->id, error) ||
        !ss_example_apply_tertiary(structure, "70", "", 0, right->id, error)) {
        return 0;
    }

    return 1;
}

static int ss_load_graph_example(SsStructure *structure, SsError *error)
{
    switch (structure->variant) {
        case SS_VARIANT_DIRECTED_GRAPH:
            return ss_example_apply_primary(structure, "A", "", 0, error) &&
                ss_example_apply_primary(structure, "B", "", 0, error) &&
                ss_example_apply_primary(structure, "C", "", 0, error) &&
                ss_example_apply_primary(structure, "D", "", 0, error) &&
                ss_example_connect(structure, "A", "B", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "A", "C", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "B", "D", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "C", "D", "graph_link", 0.0, error);
        case SS_VARIANT_UNDIRECTED_GRAPH:
            return ss_example_apply_primary(structure, "A", "", 0, error) &&
                ss_example_apply_primary(structure, "B", "", 0, error) &&
                ss_example_apply_primary(structure, "C", "", 0, error) &&
                ss_example_apply_primary(structure, "D", "", 0, error) &&
                ss_example_apply_primary(structure, "E", "", 0, error) &&
                ss_example_connect(structure, "A", "B", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "A", "C", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "B", "D", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "C", "D", "graph_link", 0.0, error) &&
                ss_example_connect(structure, "D", "E", "graph_link", 0.0, error);
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
            return ss_example_apply_primary(structure, "A", "", 0, error) &&
                ss_example_apply_primary(structure, "B", "", 0, error) &&
                ss_example_apply_primary(structure, "C", "", 0, error) &&
                ss_example_apply_primary(structure, "D", "", 0, error) &&
                ss_example_connect(structure, "A", "B", "graph_link", 4.0, error) &&
                ss_example_connect(structure, "A", "C", "graph_link", 2.0, error) &&
                ss_example_connect(structure, "C", "B", "graph_link", 1.0, error) &&
                ss_example_connect(structure, "B", "D", "graph_link", 5.0, error) &&
                ss_example_connect(structure, "C", "D", "graph_link", 8.0, error);
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            return ss_example_apply_primary(structure, "A", "", 0, error) &&
                ss_example_apply_primary(structure, "B", "", 0, error) &&
                ss_example_apply_primary(structure, "C", "", 0, error) &&
                ss_example_apply_primary(structure, "D", "", 0, error) &&
                ss_example_apply_primary(structure, "E", "", 0, error) &&
                ss_example_connect(structure, "A", "B", "graph_link", 4.0, error) &&
                ss_example_connect(structure, "A", "C", "graph_link", 2.0, error) &&
                ss_example_connect(structure, "B", "C", "graph_link", 1.0, error) &&
                ss_example_connect(structure, "B", "D", "graph_link", 5.0, error) &&
                ss_example_connect(structure, "C", "D", "graph_link", 8.0, error) &&
                ss_example_connect(structure, "C", "E", "graph_link", 10.0, error) &&
                ss_example_connect(structure, "D", "E", "graph_link", 2.0, error);
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "La variante activa no corresponde a un grafo.");
            return 0;
    }
}

int ss_structure_load_example(
    SsStructure *structure,
    char *out_message,
    size_t out_message_capacity,
    SsError *error)
{
    int ok = 0;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No hay estructura activa para poblar.");
        return 0;
    }

    if (out_message != NULL && out_message_capacity > 0) {
        out_message[0] = '\0';
    }

    ss_structure_clear(structure);

    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            ok = ss_example_apply_primary(structure, "0", "13", 0, error) &&
                ss_example_apply_primary(structure, "1", "21", 0, error) &&
                ss_example_apply_primary(structure, "2", "34", 0, error) &&
                ss_example_apply_primary(structure, "3", "55", 0, error);
            break;
        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            ok = ss_example_apply_primary(structure, "A", "", 0, error) &&
                ss_example_apply_secondary(structure, "B", "", 0, "", error) &&
                ss_example_apply_secondary(structure, "C", "", 0, "", error) &&
                ss_example_apply_secondary(structure, "D", "", 0, "", error);
            break;
        case SS_VARIANT_STACK:
            ok = ss_example_apply_primary(structure, "main()", "", 0, error) &&
                ss_example_apply_primary(structure, "parse()", "", 0, error) &&
                ss_example_apply_primary(structure, "render()", "", 0, error);
            break;
        case SS_VARIANT_QUEUE:
            ok = ss_example_apply_primary(structure, "P1", "", 0, error) &&
                ss_example_apply_primary(structure, "P2", "", 0, error) &&
                ss_example_apply_primary(structure, "P3", "", 0, error) &&
                ss_example_apply_primary(structure, "P4", "", 0, error);
            break;
        case SS_VARIANT_CIRCULAR_QUEUE:
            ok = ss_example_apply_primary(structure, "N1", "", 0, error) &&
                ss_example_apply_primary(structure, "N2", "", 0, error) &&
                ss_example_apply_primary(structure, "N3", "", 0, error) &&
                ss_example_apply_secondary(structure, "", "", 0, "", error) &&
                ss_example_apply_primary(structure, "N4", "", 0, error) &&
                ss_example_apply_primary(structure, "N5", "", 0, error);
            break;
        case SS_VARIANT_PRIORITY_QUEUE:
            ok = ss_example_apply_primary(structure, "Backup", "", 2, error) &&
                ss_example_apply_primary(structure, "Incidente", "", 9, error) &&
                ss_example_apply_primary(structure, "Correo", "", 4, error);
            break;
        case SS_VARIANT_BINARY_TREE:
            ok = ss_load_binary_tree_example(structure, error);
            break;
        case SS_VARIANT_BST:
            ok = ss_example_apply_primary(structure, "40", "", 0, error) &&
                ss_example_apply_primary(structure, "20", "", 0, error) &&
                ss_example_apply_primary(structure, "60", "", 0, error) &&
                ss_example_apply_primary(structure, "10", "", 0, error) &&
                ss_example_apply_primary(structure, "30", "", 0, error) &&
                ss_example_apply_primary(structure, "50", "", 0, error) &&
                ss_example_apply_primary(structure, "70", "", 0, error);
            break;
        case SS_VARIANT_AVL:
            ok = ss_example_apply_primary(structure, "30", "", 0, error) &&
                ss_example_apply_primary(structure, "20", "", 0, error) &&
                ss_example_apply_primary(structure, "10", "", 0, error) &&
                ss_example_apply_primary(structure, "25", "", 0, error) &&
                ss_example_apply_primary(structure, "40", "", 0, error) &&
                ss_example_apply_primary(structure, "50", "", 0, error);
            break;
        case SS_VARIANT_HEAP:
            ok = ss_example_apply_primary(structure, "50", "", 0, error) &&
                ss_example_apply_primary(structure, "35", "", 0, error) &&
                ss_example_apply_primary(structure, "42", "", 0, error) &&
                ss_example_apply_primary(structure, "18", "", 0, error) &&
                ss_example_apply_primary(structure, "27", "", 0, error) &&
                ss_example_apply_primary(structure, "12", "", 0, error);
            break;
        case SS_VARIANT_SET:
            ok = ss_example_apply_primary(structure, "rojo", "", 0, error) &&
                ss_example_apply_primary(structure, "verde", "", 0, error) &&
                ss_example_apply_primary(structure, "azul", "", 0, error);
            break;
        case SS_VARIANT_MAP:
            ok = ss_example_apply_primary(structure, "id", "42", 0, error) &&
                ss_example_apply_primary(structure, "nombre", "cola", 0, error) &&
                ss_example_apply_primary(structure, "estado", "activo", 0, error);
            break;
        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            ok = ss_load_graph_example(structure, error);
            break;
        default:
            ss_error_set(error, SS_ERROR_INVALID_STATE, "No existe un ejemplo para la variante activa.");
            return 0;
    }

    if (!ok) {
        return 0;
    }

    ss_structure_auto_layout(structure, 720.0);
    if (out_message != NULL && out_message_capacity > 0) {
        snprintf(
            out_message,
            out_message_capacity,
            "Ejemplo didactico cargado para %s.",
            ss_variant_descriptor(structure->variant)->display_name);
    }
    ss_error_clear(error);
    return 1;
}
