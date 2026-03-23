#include "core/model.h"

#include <stdlib.h>
#include <string.h>

static const SsVariantDescriptor SS_VARIANTS[SS_VARIANT_COUNT] = {
    { SS_VARIANT_VECTOR, SS_FAMILY_VECTOR, "vector", "Vector", "Insertar", "Reemplazar", "Eliminar", 0, 0 },
    { SS_VARIANT_SINGLY_LINKED_LIST, SS_FAMILY_LIST, "singly_linked_list", "Lista simple", "Inicio", "Final", "Después", 0, 0 },
    { SS_VARIANT_DOUBLY_LINKED_LIST, SS_FAMILY_LIST, "doubly_linked_list", "Lista doble", "Inicio", "Final", "Antes", 0, 0 },
    { SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST, SS_FAMILY_LIST, "circular_singly_linked_list", "Lista circular simple", "Inicio", "Final", "Después", 0, 0 },
    { SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST, SS_FAMILY_LIST, "circular_doubly_linked_list", "Lista circular doble", "Inicio", "Final", "Antes", 0, 0 },
    { SS_VARIANT_STACK, SS_FAMILY_STACK, "stack", "Stack", "Push", "Pop", "Peek", 0, 0 },
    { SS_VARIANT_QUEUE, SS_FAMILY_QUEUE, "queue", "Queue", "Enqueue", "Dequeue", "Front", 0, 0 },
    { SS_VARIANT_CIRCULAR_QUEUE, SS_FAMILY_QUEUE, "circular_queue", "Circular Queue", "Enqueue", "Dequeue", "Rear", 0, 0 },
    { SS_VARIANT_PRIORITY_QUEUE, SS_FAMILY_QUEUE, "priority_queue", "Priority Queue", "Insertar", "Extraer", "Prioridad", 0, 0 },
    { SS_VARIANT_BINARY_TREE, SS_FAMILY_TREE, "binary_tree", "Árbol binario", "Raíz", "Hijo izq", "Hijo der", 1, 0 },
    { SS_VARIANT_BST, SS_FAMILY_TREE, "bst", "BST", "Insertar", "Buscar", "Eliminar", 0, 0 },
    { SS_VARIANT_AVL, SS_FAMILY_TREE, "avl", "AVL", "Insertar", "Eliminar", "Rebalancear", 0, 0 },
    { SS_VARIANT_HEAP, SS_FAMILY_HEAP, "heap", "Heap", "Insertar", "Extraer", "Heapify", 0, 0 },
    { SS_VARIANT_SET, SS_FAMILY_SET, "set", "Set", "Agregar", "Eliminar", "Buscar", 0, 0 },
    { SS_VARIANT_MAP, SS_FAMILY_MAP, "map", "Map", "Insertar par", "Actualizar", "Buscar", 0, 0 },
    { SS_VARIANT_DIRECTED_GRAPH, SS_FAMILY_GRAPH, "directed_graph", "Grafo dirigido", "Vértice", "Arista", "Eliminar arista", 1, 1 },
    { SS_VARIANT_UNDIRECTED_GRAPH, SS_FAMILY_GRAPH, "undirected_graph", "Grafo no dirigido", "Vértice", "Arista", "Eliminar arista", 1, 1 },
    { SS_VARIANT_DIRECTED_WEIGHTED_GRAPH, SS_FAMILY_GRAPH, "directed_weighted_graph", "Grafo dirigido ponderado", "Vértice", "Arista", "Eliminar arista", 1, 1 },
    { SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH, SS_FAMILY_GRAPH, "undirected_weighted_graph", "Grafo no dirigido ponderado", "Vértice", "Arista", "Eliminar arista", 1, 1 }
};

static void ss_structure_configure_variant(SsStructure *structure)
{
    const SsVariantDescriptor *descriptor = ss_variant_descriptor(structure->variant);

    structure->family = descriptor->family;
    structure->config.is_directed =
        structure->variant == SS_VARIANT_DIRECTED_GRAPH ||
        structure->variant == SS_VARIANT_DIRECTED_WEIGHTED_GRAPH;
    structure->config.is_weighted =
        structure->variant == SS_VARIANT_DIRECTED_WEIGHTED_GRAPH ||
        structure->variant == SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH ||
        structure->variant == SS_VARIANT_PRIORITY_QUEUE;
    structure->config.is_circular =
        structure->variant == SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST ||
        structure->variant == SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST ||
        structure->variant == SS_VARIANT_CIRCULAR_QUEUE;
    structure->config.show_indices = structure->variant == SS_VARIANT_VECTOR;
    structure->config.capacity = 8;
    structure->config.heap_is_min = 0;
    ss_str_copy(structure->visual_state.layout_mode, sizeof(structure->visual_state.layout_mode), "auto");
    structure->visual_state.show_labels = 1;
    structure->visual_state.show_weights = structure->config.is_weighted;
    structure->visual_state.show_indices = structure->config.show_indices;
}

void ss_document_init(SsDocument *document, const char *name)
{
    memset(document, 0, sizeof(*document));
    ss_generate_id("doc", document->metadata.document_id, sizeof(document->metadata.document_id));
    ss_str_copy(document->metadata.name, sizeof(document->metadata.name), name != NULL ? name : "Nuevo documento");
    ss_timestamp_now(document->metadata.created_at, sizeof(document->metadata.created_at));
    ss_str_copy(document->metadata.updated_at, sizeof(document->metadata.updated_at), document->metadata.created_at);
    document->view_state.show_grid = 1;
    document->view_state.left_panel_visible = 1;
    document->view_state.right_panel_visible = 1;
    document->active_structure_index = 0;
}

void ss_document_free(SsDocument *document)
{
    size_t index;

    if (document == NULL) {
        return;
    }

    for (index = 0; index < document->structure_count; ++index) {
        ss_structure_free(&document->structures[index]);
    }
    free(document->structures);
    memset(document, 0, sizeof(*document));
}

void ss_document_touch(SsDocument *document)
{
    if (document == NULL) {
        return;
    }

    document->dirty = 1;
    ss_timestamp_now(document->metadata.updated_at, sizeof(document->metadata.updated_at));
}

SsStructure *ss_document_active_structure(SsDocument *document)
{
    if (document == NULL || document->structure_count == 0 || document->active_structure_index >= document->structure_count) {
        return NULL;
    }
    return &document->structures[document->active_structure_index];
}

const SsStructure *ss_document_active_structure_const(const SsDocument *document)
{
    if (document == NULL || document->structure_count == 0 || document->active_structure_index >= document->structure_count) {
        return NULL;
    }
    return &document->structures[document->active_structure_index];
}

SsStructure *ss_document_add_structure(SsDocument *document, SsVariant variant, const char *custom_name, SsError *error)
{
    SsStructure *structure;
    char structure_id[SS_ID_CAPACITY];
    (void) custom_name;

    if (document == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Documento no valido.");
        return NULL;
    }

    if (!ss_array_reserve((void **) &document->structures, sizeof(*document->structures), &document->structure_capacity, document->structure_count + 1)) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para la estructura.");
        return NULL;
    }

    structure = &document->structures[document->structure_count];
    ss_generate_id("structure", structure_id, sizeof(structure_id));
    ss_structure_init(structure, variant, structure_id);
    document->active_structure_index = document->structure_count;
    document->structure_count += 1;
    ss_document_touch(document);
    ss_error_clear(error);
    return structure;
}

int ss_document_set_active_structure(SsDocument *document, size_t index, SsError *error)
{
    if (document == NULL || index >= document->structure_count) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Indice de estructura fuera de rango.");
        return 0;
    }

    document->active_structure_index = index;
    ss_error_clear(error);
    return 1;
}

void ss_structure_init(SsStructure *structure, SsVariant variant, const char *id_hint)
{
    memset(structure, 0, sizeof(*structure));
    structure->variant = variant;
    ss_str_copy(structure->id, sizeof(structure->id), id_hint);
    if (structure->id[0] == '\0') {
        ss_generate_id("structure", structure->id, sizeof(structure->id));
    }
    ss_structure_configure_variant(structure);
    structure->dirty_layout = 1;
}

void ss_structure_free(SsStructure *structure)
{
    if (structure == NULL) {
        return;
    }

    free(structure->nodes);
    free(structure->edges);
    memset(structure, 0, sizeof(*structure));
}

void ss_structure_clear(SsStructure *structure)
{
    if (structure == NULL) {
        return;
    }

    free(structure->nodes);
    free(structure->edges);
    structure->nodes = NULL;
    structure->node_count = 0;
    structure->node_capacity = 0;
    structure->edges = NULL;
    structure->edge_count = 0;
    structure->edge_capacity = 0;
    structure->root_id[0] = '\0';
    structure->dirty_layout = 1;
}

int ss_structure_clone(SsStructure *dest, const SsStructure *src, SsError *error)
{
    SsStructure clone;

    if (dest == NULL || src == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No se pudo clonar la estructura.");
        return 0;
    }

    memset(&clone, 0, sizeof(clone));
    clone = *src;
    clone.nodes = NULL;
    clone.edges = NULL;

    if (src->node_count > 0) {
        clone.nodes = (SsNode *) calloc(src->node_count, sizeof(*clone.nodes));
        if (clone.nodes == NULL) {
            ss_error_set(error, SS_ERROR_MEMORY, "No se pudo clonar la lista de nodos.");
            return 0;
        }
        memcpy(clone.nodes, src->nodes, src->node_count * sizeof(*clone.nodes));
    }

    if (src->edge_count > 0) {
        clone.edges = (SsEdge *) calloc(src->edge_count, sizeof(*clone.edges));
        if (clone.edges == NULL) {
            free(clone.nodes);
            ss_error_set(error, SS_ERROR_MEMORY, "No se pudo clonar la lista de aristas.");
            return 0;
        }
        memcpy(clone.edges, src->edges, src->edge_count * sizeof(*clone.edges));
    }

    ss_structure_free(dest);
    *dest = clone;
    ss_error_clear(error);
    return 1;
}

SsNode *ss_structure_find_node(SsStructure *structure, const char *id)
{
    size_t index;

    if (structure == NULL || id == NULL) {
        return NULL;
    }

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].id, id) == 0) {
            return &structure->nodes[index];
        }
    }
    return NULL;
}

const SsNode *ss_structure_find_node_const(const SsStructure *structure, const char *id)
{
    size_t index;

    if (structure == NULL || id == NULL) {
        return NULL;
    }

    for (index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].id, id) == 0) {
            return &structure->nodes[index];
        }
    }
    return NULL;
}

SsEdge *ss_structure_find_edge(SsStructure *structure, const char *id)
{
    size_t index;

    if (structure == NULL || id == NULL) {
        return NULL;
    }

    for (index = 0; index < structure->edge_count; ++index) {
        if (strcmp(structure->edges[index].id, id) == 0) {
            return &structure->edges[index];
        }
    }
    return NULL;
}

const SsEdge *ss_structure_find_edge_const(const SsStructure *structure, const char *id)
{
    size_t index;

    if (structure == NULL || id == NULL) {
        return NULL;
    }

    for (index = 0; index < structure->edge_count; ++index) {
        if (strcmp(structure->edges[index].id, id) == 0) {
            return &structure->edges[index];
        }
    }
    return NULL;
}

const SsVariantDescriptor *ss_variant_descriptor(SsVariant variant)
{
    if (variant < 0 || variant >= SS_VARIANT_COUNT) {
        return &SS_VARIANTS[0];
    }
    return &SS_VARIANTS[variant];
}

size_t ss_variant_descriptor_count(void)
{
    return SS_VARIANT_COUNT;
}

const SsVariantDescriptor *ss_variant_descriptor_at(size_t index)
{
    if (index >= SS_VARIANT_COUNT) {
        return NULL;
    }
    return &SS_VARIANTS[index];
}

SsVariant ss_variant_from_json_name(const char *json_name)
{
    size_t index;

    if (json_name == NULL) {
        return SS_VARIANT_VECTOR;
    }

    for (index = 0; index < SS_VARIANT_COUNT; ++index) {
        if (strcmp(SS_VARIANTS[index].json_name, json_name) == 0) {
            return SS_VARIANTS[index].variant;
        }
    }

    return SS_VARIANT_VECTOR;
}

const char *ss_family_to_json_name(SsFamily family)
{
    switch (family) {
        case SS_FAMILY_VECTOR:
            return "vector";
        case SS_FAMILY_LIST:
            return "list";
        case SS_FAMILY_STACK:
            return "stack";
        case SS_FAMILY_QUEUE:
            return "queue";
        case SS_FAMILY_TREE:
            return "tree";
        case SS_FAMILY_HEAP:
            return "heap";
        case SS_FAMILY_SET:
            return "set";
        case SS_FAMILY_MAP:
            return "map";
        case SS_FAMILY_GRAPH:
            return "graph";
        default:
            return "vector";
    }
}

const char *ss_default_relation_for_variant(SsVariant variant)
{
    switch (variant) {
        case SS_VARIANT_BINARY_TREE:
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
        case SS_VARIANT_HEAP:
            return "left";
        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            return "graph_link";
        default:
            return "next";
    }
}
