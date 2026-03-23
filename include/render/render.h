/*
 * StructStudio C
 * --------------
 * Rendering contracts.
 *
 * The renderer receives editor snapshots and paints them. It does not decide
 * semantics such as "what insert means"; it only turns current state into
 * pixels and mirrors the same geometry for hit-testing.
 */

#ifndef SS_RENDER_RENDER_H
#define SS_RENDER_RENDER_H

#include <ui.h>

#include "editor/editor.h"

/* The theme is shared by live drawing and PNG export so screenshots and saved
 * images stay visually aligned. */
typedef struct SsTheme {
    uint32_t canvas_top;
    uint32_t canvas_bottom;
    uint32_t canvas_header;
    uint32_t canvas_header_line;
    uint32_t canvas_grid;
    uint32_t node_fill;
    uint32_t node_fill_alt;
    uint32_t node_shadow;
    uint32_t node_highlight;
    uint32_t node_border;
    uint32_t selected_border;
    uint32_t playback_fill;
    uint32_t playback_fill_current;
    uint32_t playback_border;
    uint32_t playback_border_current;
    uint32_t text_color;
    uint32_t edge_color;
    uint32_t playback_edge;
    uint32_t playback_edge_current;
    uint32_t error_color;
    uint32_t accent_color;
    uint32_t badge_fill;
    uint32_t badge_border;
    uint32_t badge_text;
} SsTheme;

typedef enum SsHitType {
    SS_HIT_NONE = 0,
    SS_HIT_NODE,
    SS_HIT_EDGE
} SsHitType;

/* Hit-testing reports what the mouse touched. The editor then decides whether
 * to select, drag, connect or delete that entity. */
typedef struct SsHitResult {
    SsHitType type;
    char id[SS_ID_CAPACITY];
} SsHitResult;

void ss_theme_init(SsTheme *theme);
void ss_render_draw(uiAreaDrawParams *params, const SsEditorState *editor, const SsTheme *theme);
void ss_render_hit_test(const SsEditorState *editor, double x, double y, SsHitResult *result);
int ss_render_export_png(const SsEditorState *editor, const SsTheme *theme, const char *path, int width, int height, SsError *error);

#endif
