/*
 * StructStudio C
 * --------------
 * Smoke coverage for built-in didactic examples.
 *
 * The example loader is part of the educational UX. This test confirms that
 * representative variants can be populated without going through the GUI.
 */

#include <stdio.h>
#include <string.h>

#include "editor/editor.h"

int main(void)
{
    SsEditorState editor;
    SsError error;
    const SsStructure *structure;

    ss_editor_init(&editor);
    ss_error_clear(&error);

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_BST, &error)) {
        fprintf(stderr, "replace BST failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_load_example(&editor, &error)) {
        fprintf(stderr, "load BST example failed: %s\n", error.message);
        return 1;
    }
    structure = ss_document_active_structure_const(&editor.document);
    if (structure == NULL || structure->node_count < 7 || structure->root_id[0] == '\0') {
        fprintf(stderr, "BST example did not create the expected tree\n");
        return 1;
    }
    if (strcmp(editor.status, "Ejemplo didactico cargado para BST.") != 0) {
        fprintf(stderr, "unexpected BST example status: %s\n", editor.status);
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace weighted graph failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_load_example(&editor, &error)) {
        fprintf(stderr, "load weighted graph example failed: %s\n", error.message);
        return 1;
    }
    structure = ss_document_active_structure_const(&editor.document);
    if (structure == NULL || structure->node_count < 5 || structure->edge_count < 7) {
        fprintf(stderr, "weighted graph example did not create the expected graph\n");
        return 1;
    }
    if (!structure->config.is_weighted || structure->config.is_directed) {
        fprintf(stderr, "weighted graph example has inconsistent configuration\n");
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_MAP, &error)) {
        fprintf(stderr, "replace map failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_load_example(&editor, &error)) {
        fprintf(stderr, "load map example failed: %s\n", error.message);
        return 1;
    }
    structure = ss_document_active_structure_const(&editor.document);
    if (structure == NULL || structure->node_count != 3 || strcmp(structure->nodes[1].data.key, "nombre") != 0) {
        fprintf(stderr, "map example contents were not created as expected\n");
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
