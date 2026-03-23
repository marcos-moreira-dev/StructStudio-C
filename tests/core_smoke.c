/*
 * StructStudio C
 * --------------
 * Baseline editor smoke coverage.
 *
 * This test exercises representative editor flows and also checks that the
 * automatic layout keeps semantic structures immediately drawable.
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

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_BST, &error)) {
        fprintf(stderr, "replace_active_structure failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "10", "", 10, &error)) {
        fprintf(stderr, "insert 10 failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "5", "", 5, &error)) {
        fprintf(stderr, "insert 5 failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "15", "", 15, &error)) {
        fprintf(stderr, "insert 15 failed: %s\n", error.message);
        return 1;
    }
    if (editor.document.structure_count == 0 || editor.document.structures[0].node_count != 3) {
        fprintf(stderr, "unexpected bst node count\n");
        return 1;
    }
    if (editor.document.structures[0].nodes[0].visual.width < 70.0 ||
        editor.document.structures[0].nodes[0].visual.height < 70.0 ||
        editor.document.structures[0].nodes[0].visual.y < 60.0) {
        fprintf(stderr, "bst auto-layout did not produce round positioned nodes\n");
        return 1;
    }
    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_MAP, &error)) {
        fprintf(stderr, "replace map failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_apply_primary(&editor, "usuario", "Marcos", 0, &error)) {
        fprintf(stderr, "insert map failed: %s\n", error.message);
        return 1;
    }
    if (strcmp(editor.document.structures[0].nodes[0].data.key, "usuario") != 0) {
        fprintf(stderr, "unexpected map key\n");
        return 1;
    }
    if (editor.document.structures[0].nodes[0].visual.x < 60.0 ||
        editor.document.structures[0].nodes[0].visual.y < 100.0) {
        fprintf(stderr, "map auto-layout did not position the entry\n");
        return 1;
    }
    ss_editor_set_grid_visible(&editor, 0);
    if (editor.document.view_state.show_grid != 0) {
        fprintf(stderr, "grid visibility did not update\n");
        return 1;
    }
    if (!ss_editor_clear_active_structure(&editor, &error)) {
        fprintf(stderr, "clear active structure failed: %s\n", error.message);
        return 1;
    }
    if (editor.document.structures[0].node_count != 0 || editor.document.structures[0].edge_count != 0) {
        fprintf(stderr, "active structure was not cleared\n");
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
