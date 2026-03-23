/*
 * StructStudio C
 * --------------
 * JSON import/export.
 *
 * The saved format preserves both semantics and layout because the application
 * is didactic: students are expected to reopen the same visual arrangement.
 */

#include "persistence/document_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

static cJSON *ss_json_value_from_node(const SsNode *node)
{
    int int_value;

    /* The node model stores textual values generically, but JSON should still
     * preserve integer payloads as numbers whenever possible. */
    if (strcmp(node->value_type, "int") == 0 && ss_parse_int(node->value, &int_value)) {
        return cJSON_CreateNumber(int_value);
    }
    return cJSON_CreateString(node->value);
}

static void ss_json_add_node(cJSON *array, const SsNode *node)
{
    cJSON *json_node = cJSON_CreateObject();
    cJSON *visual = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    /* Visual placement and semantic data are serialized separately so the file
     * explains both what the node means and where it appears on screen. */
    cJSON_AddStringToObject(json_node, "node_id", node->id);
    cJSON_AddStringToObject(json_node, "kind", node->kind);
    cJSON_AddStringToObject(json_node, "label", node->label);
    cJSON_AddItemToObject(json_node, "value", ss_json_value_from_node(node));
    cJSON_AddStringToObject(json_node, "value_type", node->value_type);

    cJSON_AddNumberToObject(visual, "x", node->visual.x);
    cJSON_AddNumberToObject(visual, "y", node->visual.y);
    cJSON_AddNumberToObject(visual, "width", node->visual.width);
    cJSON_AddNumberToObject(visual, "height", node->visual.height);
    cJSON_AddItemToObject(json_node, "visual", visual);

    cJSON_AddStringToObject(data, "key", node->data.key);
    cJSON_AddStringToObject(data, "aux_text", node->data.aux_text);
    cJSON_AddStringToObject(data, "ref_a", node->data.ref_a);
    cJSON_AddStringToObject(data, "ref_b", node->data.ref_b);
    cJSON_AddStringToObject(data, "ref_c", node->data.ref_c);
    cJSON_AddNumberToObject(data, "priority", node->data.priority);
    cJSON_AddNumberToObject(data, "index_hint", node->data.index_hint);
    cJSON_AddItemToObject(json_node, "data", data);

    cJSON_AddItemToArray(array, json_node);
}

static void ss_json_add_edge(cJSON *array, const SsEdge *edge)
{
    cJSON *json_edge = cJSON_CreateObject();
    cJSON *visual = cJSON_CreateObject();

    cJSON_AddStringToObject(json_edge, "edge_id", edge->id);
    cJSON_AddStringToObject(json_edge, "source_node_id", edge->source_id);
    cJSON_AddStringToObject(json_edge, "target_node_id", edge->target_id);
    cJSON_AddStringToObject(json_edge, "relation_kind", edge->relation_kind);
    cJSON_AddBoolToObject(json_edge, "is_directed", edge->is_directed);
    if (edge->has_weight) {
        cJSON_AddNumberToObject(json_edge, "weight", edge->weight);
    } else {
        cJSON_AddNullToObject(json_edge, "weight");
    }
    cJSON_AddBoolToObject(visual, "show_arrow", edge->visual.show_arrow);
    cJSON_AddBoolToObject(visual, "show_weight", edge->visual.show_weight);
    cJSON_AddItemToObject(json_edge, "visual", visual);
    cJSON_AddItemToObject(json_edge, "data", cJSON_CreateObject());
    cJSON_AddItemToArray(array, json_edge);
}

int ss_document_save_json(const SsDocument *document, const char *path, SsError *error)
{
    FILE *file;
    char *rendered;
    cJSON *root = cJSON_CreateObject();
    cJSON *json_document = cJSON_CreateObject();
    cJSON *metadata = cJSON_CreateObject();
    cJSON *view_state = cJSON_CreateObject();
    cJSON *structures = cJSON_CreateArray();

    /* Saving is a two-step process:
     * 1. build the full cJSON tree in memory,
     * 2. write the final text only if serialization succeeded. */
    if (document == NULL || path == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Ruta de guardado invalida.");
        return 0;
    }

    cJSON_AddNumberToObject(root, "format_version", 1);
    cJSON_AddItemToObject(root, "document", json_document);

    cJSON_AddStringToObject(metadata, "document_id", document->metadata.document_id);
    cJSON_AddStringToObject(metadata, "name", document->metadata.name);
    cJSON_AddStringToObject(metadata, "description", document->metadata.description);
    cJSON_AddStringToObject(metadata, "created_at", document->metadata.created_at);
    cJSON_AddStringToObject(metadata, "updated_at", document->metadata.updated_at);
    cJSON_AddItemToObject(json_document, "metadata", metadata);

    if (document->structure_count > 0) {
        cJSON_AddStringToObject(json_document, "active_structure_id", document->structures[document->active_structure_index].id);
    } else {
        cJSON_AddStringToObject(json_document, "active_structure_id", "");
    }

    cJSON_AddBoolToObject(view_state, "show_grid", document->view_state.show_grid);
    cJSON_AddBoolToObject(view_state, "left_panel_visible", document->view_state.left_panel_visible);
    cJSON_AddBoolToObject(view_state, "right_panel_visible", document->view_state.right_panel_visible);
    cJSON_AddNumberToObject(view_state, "canvas_offset_x", document->view_state.canvas_offset_x);
    cJSON_AddNumberToObject(view_state, "canvas_offset_y", document->view_state.canvas_offset_y);
    cJSON_AddItemToObject(json_document, "view_state", view_state);

    for (size_t index = 0; index < document->structure_count; ++index) {
        const SsStructure *structure = &document->structures[index];
        cJSON *json_structure = cJSON_CreateObject();
        cJSON *config = cJSON_CreateObject();
        cJSON *visual = cJSON_CreateObject();
        cJSON *nodes = cJSON_CreateArray();
        cJSON *edges = cJSON_CreateArray();

        cJSON_AddStringToObject(json_structure, "structure_id", structure->id);
        cJSON_AddStringToObject(json_structure, "family", ss_family_to_json_name(structure->family));
        cJSON_AddStringToObject(json_structure, "variant", ss_variant_descriptor(structure->variant)->json_name);

        cJSON_AddBoolToObject(config, "is_directed", structure->config.is_directed);
        cJSON_AddBoolToObject(config, "is_weighted", structure->config.is_weighted);
        cJSON_AddBoolToObject(config, "is_circular", structure->config.is_circular);
        cJSON_AddBoolToObject(config, "show_indices", structure->config.show_indices);
        cJSON_AddNumberToObject(config, "capacity", structure->config.capacity);
        cJSON_AddBoolToObject(config, "heap_is_min", structure->config.heap_is_min);
        cJSON_AddItemToObject(json_structure, "config", config);

        cJSON_AddStringToObject(visual, "layout_mode", structure->visual_state.layout_mode);
        cJSON_AddBoolToObject(visual, "show_labels", structure->visual_state.show_labels);
        cJSON_AddBoolToObject(visual, "show_weights", structure->visual_state.show_weights);
        cJSON_AddBoolToObject(visual, "show_indices", structure->visual_state.show_indices);
        cJSON_AddItemToObject(json_structure, "visual_state", visual);

        for (size_t node_index = 0; node_index < structure->node_count; ++node_index) {
            ss_json_add_node(nodes, &structure->nodes[node_index]);
        }
        for (size_t edge_index = 0; edge_index < structure->edge_count; ++edge_index) {
            ss_json_add_edge(edges, &structure->edges[edge_index]);
        }

        cJSON_AddItemToObject(json_structure, "nodes", nodes);
        cJSON_AddItemToObject(json_structure, "edges", edges);
        cJSON_AddItemToArray(structures, json_structure);
    }

    cJSON_AddItemToObject(json_document, "structures", structures);

    rendered = cJSON_Print(root);
    cJSON_Delete(root);
    if (rendered == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo serializar el documento.");
        return 0;
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        cJSON_free(rendered);
        ss_error_set(error, SS_ERROR_IO, "No se pudo abrir el archivo para escritura.");
        return 0;
    }

    fputs(rendered, file);
    fclose(file);
    cJSON_free(rendered);
    ss_error_clear(error);
    return 1;
}

static int ss_json_read_file(const char *path, char **buffer, SsError *error)
{
    FILE *file;
    long length;

    /* Whole-file loading keeps the import path easier to study than a
     * streaming parser, which is acceptable for the document sizes here. */
    file = fopen(path, "rb");
    if (file == NULL) {
        ss_error_set(error, SS_ERROR_IO, "No se pudo abrir el archivo.");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char *) malloc((size_t) length + 1);
    if (*buffer == NULL) {
        fclose(file);
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para el archivo.");
        return 0;
    }

    fread(*buffer, 1, (size_t) length, file);
    fclose(file);
    (*buffer)[length] = '\0';
    return 1;
}

static void ss_json_to_string(char *dest, size_t capacity, const cJSON *item)
{
    /* Import is forgiving: many fields can arrive either as JSON strings or
     * numbers, but the in-memory model still normalizes them as text. */
    if (cJSON_IsString(item) && item->valuestring != NULL) {
        ss_str_copy(dest, capacity, item->valuestring);
    } else if (cJSON_IsNumber(item)) {
        snprintf(dest, capacity, "%d", item->valueint);
    } else {
        dest[0] = '\0';
    }
}

int ss_document_load_json(SsDocument *document, const char *path, SsError *error)
{
    char *buffer = NULL;
    cJSON *root = NULL;
    cJSON *json_document;
    cJSON *metadata;
    cJSON *view_state;
    cJSON *structures;
    cJSON *active_id;

    /* Load reconstructs a new document snapshot instead of trying to patch an
     * existing one in place. That greatly simplifies ownership and recovery. */
    if (!ss_json_read_file(path, &buffer, error)) {
        return 0;
    }

    root = cJSON_Parse(buffer);
    free(buffer);
    if (root == NULL) {
        ss_error_set(error, SS_ERROR_PARSE, "JSON invalido.");
        return 0;
    }

    ss_document_init(document, "Cargado");
    ss_str_copy(document->save_path, sizeof(document->save_path), path);

    json_document = cJSON_GetObjectItemCaseSensitive(root, "document");
    metadata = json_document != NULL ? cJSON_GetObjectItemCaseSensitive(json_document, "metadata") : NULL;
    view_state = json_document != NULL ? cJSON_GetObjectItemCaseSensitive(json_document, "view_state") : NULL;
    structures = json_document != NULL ? cJSON_GetObjectItemCaseSensitive(json_document, "structures") : NULL;
    active_id = json_document != NULL ? cJSON_GetObjectItemCaseSensitive(json_document, "active_structure_id") : NULL;

    if (metadata != NULL) {
        ss_json_to_string(document->metadata.document_id, sizeof(document->metadata.document_id), cJSON_GetObjectItemCaseSensitive(metadata, "document_id"));
        ss_json_to_string(document->metadata.name, sizeof(document->metadata.name), cJSON_GetObjectItemCaseSensitive(metadata, "name"));
        ss_json_to_string(document->metadata.description, sizeof(document->metadata.description), cJSON_GetObjectItemCaseSensitive(metadata, "description"));
        ss_json_to_string(document->metadata.created_at, sizeof(document->metadata.created_at), cJSON_GetObjectItemCaseSensitive(metadata, "created_at"));
        ss_json_to_string(document->metadata.updated_at, sizeof(document->metadata.updated_at), cJSON_GetObjectItemCaseSensitive(metadata, "updated_at"));
    }

    if (view_state != NULL) {
        document->view_state.show_grid = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(view_state, "show_grid"));
        document->view_state.left_panel_visible = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(view_state, "left_panel_visible"));
        document->view_state.right_panel_visible = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(view_state, "right_panel_visible"));
        document->view_state.canvas_offset_x = cJSON_GetObjectItemCaseSensitive(view_state, "canvas_offset_x")->valuedouble;
        document->view_state.canvas_offset_y = cJSON_GetObjectItemCaseSensitive(view_state, "canvas_offset_y")->valuedouble;
    }

    if (structures != NULL && cJSON_IsArray(structures)) {
        cJSON *json_structure = NULL;
        cJSON_ArrayForEach(json_structure, structures) {
            SsStructure *structure;
            cJSON *nodes;
            cJSON *edges;
            cJSON *config;
            cJSON *visual;
            const char *variant_name = cJSON_GetObjectItemCaseSensitive(json_structure, "variant")->valuestring;

            structure = ss_document_add_structure(document, ss_variant_from_json_name(variant_name), NULL, error);
            if (structure == NULL) {
                cJSON_Delete(root);
                return 0;
            }

            ss_json_to_string(structure->id, sizeof(structure->id), cJSON_GetObjectItemCaseSensitive(json_structure, "structure_id"));
            config = cJSON_GetObjectItemCaseSensitive(json_structure, "config");
            visual = cJSON_GetObjectItemCaseSensitive(json_structure, "visual_state");
            if (config != NULL) {
                structure->config.is_directed = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(config, "is_directed"));
                structure->config.is_weighted = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(config, "is_weighted"));
                structure->config.is_circular = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(config, "is_circular"));
                structure->config.show_indices = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(config, "show_indices"));
                structure->config.capacity = cJSON_GetObjectItemCaseSensitive(config, "capacity")->valueint;
                structure->config.heap_is_min = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(config, "heap_is_min"));
            }
            if (visual != NULL) {
                ss_json_to_string(structure->visual_state.layout_mode, sizeof(structure->visual_state.layout_mode), cJSON_GetObjectItemCaseSensitive(visual, "layout_mode"));
                structure->visual_state.show_labels = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(visual, "show_labels"));
                structure->visual_state.show_weights = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(visual, "show_weights"));
                structure->visual_state.show_indices = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(visual, "show_indices"));
            }

            nodes = cJSON_GetObjectItemCaseSensitive(json_structure, "nodes");
            if (nodes != NULL && cJSON_IsArray(nodes)) {
                cJSON *json_node = NULL;
                cJSON_ArrayForEach(json_node, nodes) {
                    SsNode *node;
                    cJSON *visual_node;
                    cJSON *data;

                    if (!ss_array_reserve((void **) &structure->nodes, sizeof(*structure->nodes), &structure->node_capacity, structure->node_count + 1)) {
                        cJSON_Delete(root);
                        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para nodos.");
                        return 0;
                    }
                    node = &structure->nodes[structure->node_count++];
                    memset(node, 0, sizeof(*node));
                    ss_json_to_string(node->id, sizeof(node->id), cJSON_GetObjectItemCaseSensitive(json_node, "node_id"));
                    ss_json_to_string(node->kind, sizeof(node->kind), cJSON_GetObjectItemCaseSensitive(json_node, "kind"));
                    ss_json_to_string(node->label, sizeof(node->label), cJSON_GetObjectItemCaseSensitive(json_node, "label"));
                    ss_json_to_string(node->value, sizeof(node->value), cJSON_GetObjectItemCaseSensitive(json_node, "value"));
                    ss_json_to_string(node->value_type, sizeof(node->value_type), cJSON_GetObjectItemCaseSensitive(json_node, "value_type"));

                    visual_node = cJSON_GetObjectItemCaseSensitive(json_node, "visual");
                    if (visual_node != NULL) {
                        node->visual.x = cJSON_GetObjectItemCaseSensitive(visual_node, "x")->valuedouble;
                        node->visual.y = cJSON_GetObjectItemCaseSensitive(visual_node, "y")->valuedouble;
                        node->visual.width = cJSON_GetObjectItemCaseSensitive(visual_node, "width")->valuedouble;
                        node->visual.height = cJSON_GetObjectItemCaseSensitive(visual_node, "height")->valuedouble;
                    }

                    data = cJSON_GetObjectItemCaseSensitive(json_node, "data");
                    if (data != NULL) {
                        ss_json_to_string(node->data.key, sizeof(node->data.key), cJSON_GetObjectItemCaseSensitive(data, "key"));
                        ss_json_to_string(node->data.aux_text, sizeof(node->data.aux_text), cJSON_GetObjectItemCaseSensitive(data, "aux_text"));
                        ss_json_to_string(node->data.ref_a, sizeof(node->data.ref_a), cJSON_GetObjectItemCaseSensitive(data, "ref_a"));
                        ss_json_to_string(node->data.ref_b, sizeof(node->data.ref_b), cJSON_GetObjectItemCaseSensitive(data, "ref_b"));
                        ss_json_to_string(node->data.ref_c, sizeof(node->data.ref_c), cJSON_GetObjectItemCaseSensitive(data, "ref_c"));
                        if (cJSON_GetObjectItemCaseSensitive(data, "priority") != NULL) {
                            node->data.priority = cJSON_GetObjectItemCaseSensitive(data, "priority")->valueint;
                        }
                        if (cJSON_GetObjectItemCaseSensitive(data, "index_hint") != NULL) {
                            node->data.index_hint = cJSON_GetObjectItemCaseSensitive(data, "index_hint")->valueint;
                        }
                    }
                }
            }

            edges = cJSON_GetObjectItemCaseSensitive(json_structure, "edges");
            if (edges != NULL && cJSON_IsArray(edges)) {
                cJSON *json_edge = NULL;
                cJSON_ArrayForEach(json_edge, edges) {
                    SsEdge *edge;
                    cJSON *visual_edge;

                    if (!ss_array_reserve((void **) &structure->edges, sizeof(*structure->edges), &structure->edge_capacity, structure->edge_count + 1)) {
                        cJSON_Delete(root);
                        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para aristas.");
                        return 0;
                    }
                    edge = &structure->edges[structure->edge_count++];
                    memset(edge, 0, sizeof(*edge));
                    ss_json_to_string(edge->id, sizeof(edge->id), cJSON_GetObjectItemCaseSensitive(json_edge, "edge_id"));
                    ss_json_to_string(edge->source_id, sizeof(edge->source_id), cJSON_GetObjectItemCaseSensitive(json_edge, "source_node_id"));
                    ss_json_to_string(edge->target_id, sizeof(edge->target_id), cJSON_GetObjectItemCaseSensitive(json_edge, "target_node_id"));
                    ss_json_to_string(edge->relation_kind, sizeof(edge->relation_kind), cJSON_GetObjectItemCaseSensitive(json_edge, "relation_kind"));
                    edge->is_directed = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json_edge, "is_directed"));
                    if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(json_edge, "weight"))) {
                        edge->weight = cJSON_GetObjectItemCaseSensitive(json_edge, "weight")->valuedouble;
                        edge->has_weight = 1;
                    }
                    visual_edge = cJSON_GetObjectItemCaseSensitive(json_edge, "visual");
                    if (visual_edge != NULL) {
                        edge->visual.show_arrow = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(visual_edge, "show_arrow"));
                        edge->visual.show_weight = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(visual_edge, "show_weight"));
                    }
                }
            }

            if ((structure->variant == SS_VARIANT_BINARY_TREE || structure->variant == SS_VARIANT_BST || structure->variant == SS_VARIANT_AVL || structure->variant == SS_VARIANT_HEAP) && structure->node_count > 0) {
                ss_str_copy(structure->root_id, sizeof(structure->root_id), structure->nodes[0].id);
                for (size_t i = 0; i < structure->node_count; ++i) {
                    if (structure->nodes[i].data.ref_c[0] == '\0') {
                        ss_str_copy(structure->root_id, sizeof(structure->root_id), structure->nodes[i].id);
                        break;
                    }
                }
            }
        }
    }

    if (active_id != NULL && cJSON_IsString(active_id)) {
        for (size_t index = 0; index < document->structure_count; ++index) {
            if (strcmp(document->structures[index].id, active_id->valuestring) == 0) {
                document->active_structure_index = index;
                break;
            }
        }
    }

    document->dirty = 0;
    cJSON_Delete(root);
    ss_error_clear(error);
    return 1;
}
