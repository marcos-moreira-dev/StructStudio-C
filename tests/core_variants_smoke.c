/*
 * StructStudio C
 * --------------
 * Additional smoke coverage for variant-specific behavior.
 *
 * The goal is to validate a few representative domain rules after the API
 * split: priority ordering and duplicate rejection in set structures.
 */

#include <stdio.h>
#include <string.h>

#include "editor/editor.h"

int main(void)
{
    SsEditorState editor;
    SsError error;

    ss_editor_init(&editor);
    ss_error_clear(&error);

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_PRIORITY_QUEUE, &error)) {
        fprintf(stderr, "replace priority queue failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "A", "", 2, &error)) {
        fprintf(stderr, "insert A failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "B", "", 5, &error)) {
        fprintf(stderr, "insert B failed: %s\n", error.message);
        return 1;
    }
    if (editor.document.structures[0].node_count != 2 || strcmp(editor.document.structures[0].nodes[0].value, "B") != 0) {
        fprintf(stderr, "priority queue ordering failed\n");
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_SET, &error)) {
        fprintf(stderr, "replace set failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "uno", "", 0, &error)) {
        fprintf(stderr, "insert set value failed: %s\n", error.message);
        return 1;
    }
    if (editor.document.structures[0].nodes[0].visual.width < 70.0 ||
        editor.document.structures[0].nodes[0].visual.height < 70.0 ||
        editor.document.structures[0].nodes[0].visual.x < 60.0) {
        fprintf(stderr, "set node was not laid out as a round canvas entity\n");
        return 1;
    }
    ss_error_clear(&error);
    if (ss_editor_apply_primary(&editor, "uno", "", 0, &error)) {
        fprintf(stderr, "duplicate set value unexpectedly succeeded\n");
        return 1;
    }
    if (error.code != SS_ERROR_DUPLICATE) {
        fprintf(stderr, "unexpected duplicate error code: %d\n", error.code);
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_DIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace weighted graph failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "A", "", 0, &error) ||
        !ss_editor_apply_primary(&editor, "B", "", 0, &error)) {
        fprintf(stderr, "insert graph vertices failed: %s\n", error.message);
        return 1;
    }
    ss_editor_select_node(&editor, editor.document.structures[0].nodes[0].id);
    if (!ss_editor_apply_secondary(&editor, "", "B", 7, &error)) {
        fprintf(stderr, "graph connection by value token failed: %s\n", error.message);
        return 1;
    }
    if (editor.document.structures[0].edge_count != 1 ||
        strcmp(editor.document.structures[0].edges[0].target_id, editor.document.structures[0].nodes[1].id) != 0) {
        fprintf(stderr, "graph connection by value token produced wrong edge\n");
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
