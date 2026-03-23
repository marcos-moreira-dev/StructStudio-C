#ifndef SS_CORE_API_H
#define SS_CORE_API_H

#include "core/model.h"

typedef enum SsAnalysisKind {
    SS_ANALYSIS_NONE = 0,
    SS_ANALYSIS_TREE_PREORDER,
    SS_ANALYSIS_TREE_INORDER,
    SS_ANALYSIS_TREE_POSTORDER,
    SS_ANALYSIS_TREE_LEVEL_ORDER,
    SS_ANALYSIS_GRAPH_BFS,
    SS_ANALYSIS_GRAPH_DFS,
    SS_ANALYSIS_GRAPH_DIJKSTRA,
    SS_ANALYSIS_GRAPH_FLOYD_WARSHALL,
    SS_ANALYSIS_GRAPH_PRIM,
    SS_ANALYSIS_GRAPH_KRUSKAL
} SsAnalysisKind;

typedef enum SsAnalysisStartMode {
    SS_ANALYSIS_START_NONE = 0,
    SS_ANALYSIS_START_OPTIONAL,
    SS_ANALYSIS_START_REQUIRED
} SsAnalysisStartMode;

typedef enum SsAnalysisStepKind {
    SS_ANALYSIS_STEP_VISIT = 0,
    SS_ANALYSIS_STEP_DISCOVER,
    SS_ANALYSIS_STEP_RELAX,
    SS_ANALYSIS_STEP_FINALIZE
} SsAnalysisStepKind;

/*
 * A playback step is intentionally compact: the core names the relevant node
 * and/or edge and leaves the final highlighting strategy to the editor/render
 * layers. This keeps the algorithms reusable outside the GUI.
 */
typedef struct SsAnalysisStep {
    SsAnalysisStepKind kind;
    char node_id[SS_ID_CAPACITY];
    char edge_id[SS_ID_CAPACITY];
    char message[SS_MESSAGE_CAPACITY];
} SsAnalysisStep;

int ss_structure_apply_primary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error,
    char *out_message,
    size_t out_message_capacity);

int ss_structure_apply_secondary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error,
    char *out_message,
    size_t out_message_capacity);

int ss_structure_apply_tertiary(
    SsStructure *structure,
    const char *primary,
    const char *secondary,
    int numeric_value,
    const char *selected_node_id,
    SsError *error,
    char *out_message,
    size_t out_message_capacity);

int ss_structure_connect(
    SsStructure *structure,
    const char *source_id,
    const char *target_id,
    const char *relation_kind,
    double weight,
    SsError *error);

int ss_structure_delete_node(SsStructure *structure, const char *node_id, SsError *error);
int ss_structure_delete_edge(SsStructure *structure, const char *edge_id, SsError *error);
int ss_structure_update_node(
    SsStructure *structure,
    const char *node_id,
    const char *label,
    const char *value,
    const char *secondary,
    int numeric_value,
    SsError *error);

int ss_structure_update_edge(
    SsStructure *structure,
    const char *edge_id,
    const char *relation_kind,
    double weight,
    SsError *error);

const char *ss_analysis_kind_label(SsAnalysisKind kind);
const char *ss_analysis_step_kind_label(SsAnalysisStepKind kind);
size_t ss_analysis_kinds_for_variant(SsVariant variant, SsAnalysisKind *items, size_t capacity);
SsAnalysisStartMode ss_analysis_start_mode(SsAnalysisKind kind);
int ss_analysis_requires_start_node(SsAnalysisKind kind);
int ss_analysis_supports_playback(SsAnalysisKind kind);
int ss_analysis_supports_tree_generation(SsAnalysisKind kind);
int ss_structure_run_analysis(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    char *report,
    size_t report_capacity,
    SsError *error);
int ss_structure_build_analysis_playback(
    const SsStructure *structure,
    SsAnalysisKind kind,
    const char *start_node_id,
    SsAnalysisStep **steps,
    size_t *step_count,
    char *report,
    size_t report_capacity,
    SsError *error);
int ss_build_theory_summary(
    const SsStructure *structure,
    SsAnalysisKind kind,
    char *buffer,
    size_t capacity,
    SsError *error);
int ss_structure_rotate_left(SsStructure *structure, const char *pivot_node_id, SsError *error);
int ss_structure_rotate_right(SsStructure *structure, const char *pivot_node_id, SsError *error);
int ss_structure_rotate_graph_layout(SsStructure *structure, double radians, const char *pivot_node_id, SsError *error);
int ss_structure_build_graph_tree(
    const SsStructure *source,
    SsAnalysisKind kind,
    const char *start_node_id,
    SsStructure *out_structure,
    char *out_message,
    size_t out_message_capacity,
    SsError *error);
int ss_structure_load_example(
    SsStructure *structure,
    char *out_message,
    size_t out_message_capacity,
    SsError *error);

void ss_structure_auto_layout(SsStructure *structure, double canvas_width);
void ss_structure_validate(const SsStructure *structure, SsValidationResult *result);

#endif
