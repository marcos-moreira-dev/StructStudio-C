/*
 * StructStudio C
 * --------------
 * Internal contracts shared by the core API implementation files.
 *
 * This header is intentionally private to src/core. It allows the public API
 * to stay compact while the implementation is split into smaller modules.
 */

#ifndef SS_CORE_API_INTERNAL_H
#define SS_CORE_API_INTERNAL_H

#include "core/api.h"

extern const double SS_NODE_WIDTH;
extern const double SS_NODE_HEIGHT;
extern const double SS_ROUND_NODE_SIZE;

const char *ss_node_kind_for_variant(SsVariant variant);
int ss_variant_prefers_round_nodes(SsVariant variant);
void ss_apply_round_node_visual(SsNode *node);

int ss_find_node_index(const SsStructure *structure, const char *node_id);
int ss_find_edge_index(const SsStructure *structure, const char *edge_id);

int ss_reserve_node(SsStructure *structure, size_t required, SsError *error);
void ss_assign_node_defaults(SsNode *node, const char *kind, const char *value);
void ss_format_priority_label(SsNode *node);
void ss_format_map_label(SsNode *node);
SsNode *ss_insert_node_at(SsStructure *structure, size_t index, const char *value, SsError *error);
SsNode *ss_append_node_with_id(
    SsStructure *structure,
    const char *id,
    const char *kind,
    const char *label,
    const char *value,
    SsError *error);
SsEdge *ss_append_edge(
    SsStructure *structure,
    const char *source_id,
    const char *target_id,
    const char *relation_kind,
    int directed,
    int has_weight,
    double weight,
    SsError *error);
void ss_remove_edge_at(SsStructure *structure, size_t index);
void ss_remove_node_at(SsStructure *structure, size_t index);

void ss_rebuild_priority_queue(SsStructure *structure);
void ss_rebuild_structure_internals(SsStructure *structure);

int ss_node_int_value(const SsNode *node, int *out_value);
void ss_rebuild_tree_edges(SsStructure *structure);
void ss_rebuild_heap_tree(SsStructure *structure);
void ss_heap_sift_up(SsStructure *structure, size_t index);
void ss_heap_sift_down(SsStructure *structure, size_t index);
SsNode *ss_tree_node_by_child(SsStructure *structure, const char *child_id);
void ss_delete_subtree(SsStructure *structure, const char *node_id);
int ss_collect_int_values(const SsStructure *structure, int *values, size_t capacity);
int ss_bst_insert_value(SsStructure *structure, int value, SsError *error);
int ss_rebuild_numeric_tree(SsStructure *structure, int *values, size_t count, int balanced, SsError *error);

int ss_value_exists(const SsStructure *structure, const char *value);
int ss_map_key_exists(const SsStructure *structure, const char *key, const char *ignore_node_id);
void ss_format_message(char *buffer, size_t capacity, const char *fmt, const char *label);
int ss_format_numeric_input(const char *primary, int numeric_value, int *out_value, SsError *error);
int ss_run_graph_advanced_analysis(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error);

#endif
