/*
 * StructStudio C
 * --------------
 * Public editor contracts.
 *
 * This module coordinates user intent over the active document: selection,
 * dragging, contextual connections and high-level editing actions.
 */

#ifndef SS_EDITOR_EDITOR_H
#define SS_EDITOR_EDITOR_H

#include <stddef.h>

#include "common/error.h"
#include "core/api.h"

typedef enum SsTool {
    SS_TOOL_SELECT = 0,
    SS_TOOL_INSERT,
    SS_TOOL_CONNECT,
    SS_TOOL_DELETE
} SsTool;

/* Selection is normalized here so every UI gesture can talk about "the
 * current target" without caring whether it is a node, edge or structure. */
typedef enum SsSelectionType {
    SS_SELECTION_NONE = 0,
    SS_SELECTION_NODE,
    SS_SELECTION_EDGE,
    SS_SELECTION_STRUCTURE
} SsSelectionType;

typedef struct SsSelection {
    SsSelectionType type;
    char node_id[SS_ID_CAPACITY];
    char edge_id[SS_ID_CAPACITY];
} SsSelection;

/* Dragging stores the grabbed node plus the mouse-to-node offset so the node
 * does not jump when the drag starts. */
typedef struct SsDragState {
    int is_dragging;
    int moved;
    char node_id[SS_ID_CAPACITY];
    double offset_x;
    double offset_y;
} SsDragState;

/* Pan state works like a tiny camera controller over the canvas. */
typedef struct SsPanState {
    int is_panning;
    int moved;
    double anchor_x;
    double anchor_y;
    double origin_offset_x;
    double origin_offset_y;
} SsPanState;

/* Connection mode stores the source node and a live preview line until the
 * user clicks a valid destination or cancels the gesture. */
typedef struct SsConnectionState {
    int is_connecting;
    char source_node_id[SS_ID_CAPACITY];
    double preview_x;
    double preview_y;
    int has_preview;
} SsConnectionState;

/* Playback is the semantic timeline produced by an analysis algorithm. */
typedef struct SsAnalysisPlayback {
    SsAnalysisKind kind;
    char start_node_id[SS_ID_CAPACITY];
    SsAnalysisStep *steps;
    size_t step_count;
    size_t current_step;
    int prepared;
} SsAnalysisPlayback;

/*
 * Layout snapshots let the editor compare node positions before and after a
 * semantic mutation so the canvas can interpolate the transition instead of
 * teleporting nodes to the final layout.
 */
typedef struct SsNodeLayoutSnapshot {
    char node_id[SS_ID_CAPACITY];
    SsRect visual;
} SsNodeLayoutSnapshot;

typedef struct SsLayoutSnapshot {
    SsNodeLayoutSnapshot *nodes;
    size_t count;
} SsLayoutSnapshot;

typedef struct SsAnimatedNode {
    char node_id[SS_ID_CAPACITY];
    SsRect from_visual;
    SsRect to_visual;
} SsAnimatedNode;

typedef struct SsLayoutTransition {
    int active;
    int elapsed_ms;
    int duration_ms;
    SsAnimatedNode *nodes;
    size_t node_count;
} SsLayoutTransition;

/*
 * Playback effects are time-based UI hints layered over the analysis steps.
 * They stay outside the core algorithms so traversal logic remains reusable.
 */
typedef struct SsPlaybackFx {
    int pulse_active;
    int pulse_elapsed_ms;
    int pulse_duration_ms;
    int autoplay_enabled;
    int autoplay_elapsed_ms;
    int autoplay_interval_ms;
} SsPlaybackFx;

#define SS_ROTATION_HISTORY_CAPACITY 12

/* Rotation history is intentionally shallow and focused on teaching-oriented
 * transforms instead of a full universal undo stack for every action. */
typedef struct SsRotationHistoryEntry {
    size_t structure_index;
    SsStructure before_structure;
    SsStructure after_structure;
    char selection_before_node_id[SS_ID_CAPACITY];
    char selection_after_node_id[SS_ID_CAPACITY];
} SsRotationHistoryEntry;

typedef struct SsRotationHistory {
    SsRotationHistoryEntry undo_entries[SS_ROTATION_HISTORY_CAPACITY];
    size_t undo_count;
    SsRotationHistoryEntry redo_entries[SS_ROTATION_HISTORY_CAPACITY];
    size_t redo_count;
} SsRotationHistory;

enum {
    SS_EDITOR_ANIMATION_NONE = 0,
    SS_EDITOR_ANIMATION_REDRAW = 1 << 0,
    SS_EDITOR_ANIMATION_STATE_CHANGED = 1 << 1
};

/* SsEditorState is the bridge between the persistent document and the
 * transient interaction state of the GUI. */
typedef struct SsEditorState {
    SsDocument document;
    SsTool tool;
    SsSelection selection;
    SsDragState drag;
    SsPanState pan;
    SsConnectionState connection;
    SsAnalysisPlayback playback;
    SsLayoutTransition transition;
    SsPlaybackFx playback_fx;
    SsRotationHistory rotation_history;
    SsValidationResult validation;
    char status[SS_MESSAGE_CAPACITY];
    char analysis_report[SS_ANALYSIS_REPORT_CAPACITY];
} SsEditorState;

void ss_editor_init(SsEditorState *editor);
void ss_editor_dispose(SsEditorState *editor);

int ss_editor_new_document(SsEditorState *editor, const char *name, SsVariant variant, SsError *error);
int ss_editor_add_structure(SsEditorState *editor, SsVariant variant, SsError *error);
int ss_editor_replace_active_structure(SsEditorState *editor, SsVariant variant, SsError *error);
int ss_editor_activate_structure(SsEditorState *editor, size_t index, SsError *error);
int ss_editor_clear_active_structure(SsEditorState *editor, SsError *error);
int ss_editor_load_example(SsEditorState *editor, SsError *error);

int ss_editor_save_document(SsEditorState *editor, const char *path, SsError *error);
int ss_editor_load_document(SsEditorState *editor, const char *path, SsError *error);
int ss_editor_export_png(SsEditorState *editor, const char *path, int width, int height, SsError *error);

void ss_editor_set_tool(SsEditorState *editor, SsTool tool);
void ss_editor_set_status(SsEditorState *editor, const char *fmt, ...);
void ss_editor_clear_selection(SsEditorState *editor);
void ss_editor_clear_analysis(SsEditorState *editor);
void ss_editor_select_node(SsEditorState *editor, const char *node_id);
void ss_editor_select_edge(SsEditorState *editor, const char *edge_id);
void ss_editor_validate(SsEditorState *editor);
void ss_editor_auto_layout(SsEditorState *editor, double canvas_width);
void ss_editor_delete_selection(SsEditorState *editor, SsError *error);
void ss_editor_set_grid_visible(SsEditorState *editor, int visible);
int ss_editor_run_analysis(SsEditorState *editor, SsAnalysisKind kind, const char *start_token, SsError *error);
int ss_editor_prepare_analysis_playback(SsEditorState *editor, SsAnalysisKind kind, const char *start_token, SsError *error);
void ss_editor_reset_analysis_playback(SsEditorState *editor);
int ss_editor_analysis_playback_previous(SsEditorState *editor);
int ss_editor_analysis_playback_next(SsEditorState *editor);
int ss_editor_analysis_playback_has_steps(const SsEditorState *editor);
const SsAnalysisStep *ss_editor_current_analysis_step(const SsEditorState *editor);
int ss_editor_playback_node_state(const SsEditorState *editor, const char *node_id, int *visited, int *current);
int ss_editor_playback_edge_state(const SsEditorState *editor, const char *edge_id, int *visited, int *current);
void ss_editor_layout_snapshot_capture(const SsStructure *structure, SsLayoutSnapshot *snapshot);
void ss_editor_layout_snapshot_dispose(SsLayoutSnapshot *snapshot);
void ss_editor_begin_layout_transition(SsEditorState *editor, const SsStructure *structure, const SsLayoutSnapshot *before, int duration_ms);
void ss_editor_clear_layout_transition(SsEditorState *editor);
void ss_editor_trigger_playback_pulse(SsEditorState *editor);
void ss_editor_set_playback_autoplay(SsEditorState *editor, int enabled);
int ss_editor_playback_autoplay_enabled(const SsEditorState *editor);
int ss_editor_tick_animations(SsEditorState *editor, int elapsed_ms);
int ss_editor_has_active_animation(const SsEditorState *editor);
void ss_editor_get_node_visual(const SsEditorState *editor, const SsNode *node, SsRect *out_rect);
double ss_editor_playback_pulse_amount(const SsEditorState *editor);

int ss_editor_apply_primary(
    SsEditorState *editor,
    const char *primary,
    const char *secondary,
    int numeric_value,
    SsError *error);

int ss_editor_apply_secondary(
    SsEditorState *editor,
    const char *primary,
    const char *secondary,
    int numeric_value,
    SsError *error);

int ss_editor_apply_tertiary(
    SsEditorState *editor,
    const char *primary,
    const char *secondary,
    int numeric_value,
    SsError *error);

int ss_editor_apply_property_update(
    SsEditorState *editor,
    const char *label,
    const char *value,
    const char *secondary,
    int numeric_value,
    SsError *error);

void ss_editor_begin_pan(SsEditorState *editor, double mouse_x, double mouse_y);
void ss_editor_pan_to(SsEditorState *editor, double mouse_x, double mouse_y);
void ss_editor_end_pan(SsEditorState *editor);
void ss_editor_reset_canvas_view(SsEditorState *editor);
void ss_editor_canvas_to_world(const SsEditorState *editor, double canvas_x, double canvas_y, double *world_x, double *world_y);
void ss_editor_world_to_canvas(const SsEditorState *editor, double world_x, double world_y, double *canvas_x, double *canvas_y);
int ss_editor_rotate_selection_left(SsEditorState *editor, SsError *error);
int ss_editor_rotate_selection_right(SsEditorState *editor, SsError *error);
int ss_editor_rotate_graph_left(SsEditorState *editor, SsError *error);
int ss_editor_rotate_graph_right(SsEditorState *editor, SsError *error);
int ss_editor_can_undo_rotation(const SsEditorState *editor);
int ss_editor_can_redo_rotation(const SsEditorState *editor);
int ss_editor_undo_rotation(SsEditorState *editor, SsError *error);
int ss_editor_redo_rotation(SsEditorState *editor, SsError *error);
int ss_editor_generate_graph_tree(SsEditorState *editor, SsAnalysisKind kind, const char *start_token, SsError *error);

void ss_editor_begin_drag(SsEditorState *editor, const char *node_id, double mouse_x, double mouse_y);
void ss_editor_drag_to(SsEditorState *editor, double mouse_x, double mouse_y);
void ss_editor_end_drag(SsEditorState *editor);
int ss_editor_connect(SsEditorState *editor, const char *target_node_id, const char *relation_kind, double weight, SsError *error);
void ss_editor_update_connection_preview(SsEditorState *editor, double mouse_x, double mouse_y);
void ss_editor_cancel_connection(SsEditorState *editor);

#endif
