/*
 * StructStudio C
 * --------------
 * Smoke coverage for contextual theory summaries.
 *
 * The theory catalog is part of the educational value of the product, so this
 * test keeps the public summary API wired to representative structures and
 * algorithms.
 */

#include <stdio.h>
#include <string.h>

#include "editor/editor.h"

int main(void)
{
    SsEditorState editor;
    SsError error;
    const SsStructure *structure;
    char theory[SS_THEORY_TEXT_CAPACITY];

    ss_editor_init(&editor);
    ss_error_clear(&error);

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_BST, &error)) {
        fprintf(stderr, "replace BST failed: %s\n", error.message);
        return 1;
    }

    structure = ss_document_active_structure_const(&editor.document);
    if (!ss_build_theory_summary(structure, SS_ANALYSIS_TREE_INORDER, theory, sizeof(theory), &error)) {
        fprintf(stderr, "BST theory failed: %s\n", error.message);
        return 1;
    }
    if (strstr(theory, "BST") == NULL ||
        strstr(theory, "Familia y variantes") == NULL ||
        strstr(theory, "AVL") == NULL ||
        strstr(theory, "Recorridos y algoritmos disponibles") == NULL ||
        strstr(theory, "Inorden") == NULL ||
        strstr(theory, "Preorden") == NULL) {
        fprintf(stderr, "unexpected BST theory summary: %s\n", theory);
        return 1;
    }

    if (!ss_editor_replace_active_structure(&editor, SS_VARIANT_DIRECTED_WEIGHTED_GRAPH, &error)) {
        fprintf(stderr, "replace weighted graph failed: %s\n", error.message);
        return 1;
    }

    structure = ss_document_active_structure_const(&editor.document);
    if (!ss_build_theory_summary(structure, SS_ANALYSIS_GRAPH_DIJKSTRA, theory, sizeof(theory), &error)) {
        fprintf(stderr, "graph theory failed: %s\n", error.message);
        return 1;
    }
    if (strstr(theory, "grafo dirigido ponderado") == NULL ||
        strstr(theory, "Variantes relacionadas") == NULL ||
        strstr(theory, "Floyd-Warshall") == NULL ||
        strstr(theory, "Dijkstra") == NULL ||
        strstr(theory, "Algoritmo destacado ahora") == NULL) {
        fprintf(stderr, "unexpected graph theory summary: %s\n", theory);
        return 1;
    }

    if (!ss_build_theory_summary(structure, SS_ANALYSIS_NONE, theory, sizeof(theory), &error)) {
        fprintf(stderr, "structure-only theory failed: %s\n", error.message);
        return 1;
    }
    if (strstr(theory, "Recorridos y algoritmos") == NULL || strstr(theory, "BFS") == NULL) {
        fprintf(stderr, "unexpected structure-only theory summary: %s\n", theory);
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
