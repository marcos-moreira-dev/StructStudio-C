/*
 * StructStudio C
 * --------------
 * Smoke coverage for guided analysis playback.
 *
 * The intent is to lock the contract between core step builders, editor
 * playback state and the textual report that still accompanies the guided run.
 */

#include <stdio.h>
#include <string.h>

#include "editor/editor.h"

static const char *ss_find_node_id_by_value(const SsStructure *structure, const char *value)
{
    if (structure == NULL || value == NULL) {
        return NULL;
    }

    for (size_t index = 0; index < structure->node_count; ++index) {
        if (strcmp(structure->nodes[index].value, value) == 0 || strcmp(structure->nodes[index].label, value) == 0) {
            return structure->nodes[index].id;
        }
    }

    return NULL;
}

static int ss_connect_selected_to(
    SsEditorState *editor,
    const char *source_id,
    const char *target_id,
    double weight,
    SsError *error)
{
    ss_editor_select_node(editor, source_id);
    if (!ss_editor_connect(editor, source_id, "graph_link", weight, error)) {
        return 0;
    }
    return ss_editor_connect(editor, target_id, "graph_link", weight, error);
}

int main(void)
{
    SsEditorState editor;
    SsError error;
    const SsAnalysisStep *step;
    const SsStructure *structure;
    const char *node_five;
    const char *node_ten;
    int visited = 0;
    int current = 0;

    ss_editor_init(&editor);
    ss_error_clear(&error);

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_BST, &error)) {
        fprintf(stderr, "replace BST failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "10", "", 10, &error) ||
        !ss_editor_apply_primary(&editor, "5", "", 5, &error) ||
        !ss_editor_apply_primary(&editor, "15", "", 15, &error)) {
        fprintf(stderr, "BST setup failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_prepare_analysis_playback(&editor, SS_ANALYSIS_TREE_INORDER, "", &error)) {
        fprintf(stderr, "prepare inorder playback failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_analysis_playback_has_steps(&editor) || editor.playback.step_count != 3) {
        fprintf(stderr, "unexpected inorder playback size: %zu\n", editor.playback.step_count);
        return 1;
    }
    if (!strstr(editor.analysis_report, "Inorden: 5 -> 10 -> 15")) {
        fprintf(stderr, "unexpected inorder report: %s\n", editor.analysis_report);
        return 1;
    }

    structure = ss_document_active_structure_const(&editor.document);
    node_five = ss_find_node_id_by_value(structure, "5");
    node_ten = ss_find_node_id_by_value(structure, "10");
    step = ss_editor_current_analysis_step(&editor);
    if (step == NULL || node_five == NULL || strcmp(step->node_id, node_five) != 0) {
        fprintf(stderr, "unexpected first inorder step\n");
        return 1;
    }
    if (!ss_editor_analysis_playback_next(&editor)) {
        fprintf(stderr, "could not move to second inorder step\n");
        return 1;
    }
    step = ss_editor_current_analysis_step(&editor);
    if (step == NULL || node_ten == NULL || strcmp(step->node_id, node_ten) != 0) {
        fprintf(stderr, "unexpected second inorder step\n");
        return 1;
    }
    if (!ss_editor_playback_node_state(&editor, node_five, &visited, &current) || !visited || current) {
        fprintf(stderr, "unexpected playback state for value 5\n");
        return 1;
    }
    if (!ss_editor_playback_node_state(&editor, node_ten, &visited, &current) || !visited || !current) {
        fprintf(stderr, "unexpected playback state for value 10\n");
        return 1;
    }
    if (ss_analysis_supports_playback(SS_ANALYSIS_GRAPH_KRUSKAL)) {
        fprintf(stderr, "kruskal should not expose guided playback yet\n");
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_DIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace directed weighted graph failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "A", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "B", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "C", "", 0, &error)) {
        fprintf(stderr, "weighted graph setup failed: %s\n", error.message);
        return 1;
    }

    structure = ss_document_active_structure_const(&editor.document);
    if (structure == NULL || structure->node_count != 3) {
        fprintf(stderr, "unexpected weighted graph state\n");
        return 1;
    }
    if (!ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[1].id, 4.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[1].id, structure->nodes[2].id, 3.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[2].id, 10.0, &error)) {
        fprintf(stderr, "weighted graph connections failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_prepare_analysis_playback(&editor, SS_ANALYSIS_GRAPH_DIJKSTRA, "A", &error)) {
        fprintf(stderr, "prepare dijkstra playback failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_analysis_playback_has_steps(&editor) || editor.playback.step_count < 3) {
        fprintf(stderr, "unexpected dijkstra playback size: %zu\n", editor.playback.step_count);
        return 1;
    }
    if (!strstr(editor.analysis_report, "Dijkstra desde A")) {
        fprintf(stderr, "unexpected dijkstra report: %s\n", editor.analysis_report);
        return 1;
    }
    step = ss_editor_current_analysis_step(&editor);
    if (step == NULL || strstr(step->message, "Fijar") == NULL) {
        fprintf(stderr, "unexpected dijkstra first step\n");
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
