/*
 * StructStudio C
 * --------------
 * Smoke coverage for editor-side canvas animations.
 *
 * These checks lock two promises: semantic operations can animate layout
 * changes smoothly, and guided playback can advance automatically in time.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "editor/editor.h"

static int ss_rect_changed(const SsRect *left, const SsRect *right)
{
    const double epsilon = 0.5;

    return fabs(left->x - right->x) > epsilon ||
        fabs(left->y - right->y) > epsilon ||
        fabs(left->width - right->width) > epsilon ||
        fabs(left->height - right->height) > epsilon;
}

int main(void)
{
    SsEditorState editor;
    SsError error;
    const SsStructure *structure;
    SsRect animated_visual;
    int tick_result;
    int finish_tick_ms;
    char original_root_id[SS_ID_CAPACITY];
    char rotated_root_id[SS_ID_CAPACITY];

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
    if (structure == NULL || structure->node_count == 0) {
        fprintf(stderr, "missing BST example structure\n");
        return 1;
    }
    if (!ss_editor_has_active_animation(&editor) || !editor.transition.active) {
        fprintf(stderr, "expected active layout transition after example load\n");
        return 1;
    }

    ss_editor_get_node_visual(&editor, &structure->nodes[0], &animated_visual);
    if (!ss_rect_changed(&animated_visual, &structure->nodes[0].visual)) {
        fprintf(stderr, "animated node visual should differ from final layout during transition\n");
        return 1;
    }

    finish_tick_ms = editor.transition.duration_ms + 40;
    tick_result = ss_editor_tick_animations(&editor, finish_tick_ms);
    if ((tick_result & SS_EDITOR_ANIMATION_REDRAW) == 0 ||
        (ss_editor_has_active_animation(&editor) && editor.transition.active)) {
        fprintf(
            stderr,
            "layout transition did not finish after enough time (tick=%d duration=%d elapsed=%d active=%d pulse=%d autoplay=%d)\n",
            tick_result,
            editor.transition.duration_ms,
            editor.transition.elapsed_ms,
            editor.transition.active,
            editor.playback_fx.pulse_active,
            editor.playback_fx.autoplay_enabled);
        return 1;
    }

    ss_str_copy(original_root_id, sizeof(original_root_id), structure->root_id);
    ss_editor_select_node(&editor, original_root_id);
    if (!ss_editor_rotate_selection_left(&editor, &error)) {
        fprintf(stderr, "rotate left failed: %s\n", error.message);
        return 1;
    }
    structure = ss_document_active_structure_const(&editor.document);
    ss_str_copy(rotated_root_id, sizeof(rotated_root_id), structure->root_id);
    if (strcmp(original_root_id, rotated_root_id) == 0) {
        fprintf(stderr, "rotation should change the root node\n");
        return 1;
    }
    if (!ss_editor_can_undo_rotation(&editor)) {
        fprintf(stderr, "rotation should enable undo\n");
        return 1;
    }
    if (!ss_editor_undo_rotation(&editor, &error)) {
        fprintf(stderr, "undo rotation failed: %s\n", error.message);
        return 1;
    }
    structure = ss_document_active_structure_const(&editor.document);
    if (strcmp(structure->root_id, original_root_id) != 0) {
        fprintf(stderr, "undo rotation should restore the original root\n");
        return 1;
    }
    if (!ss_editor_can_redo_rotation(&editor)) {
        fprintf(stderr, "undo rotation should enable redo\n");
        return 1;
    }
    if (!ss_editor_redo_rotation(&editor, &error)) {
        fprintf(stderr, "redo rotation failed: %s\n", error.message);
        return 1;
    }
    structure = ss_document_active_structure_const(&editor.document);
    if (strcmp(structure->root_id, rotated_root_id) != 0) {
        fprintf(stderr, "redo rotation should restore the rotated root\n");
        return 1;
    }

    if (!ss_editor_prepare_analysis_playback(&editor, SS_ANALYSIS_TREE_PREORDER, "", &error)) {
        fprintf(stderr, "prepare preorder playback failed: %s\n", error.message);
        return 1;
    }
    if (!ss_editor_analysis_playback_has_steps(&editor) || editor.playback.step_count < 3) {
        fprintf(stderr, "unexpected preorder playback size: %zu\n", editor.playback.step_count);
        return 1;
    }

    ss_editor_set_playback_autoplay(&editor, 1);
    if (!ss_editor_playback_autoplay_enabled(&editor)) {
        fprintf(stderr, "autoplay should be enabled\n");
        return 1;
    }

    tick_result = ss_editor_tick_animations(&editor, 900);
    if ((tick_result & SS_EDITOR_ANIMATION_STATE_CHANGED) == 0) {
        fprintf(stderr, "autoplay did not advance playback state\n");
        return 1;
    }
    if (editor.playback.current_step == 0) {
        fprintf(stderr, "autoplay should have advanced to the next step\n");
        return 1;
    }

    ss_editor_dispose(&editor);
    return 0;
}
