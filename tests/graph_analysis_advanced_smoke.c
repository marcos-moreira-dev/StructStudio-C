/*
 * StructStudio C
 * --------------
 * Smoke coverage for advanced graph analyses.
 *
 * These checks keep all-pairs shortest paths and MST algorithms under a small
 * but stable regression net using deterministic weighted graphs.
 */

#include <stdio.h>
#include <string.h>

#include "editor/editor.h"

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
    SsStructure *structure;

    ss_editor_init(&editor);
    ss_error_clear(&error);

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_DIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace directed weighted graph failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "A", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "B", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "C", "", 0, &error)) {
        fprintf(stderr, "directed graph setup failed: %s\n", error.message);
        return 1;
    }

    structure = ss_document_active_structure(&editor.document);
    if (structure == NULL || structure->node_count != 3) {
        fprintf(stderr, "unexpected directed weighted graph state\n");
        return 1;
    }

    if (!ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[1].id, 4.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[1].id, structure->nodes[2].id, 3.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[2].id, 10.0, &error)) {
        fprintf(stderr, "directed graph connections failed: %s\n", error.message);
        return 1;
    }

    if (!ss_editor_run_analysis(&editor, SS_ANALYSIS_GRAPH_FLOYD_WARSHALL, "", &error)) {
        fprintf(stderr, "floyd-warshall failed: %s\n", error.message);
        return 1;
    }
    if (strstr(editor.analysis_report, "A: A=0 | B=4 | C=7") == NULL ||
        strstr(editor.analysis_report, "B: A=INF | B=0 | C=3") == NULL) {
        fprintf(stderr, "unexpected floyd-warshall report: %s\n", editor.analysis_report);
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace undirected weighted graph failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "A", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "B", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "C", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "D", "", 0, &error)) {
        fprintf(stderr, "undirected graph setup failed: %s\n", error.message);
        return 1;
    }

    structure = ss_document_active_structure(&editor.document);
    if (structure == NULL || structure->node_count != 4) {
        fprintf(stderr, "unexpected undirected weighted graph state\n");
        return 1;
    }

    if (!ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[1].id, 1.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[1].id, structure->nodes[2].id, 3.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[2].id, structure->nodes[3].id, 2.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[3].id, 10.0, &error)) {
        fprintf(stderr, "undirected graph connections failed: %s\n", error.message);
        return 1;
    }

    if (!ss_editor_run_analysis(&editor, SS_ANALYSIS_GRAPH_PRIM, "A", &error)) {
        fprintf(stderr, "prim failed: %s\n", error.message);
        return 1;
    }
    if (strstr(editor.analysis_report, "A - B = 1") == NULL ||
        strstr(editor.analysis_report, "C - D = 2") == NULL ||
        strstr(editor.analysis_report, "Costo total = 6") == NULL) {
        fprintf(stderr, "unexpected prim report: %s\n", editor.analysis_report);
        return 1;
    }

    if (!ss_editor_run_analysis(&editor, SS_ANALYSIS_GRAPH_KRUSKAL, "", &error)) {
        fprintf(stderr, "kruskal failed: %s\n", error.message);
        return 1;
    }
    if (strstr(editor.analysis_report, "A - B = 1") == NULL ||
        strstr(editor.analysis_report, "B - C = 3") == NULL ||
        strstr(editor.analysis_report, "Costo total = 6") == NULL) {
        fprintf(stderr, "unexpected kruskal report: %s\n", editor.analysis_report);
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
