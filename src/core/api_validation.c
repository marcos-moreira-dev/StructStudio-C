/*
 * StructStudio C
 * --------------
 * Structural validation rules.
 *
 * Validation stays independent from the UI so the same checks can be reused by
 * the editor, tests and future batch tooling.
 */

#include "api_internal.h"

#include <string.h>

static void ss_push_validation(SsValidationResult *result, SsValidationSeverity severity, const char *entity_id, const char *message)
{
    if (result->count >= SS_VALIDATION_CAPACITY) {
        return;
    }
    result->items[result->count].severity = severity;
    ss_str_copy(result->items[result->count].entity_id, sizeof(result->items[result->count].entity_id), entity_id);
    ss_str_copy(result->items[result->count].message, sizeof(result->items[result->count].message), message);
    result->count += 1;
    if (severity == SS_VALIDATION_ERROR) {
        result->ok = 0;
    }
}

void ss_structure_validate(const SsStructure *structure, SsValidationResult *result)
{
    if (result == NULL) {
        return;
    }

    memset(result, 0, sizeof(*result));
    result->ok = 1;

    if (structure == NULL) {
        ss_push_validation(result, SS_VALIDATION_ERROR, "", "No hay estructura activa.");
        return;
    }

    for (size_t index = 0; index < structure->edge_count; ++index) {
        const SsEdge *edge = &structure->edges[index];
        if (ss_structure_find_node_const(structure, edge->source_id) == NULL || ss_structure_find_node_const(structure, edge->target_id) == NULL) {
            ss_push_validation(result, SS_VALIDATION_ERROR, edge->id, "Hay una arista con referencias rotas.");
        }
        if (structure->config.is_weighted && !edge->has_weight) {
            ss_push_validation(result, SS_VALIDATION_ERROR, edge->id, "La arista requiere peso.");
        }
    }

    if (structure->variant == SS_VARIANT_SET) {
        for (size_t index = 0; index < structure->node_count; ++index) {
            for (size_t inner = index + 1; inner < structure->node_count; ++inner) {
                if (strcmp(structure->nodes[index].value, structure->nodes[inner].value) == 0) {
                    ss_push_validation(result, SS_VALIDATION_ERROR, structure->nodes[inner].id, "Set con valor duplicado.");
                }
            }
        }
    }

    if (structure->variant == SS_VARIANT_MAP) {
        for (size_t index = 0; index < structure->node_count; ++index) {
            if (structure->nodes[index].data.key[0] == '\0') {
                ss_push_validation(result, SS_VALIDATION_ERROR, structure->nodes[index].id, "Hay una entrada de map sin clave.");
            }
            for (size_t inner = index + 1; inner < structure->node_count; ++inner) {
                if (strcmp(structure->nodes[index].data.key, structure->nodes[inner].data.key) == 0) {
                    ss_push_validation(result, SS_VALIDATION_ERROR, structure->nodes[inner].id, "Map con clave duplicada.");
                }
            }
        }
    }

    if ((structure->variant == SS_VARIANT_BST || structure->variant == SS_VARIANT_AVL) && structure->node_count > 0) {
        int values[256];
        if (!ss_collect_int_values(structure, values, 256)) {
            ss_push_validation(result, SS_VALIDATION_ERROR, structure->root_id, "El arbol contiene valores no numericos.");
        }
    }

    if (result->count == 0) {
        ss_push_validation(result, SS_VALIDATION_INFO, structure->id, "Estructura valida.");
    }
}
