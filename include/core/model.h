/*
 * StructStudio C
 * --------------
 * Core data model.
 *
 * The model is intentionally UI-agnostic: documents, structures, nodes and
 * edges can be saved, cloned, validated and tested without any widget code.
 */

#ifndef SS_CORE_MODEL_H
#define SS_CORE_MODEL_H

#include <stddef.h>

#include "common/error.h"
#include "common/util.h"

/* Families are broad categories used by render/layout/theory code to make
 * coarse decisions without listing every specific variant each time. */
typedef enum SsFamily {
    SS_FAMILY_VECTOR = 0,
    SS_FAMILY_LIST,
    SS_FAMILY_STACK,
    SS_FAMILY_QUEUE,
    SS_FAMILY_TREE,
    SS_FAMILY_HEAP,
    SS_FAMILY_SET,
    SS_FAMILY_MAP,
    SS_FAMILY_GRAPH
} SsFamily;

/* Variants are the concrete TDAs shown to the user in the application. */
typedef enum SsVariant {
    SS_VARIANT_VECTOR = 0,
    SS_VARIANT_SINGLY_LINKED_LIST,
    SS_VARIANT_DOUBLY_LINKED_LIST,
    SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST,
    SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST,
    SS_VARIANT_STACK,
    SS_VARIANT_QUEUE,
    SS_VARIANT_CIRCULAR_QUEUE,
    SS_VARIANT_PRIORITY_QUEUE,
    SS_VARIANT_BINARY_TREE,
    SS_VARIANT_BST,
    SS_VARIANT_AVL,
    SS_VARIANT_HEAP,
    SS_VARIANT_SET,
    SS_VARIANT_MAP,
    SS_VARIANT_DIRECTED_GRAPH,
    SS_VARIANT_UNDIRECTED_GRAPH,
    SS_VARIANT_DIRECTED_WEIGHTED_GRAPH,
    SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH,
    SS_VARIANT_COUNT
} SsVariant;

/* Descriptors form a small catalog of capabilities and default labels for
 * each variant. The UI reads them to avoid hardcoding strings everywhere. */
typedef struct SsVariantDescriptor {
    SsVariant variant;
    SsFamily family;
    const char *json_name;
    const char *display_name;
    const char *operation_primary;
    const char *operation_secondary;
    const char *operation_tertiary;
    int supports_connect;
    int supports_manual_drag;
} SsVariantDescriptor;

/* Geometry is persisted because StructStudio is both an editor and a study
 * tool: a reopened document should preserve its educational arrangement. */
typedef struct SsRect {
    double x;
    double y;
    double width;
    double height;
} SsRect;

/* NodeData stores semantic helper fields shared across many TDAs. Different
 * variants reuse different fields, which is why the node model is generic. */
typedef struct SsNodeData {
    char key[SS_LABEL_CAPACITY];
    char aux_text[SS_VALUE_CAPACITY];
    char ref_a[SS_ID_CAPACITY];
    char ref_b[SS_ID_CAPACITY];
    char ref_c[SS_ID_CAPACITY];
    int priority;
    int index_hint;
} SsNodeData;

/* A node mixes persistent data with visual placement. That may look unusual in
 * pure domain modeling, but it makes animation/export/save behavior coherent. */
typedef struct SsNode {
    char id[SS_ID_CAPACITY];
    char kind[SS_KIND_CAPACITY];
    char label[SS_LABEL_CAPACITY];
    char value[SS_VALUE_CAPACITY];
    char value_type[SS_VALUE_TYPE_CAPACITY];
    SsRect visual;
    SsNodeData data;
} SsNode;

/* EdgeVisual answers "how should this relationship be painted?" while the edge
 * itself answers "what relationship exists semantically?". */
typedef struct SsEdgeVisual {
    int show_arrow;
    int show_weight;
} SsEdgeVisual;

typedef struct SsEdge {
    char id[SS_ID_CAPACITY];
    char source_id[SS_ID_CAPACITY];
    char target_id[SS_ID_CAPACITY];
    char relation_kind[SS_RELATION_CAPACITY];
    int is_directed;
    int has_weight;
    double weight;
    SsEdgeVisual visual;
} SsEdge;

typedef struct SsStructureConfig {
    int is_directed;
    int is_weighted;
    int is_circular;
    int show_indices;
    int capacity;
    int heap_is_min;
} SsStructureConfig;

typedef struct SsStructureVisualState {
    char layout_mode[SS_LAYOUT_CAPACITY];
    int show_labels;
    int show_weights;
    int show_indices;
} SsStructureVisualState;

/* Structures are the actual tabs/documents the user manipulates on screen. */
typedef struct SsStructure {
    char id[SS_ID_CAPACITY];
    SsFamily family;
    SsVariant variant;
    SsStructureConfig config;
    SsStructureVisualState visual_state;
    SsNode *nodes;
    size_t node_count;
    size_t node_capacity;
    SsEdge *edges;
    size_t edge_count;
    size_t edge_capacity;
    char root_id[SS_ID_CAPACITY];
    int dirty_layout;
} SsStructure;

typedef struct SsDocumentMetadata {
    char document_id[SS_ID_CAPACITY];
    char name[SS_NAME_CAPACITY];
    char description[SS_DESCRIPTION_CAPACITY];
    char created_at[32];
    char updated_at[32];
} SsDocumentMetadata;

typedef struct SsViewState {
    int show_grid;
    int left_panel_visible;
    int right_panel_visible;
    double canvas_offset_x;
    double canvas_offset_y;
} SsViewState;

/* The document is the long-lived root object of the session. */
typedef struct SsDocument {
    SsDocumentMetadata metadata;
    SsViewState view_state;
    SsStructure *structures;
    size_t structure_count;
    size_t structure_capacity;
    size_t active_structure_index;
    char save_path[SS_PATH_CAPACITY];
    int dirty;
} SsDocument;

typedef enum SsValidationSeverity {
    SS_VALIDATION_INFO = 0,
    SS_VALIDATION_WARNING,
    SS_VALIDATION_ERROR
} SsValidationSeverity;

typedef struct SsValidationMessage {
    SsValidationSeverity severity;
    char entity_id[SS_ID_CAPACITY];
    char message[SS_MESSAGE_CAPACITY];
} SsValidationMessage;

typedef struct SsValidationResult {
    int ok;
    size_t count;
    SsValidationMessage items[SS_VALIDATION_CAPACITY];
} SsValidationResult;

void ss_document_init(SsDocument *document, const char *name);
void ss_document_free(SsDocument *document);
void ss_document_touch(SsDocument *document);
SsStructure *ss_document_active_structure(SsDocument *document);
const SsStructure *ss_document_active_structure_const(const SsDocument *document);
SsStructure *ss_document_add_structure(SsDocument *document, SsVariant variant, const char *custom_name, SsError *error);
int ss_document_set_active_structure(SsDocument *document, size_t index, SsError *error);

void ss_structure_init(SsStructure *structure, SsVariant variant, const char *id_hint);
void ss_structure_free(SsStructure *structure);
void ss_structure_clear(SsStructure *structure);
int ss_structure_clone(SsStructure *dest, const SsStructure *src, SsError *error);
SsNode *ss_structure_find_node(SsStructure *structure, const char *id);
const SsNode *ss_structure_find_node_const(const SsStructure *structure, const char *id);
const SsNode *ss_structure_find_node_by_token_const(const SsStructure *structure, const char *token, SsError *error);
SsEdge *ss_structure_find_edge(SsStructure *structure, const char *id);
const SsEdge *ss_structure_find_edge_const(const SsStructure *structure, const char *id);
const SsVariantDescriptor *ss_variant_descriptor(SsVariant variant);
size_t ss_variant_descriptor_count(void);
const SsVariantDescriptor *ss_variant_descriptor_at(size_t index);
SsVariant ss_variant_from_json_name(const char *json_name);
const char *ss_family_to_json_name(SsFamily family);
const char *ss_default_relation_for_variant(SsVariant variant);

#endif
