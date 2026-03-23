/*
 * StructStudio C
 * --------------
 * Time-based editor animation helpers.
 *
 * This module keeps interpolation, playback pulses and autoplay outside the
 * semantic editor actions. The editor mutates the document once; this layer
 * only animates how those changes are presented on screen.
 */

#include "editor/editor.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

enum {
    /* Playback timings are intentionally shorter than layout transitions:
     * guided simulations should feel reactive and educational, not cinematic. */
    SS_PLAYBACK_PULSE_DURATION_MS = 220,
    SS_PLAYBACK_AUTOPLAY_INTERVAL_MS = 320,
    SS_PLAYBACK_AUTOPLAY_INITIAL_LEAD_MS = 220
};

static const SsNodeLayoutSnapshot *ss_layout_snapshot_find(const SsLayoutSnapshot *snapshot, const char *node_id)
{
    size_t index;

    if (snapshot == NULL || snapshot->nodes == NULL || node_id == NULL || node_id[0] == '\0') {
        return NULL;
    }

    for (index = 0; index < snapshot->count; ++index) {
        if (strcmp(snapshot->nodes[index].node_id, node_id) == 0) {
            return &snapshot->nodes[index];
        }
    }

    return NULL;
}

static int ss_rects_differ(const SsRect *left, const SsRect *right)
{
    const double epsilon = 0.5;

    if (left == NULL || right == NULL) {
        return 0;
    }

    return fabs(left->x - right->x) > epsilon ||
        fabs(left->y - right->y) > epsilon ||
        fabs(left->width - right->width) > epsilon ||
        fabs(left->height - right->height) > epsilon;
}

static double ss_transition_progress(const SsEditorState *editor)
{
    double t;

    /* The interpolation uses smoothstep instead of a linear ramp so nodes ease
     * in/out and feel less mechanical while still remaining deterministic. */
    if (editor == NULL || !editor->transition.active || editor->transition.duration_ms <= 0) {
        return 1.0;
    }

    t = (double) editor->transition.elapsed_ms / (double) editor->transition.duration_ms;
    t = ss_clamp_double(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

static void ss_rect_lerp(const SsRect *from_visual, const SsRect *to_visual, double t, SsRect *out_rect)
{
    if (from_visual == NULL || to_visual == NULL || out_rect == NULL) {
        return;
    }

    out_rect->x = from_visual->x + (to_visual->x - from_visual->x) * t;
    out_rect->y = from_visual->y + (to_visual->y - from_visual->y) * t;
    out_rect->width = from_visual->width + (to_visual->width - from_visual->width) * t;
    out_rect->height = from_visual->height + (to_visual->height - from_visual->height) * t;
}

static SsRect ss_entry_rect_for_node(const SsStructure *structure, const SsNode *node)
{
    SsRect rect;
    double shrink_x;
    double shrink_y;

    /* New nodes animate from a slightly displaced rectangle rather than from
     * nowhere. That gives insertions a readable "arrival" motion. */
    rect = node->visual;
    shrink_x = rect.width * 0.08;
    shrink_y = rect.height * 0.08;
    rect.x += shrink_x;
    rect.y += shrink_y;
    rect.width -= shrink_x * 2.0;
    rect.height -= shrink_y * 2.0;

    if (structure == NULL) {
        rect.y += 22.0;
        return rect;
    }

    switch (structure->family) {
        case SS_FAMILY_TREE:
        case SS_FAMILY_HEAP:
            rect.y -= 30.0;
            break;
        case SS_FAMILY_STACK:
            rect.y += 26.0;
            break;
        case SS_FAMILY_QUEUE:
        case SS_FAMILY_VECTOR:
        case SS_FAMILY_LIST:
        case SS_FAMILY_MAP:
            rect.x += 26.0;
            break;
        case SS_FAMILY_GRAPH:
        case SS_FAMILY_SET:
            rect.y -= 20.0;
            break;
        default:
            rect.y += 20.0;
            break;
    }

    return rect;
}

static const SsAnimatedNode *ss_transition_find_node(const SsEditorState *editor, const char *node_id)
{
    size_t index;

    if (editor == NULL || !editor->transition.active || editor->transition.nodes == NULL || node_id == NULL || node_id[0] == '\0') {
        return NULL;
    }

    for (index = 0; index < editor->transition.node_count; ++index) {
        if (strcmp(editor->transition.nodes[index].node_id, node_id) == 0) {
            return &editor->transition.nodes[index];
        }
    }

    return NULL;
}

void ss_editor_layout_snapshot_capture(const SsStructure *structure, SsLayoutSnapshot *snapshot)
{
    size_t index;

    /* A layout snapshot captures only geometry, not semantic data. That is all
     * the animation system needs to interpolate between two states. */
    if (snapshot == NULL) {
        return;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    if (structure == NULL || structure->node_count == 0) {
        return;
    }

    snapshot->nodes = (SsNodeLayoutSnapshot *) calloc(structure->node_count, sizeof(*snapshot->nodes));
    if (snapshot->nodes == NULL) {
        return;
    }

    snapshot->count = structure->node_count;
    for (index = 0; index < snapshot->count; ++index) {
        ss_str_copy(snapshot->nodes[index].node_id, sizeof(snapshot->nodes[index].node_id), structure->nodes[index].id);
        snapshot->nodes[index].visual = structure->nodes[index].visual;
    }
}

void ss_editor_layout_snapshot_dispose(SsLayoutSnapshot *snapshot)
{
    if (snapshot == NULL) {
        return;
    }

    free(snapshot->nodes);
    memset(snapshot, 0, sizeof(*snapshot));
}

void ss_editor_clear_layout_transition(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    free(editor->transition.nodes);
    memset(&editor->transition, 0, sizeof(editor->transition));
}

void ss_editor_begin_layout_transition(SsEditorState *editor, const SsStructure *structure, const SsLayoutSnapshot *before, int duration_ms)
{
    size_t index;
    size_t animated_count = 0;
    SsAnimatedNode *nodes;

    /* Only nodes whose geometry actually changed are added to the transition.
     * That keeps animation memory and per-frame interpolation smaller. */
    if (editor == NULL) {
        return;
    }

    ss_editor_clear_layout_transition(editor);
    if (structure == NULL || structure->node_count == 0) {
        return;
    }

    nodes = (SsAnimatedNode *) calloc(structure->node_count, sizeof(*nodes));
    if (nodes == NULL) {
        return;
    }

    for (index = 0; index < structure->node_count; ++index) {
        const SsNode *node = &structure->nodes[index];
        const SsNodeLayoutSnapshot *previous = ss_layout_snapshot_find(before, node->id);
        SsRect from_visual = previous != NULL ? previous->visual : ss_entry_rect_for_node(structure, node);

        if (previous == NULL || ss_rects_differ(&from_visual, &node->visual)) {
            ss_str_copy(nodes[animated_count].node_id, sizeof(nodes[animated_count].node_id), node->id);
            nodes[animated_count].from_visual = from_visual;
            nodes[animated_count].to_visual = node->visual;
            animated_count++;
        }
    }

    if (animated_count == 0) {
        free(nodes);
        return;
    }

    editor->transition.active = 1;
    editor->transition.elapsed_ms = 0;
    editor->transition.duration_ms = duration_ms > 0 ? duration_ms : 320;
    editor->transition.nodes = nodes;
    editor->transition.node_count = animated_count;
}

void ss_editor_trigger_playback_pulse(SsEditorState *editor)
{
    if (editor == NULL || !ss_editor_analysis_playback_has_steps(editor)) {
        return;
    }

    editor->playback_fx.pulse_active = 1;
    editor->playback_fx.pulse_elapsed_ms = 0;
    if (editor->playback_fx.pulse_duration_ms <= 0) {
        editor->playback_fx.pulse_duration_ms = SS_PLAYBACK_PULSE_DURATION_MS;
    }
}

void ss_editor_set_playback_autoplay(SsEditorState *editor, int enabled)
{
    if (editor == NULL) {
        return;
    }

    if (!enabled || !ss_editor_analysis_playback_has_steps(editor)) {
        editor->playback_fx.autoplay_enabled = 0;
        editor->playback_fx.autoplay_elapsed_ms = 0;
        return;
    }

    editor->playback_fx.autoplay_enabled = 1;
    editor->playback_fx.autoplay_elapsed_ms = 0;
    if (editor->playback_fx.autoplay_interval_ms <= 0) {
        editor->playback_fx.autoplay_interval_ms = SS_PLAYBACK_AUTOPLAY_INTERVAL_MS;
    }
    if (editor->playback_fx.autoplay_interval_ms > SS_PLAYBACK_AUTOPLAY_INITIAL_LEAD_MS) {
        editor->playback_fx.autoplay_elapsed_ms =
            editor->playback_fx.autoplay_interval_ms - SS_PLAYBACK_AUTOPLAY_INITIAL_LEAD_MS;
    }
    ss_editor_trigger_playback_pulse(editor);
}

int ss_editor_playback_autoplay_enabled(const SsEditorState *editor)
{
    return editor != NULL && editor->playback_fx.autoplay_enabled;
}

int ss_editor_tick_animations(SsEditorState *editor, int elapsed_ms)
{
    int result = SS_EDITOR_ANIMATION_NONE;

    if (editor == NULL || elapsed_ms <= 0) {
        return SS_EDITOR_ANIMATION_NONE;
    }

    if (editor->transition.active) {
        editor->transition.elapsed_ms += elapsed_ms;
        result |= SS_EDITOR_ANIMATION_REDRAW;
        if (editor->transition.elapsed_ms >= editor->transition.duration_ms) {
            ss_editor_clear_layout_transition(editor);
            result |= SS_EDITOR_ANIMATION_REDRAW;
        }
    }

    if (editor->playback_fx.pulse_active) {
        editor->playback_fx.pulse_elapsed_ms += elapsed_ms;
        result |= SS_EDITOR_ANIMATION_REDRAW;
        if (editor->playback_fx.pulse_elapsed_ms >= editor->playback_fx.pulse_duration_ms) {
            editor->playback_fx.pulse_active = 0;
            editor->playback_fx.pulse_elapsed_ms = 0;
        }
    }

    if (editor->playback_fx.autoplay_enabled) {
        editor->playback_fx.autoplay_elapsed_ms += elapsed_ms;
        while (editor->playback_fx.autoplay_elapsed_ms >= editor->playback_fx.autoplay_interval_ms) {
            editor->playback_fx.autoplay_elapsed_ms -= editor->playback_fx.autoplay_interval_ms;
            if (!ss_editor_analysis_playback_has_steps(editor) || editor->playback.current_step + 1 >= editor->playback.step_count) {
                editor->playback_fx.autoplay_enabled = 0;
                editor->playback_fx.autoplay_elapsed_ms = 0;
                ss_editor_set_status(editor, "Reproduccion automatica finalizada.");
                result |= SS_EDITOR_ANIMATION_STATE_CHANGED | SS_EDITOR_ANIMATION_REDRAW;
                break;
            }
            ++editor->playback.current_step;
            ss_editor_trigger_playback_pulse(editor);
            ss_editor_set_status(
                editor,
                "Reproduccion automatica %s (%zu/%zu): %s",
                ss_analysis_kind_label(editor->playback.kind),
                editor->playback.current_step + 1,
                editor->playback.step_count,
                editor->playback.steps[editor->playback.current_step].message);
            result |= SS_EDITOR_ANIMATION_STATE_CHANGED | SS_EDITOR_ANIMATION_REDRAW;
        }
    }

    return result;
}

int ss_editor_has_active_animation(const SsEditorState *editor)
{
    return editor != NULL &&
        (editor->transition.active ||
            editor->playback_fx.pulse_active ||
            editor->playback_fx.autoplay_enabled);
}

void ss_editor_get_node_visual(const SsEditorState *editor, const SsNode *node, SsRect *out_rect)
{
    const SsAnimatedNode *animated;
    double canvas_x;
    double canvas_y;

    if (out_rect == NULL) {
        return;
    }

    memset(out_rect, 0, sizeof(*out_rect));
    if (node == NULL) {
        return;
    }

    *out_rect = node->visual;
    animated = ss_transition_find_node(editor, node->id);
    if (animated == NULL) {
    } else {
        ss_rect_lerp(&animated->from_visual, &animated->to_visual, ss_transition_progress(editor), out_rect);
    }

    ss_editor_world_to_canvas(editor, out_rect->x, out_rect->y, &canvas_x, &canvas_y);
    out_rect->x = canvas_x;
    out_rect->y = canvas_y;
}

double ss_editor_playback_pulse_amount(const SsEditorState *editor)
{
    double phase;

    if (editor == NULL || !editor->playback_fx.pulse_active || editor->playback_fx.pulse_duration_ms <= 0) {
        return 0.0;
    }

    phase = (double) editor->playback_fx.pulse_elapsed_ms / (double) editor->playback_fx.pulse_duration_ms;
    phase = ss_clamp_double(phase, 0.0, 1.0);
    return sin(phase * 3.14159265358979323846);
}
