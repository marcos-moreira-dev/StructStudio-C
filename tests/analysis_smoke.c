/*
 * StructStudio C
 * --------------
 * Smoke coverage for educational analyses.
 *
 * These checks verify that the analysis layer stays connected to the editor
 * state and produces stable textual summaries for representative structures.
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
    if (!ss_editor_run_analysis(&editor, SS_ANALYSIS_TREE_INORDER, "", &error)) {
        fprintf(stderr, "tree inorder failed: %s\n", error.message);
        return 1;
    }
    if (strstr(editor.analysis_report, "Inorden: 5 -> 10 -> 15") == NULL) {
        fprintf(stderr, "unexpected inorder report: %s\n", editor.analysis_report);
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_DIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace weighted graph failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "A", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "B", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "C", "", 0, &error)) {
        fprintf(stderr, "graph setup failed: %s\n", error.message);
        return 1;
    }

    structure = ss_document_active_structure(&editor.document);
    if (structure == NULL || structure->node_count != 3) {
        fprintf(stderr, "unexpected weighted graph state\n");
        return 1;
    }

    if (!ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[1].id, 4.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[1].id, structure->nodes[2].id, 3.0, &error) ||
        !ss_connect_selected_to(&editor, structure->nodes[0].id, structure->nodes[2].id, 10.0, &error)) {
        fprintf(stderr, "graph connection failed: %s\n", error.message);
        return 1;
    }

    if (!ss_editor_run_analysis(&editor, SS_ANALYSIS_GRAPH_DIJKSTRA, "A", &error)) {
        fprintf(stderr, "dijkstra failed: %s\n", error.message);
        return 1;
    }
    if (strstr(editor.analysis_report, "B = 4 | camino: A -> B") == NULL ||
        strstr(editor.analysis_report, "C = 7 | camino: A -> B -> C") == NULL) {
        fprintf(stderr, "unexpected dijkstra report: %s\n", editor.analysis_report);
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
