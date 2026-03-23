/*
 * StructStudio C
 * --------------
 * Native drawing and PNG export routines.
 *
 * The same visual rules are reused for on-screen drawing and software export
 * so the PNG output matches the canvas without relying on screenshots.
 */

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "render/render.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "stb_easy_font.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include "stb_image_write.h"

typedef struct SsSurface {
    int width;
    int height;
    unsigned char *pixels;
} SsSurface;

typedef struct SsTextVertex {
    float x;
    float y;
    float z;
    unsigned char color[4];
} SsTextVertex;

typedef struct SsPointD {
    double x;
    double y;
} SsPointD;

typedef struct SsEdgeGeometry {
    size_t point_count;
    SsPointD points[4];
    double label_x;
    double label_y;
    int show_arrow;
} SsEdgeGeometry;

static void ss_draw_line(uiDrawContext *context, double x1, double y1, double x2, double y2, uint32_t color, double thickness);
static void ss_surface_draw_line(SsSurface *surface, int x1, int y1, int x2, int y2, uint32_t color);
static void ss_surface_draw_text(SsSurface *surface, int x, int y, const char *text, uint32_t color);

static unsigned char ss_color_component(uint32_t color, int shift)
{
    return (unsigned char) ((color >> shift) & 0xFF);
}

static void ss_set_solid_brush(uiDrawBrush *brush, uint32_t color, double alpha_override)
{
    brush->Type = uiDrawBrushTypeSolid;
    brush->R = ss_color_component(color, 24) / 255.0;
    brush->G = ss_color_component(color, 16) / 255.0;
    brush->B = ss_color_component(color, 8) / 255.0;
    brush->A = alpha_override >= 0.0 ? alpha_override : ss_color_component(color, 0) / 255.0;
}

static uint32_t ss_color_with_alpha(uint32_t color, uint8_t alpha)
{
    return (color & 0xFFFFFF00u) | alpha;
}

void ss_theme_init(SsTheme *theme)
{
    theme->canvas_top = ss_color_rgba(242, 247, 252, 255);
    theme->canvas_bottom = ss_color_rgba(214, 224, 238, 255);
    theme->canvas_header = ss_color_rgba(242, 247, 252, 255);
    theme->canvas_header_line = ss_color_rgba(188, 200, 218, 210);
    theme->canvas_grid = ss_color_rgba(188, 200, 218, 255);
    theme->node_fill = ss_color_rgba(255, 255, 255, 255);
    theme->node_fill_alt = ss_color_rgba(243, 248, 255, 255);
    theme->node_shadow = ss_color_rgba(103, 117, 138, 52);
    theme->node_highlight = ss_color_rgba(255, 255, 255, 190);
    theme->node_border = ss_color_rgba(96, 112, 134, 255);
    theme->selected_border = ss_color_rgba(49, 114, 194, 255);
    theme->playback_fill = ss_color_rgba(255, 244, 203, 255);
    theme->playback_fill_current = ss_color_rgba(255, 226, 141, 255);
    theme->playback_border = ss_color_rgba(196, 142, 47, 255);
    theme->playback_border_current = ss_color_rgba(174, 104, 18, 255);
    theme->text_color = ss_color_rgba(33, 44, 58, 255);
    theme->edge_color = ss_color_rgba(74, 91, 115, 255);
    theme->playback_edge = ss_color_rgba(191, 145, 55, 255);
    theme->playback_edge_current = ss_color_rgba(171, 98, 19, 255);
    theme->error_color = ss_color_rgba(182, 58, 58, 255);
    theme->accent_color = ss_color_rgba(60, 106, 178, 255);
    theme->badge_fill = ss_color_rgba(235, 242, 252, 248);
    theme->badge_border = ss_color_rgba(126, 147, 178, 255);
    theme->badge_text = ss_color_rgba(49, 87, 145, 255);
}

static const SsStructure *ss_active_structure(const SsEditorState *editor)
{
    return ss_document_active_structure_const(&editor->document);
}

static int ss_editor_is_direct_manipulation(const SsEditorState *editor)
{
    return editor != NULL && (editor->drag.is_dragging || editor->pan.is_panning);
}

static int ss_validation_has_entity(const SsEditorState *editor, const char *entity_id)
{
    for (size_t index = 0; index < editor->validation.count; ++index) {
        if (strcmp(editor->validation.items[index].entity_id, entity_id) == 0 && editor->validation.items[index].severity == SS_VALIDATION_ERROR) {
            return 1;
        }
    }
    return 0;
}

static void ss_append_text(char *buffer, size_t capacity, const char *text)
{
    size_t length;

    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    length = strlen(buffer);
    if (length >= capacity - 1) {
        return;
    }

    ss_str_copy(buffer + length, capacity - length, text);
}

static void ss_format_node_value_text(char *buffer, size_t capacity, const SsStructure *structure, const SsNode *node)
{
    if (buffer == NULL || capacity == 0) {
        return;
    }

    buffer[0] = '\0';
    if (structure->variant == SS_VARIANT_MAP) {
        /* Build map labels incrementally to keep formatting safe and avoid
         * compiler truncation warnings for variable-length text. */
        ss_append_text(buffer, capacity, node->data.key);
        ss_append_text(buffer, capacity, " -> ");
        ss_append_text(buffer, capacity, node->value);
        return;
    }

    ss_str_copy(buffer, capacity, node->value[0] != '\0' ? node->value : node->label);
}

static int ss_structure_uses_round_nodes(const SsStructure *structure)
{
    return structure->family == SS_FAMILY_TREE ||
        structure->family == SS_FAMILY_HEAP ||
        structure->family == SS_FAMILY_GRAPH ||
        structure->family == SS_FAMILY_SET;
}

static int ss_structure_uses_map_cards(const SsStructure *structure)
{
    return structure->variant == SS_VARIANT_MAP;
}

static int ss_structure_shows_graph_ids(const SsStructure *structure)
{
    return structure->family == SS_FAMILY_GRAPH;
}

static int ss_structure_uses_list_slots(const SsStructure *structure)
{
    return structure->family == SS_FAMILY_LIST;
}

static void ss_structure_canvas_title(const SsStructure *structure, char *buffer, size_t capacity)
{
    const char *suffix = "";

    if (buffer == NULL || capacity == 0) {
        return;
    }

    buffer[0] = '\0';
    if (structure == NULL) {
        return;
    }

    if (strcmp(structure->visual_state.layout_mode, "tree_bfs") == 0) {
        suffix = " [Arbol BFS]";
    } else if (strcmp(structure->visual_state.layout_mode, "tree_dfs") == 0) {
        suffix = " [Arbol DFS]";
    } else if (strcmp(structure->visual_state.layout_mode, "tree_prim") == 0) {
        suffix = " [Arbol Prim]";
    } else if (strcmp(structure->visual_state.layout_mode, "tree_kruskal") == 0) {
        suffix = " [Arbol Kruskal]";
    }

    snprintf(buffer, capacity, "%s%s", ss_variant_descriptor(structure->variant)->display_name, suffix);
}

static int ss_structure_uses_double_slots(const SsStructure *structure)
{
    return structure->variant == SS_VARIANT_DOUBLY_LINKED_LIST ||
        structure->variant == SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST;
}

static double ss_node_left_slot_width(const SsStructure *structure)
{
    return ss_structure_uses_double_slots(structure) ? 18.0 : 0.0;
}

static double ss_node_right_slot_width(const SsStructure *structure)
{
    return ss_structure_uses_list_slots(structure) ? 18.0 : 0.0;
}

static void ss_node_content_bounds(const SsStructure *structure, const SsNode *node, SsRect *bounds)
{
    double left_slot = ss_node_left_slot_width(structure);
    double right_slot = ss_node_right_slot_width(structure);

    *bounds = node->visual;
    bounds->x += left_slot;
    bounds->width -= left_slot + right_slot;
    if (ss_structure_uses_map_cards(structure)) {
        bounds->x += 6.0;
        bounds->width -= 12.0;
    }
}

static int ss_edge_should_draw_arrow(const SsStructure *structure, const SsEdge *edge)
{
    if (structure->family == SS_FAMILY_TREE || structure->family == SS_FAMILY_HEAP) {
        return 0;
    }
    return edge->is_directed;
}

static void ss_polyline_label_position(const SsEdgeGeometry *geometry, double *x, double *y)
{
    size_t middle_segment;

    if (geometry->point_count < 2) {
        *x = 0.0;
        *y = 0.0;
        return;
    }

    middle_segment = (geometry->point_count - 1) / 2;
    *x = (geometry->points[middle_segment].x + geometry->points[middle_segment + 1].x) / 2.0;
    *y = (geometry->points[middle_segment].y + geometry->points[middle_segment + 1].y) / 2.0;
}

static double ss_grid_origin(double offset, double step)
{
    double origin;

    if (step <= 0.0) {
        return 0.0;
    }

    origin = fmod(offset, step);
    if (origin > 0.0) {
        origin -= step;
    }
    return origin;
}

static void ss_draw_text(uiDrawContext *context, const char *text, double x, double y, double width, double size, uint32_t color)
{
    uiAttributedString *string;
    uiAttribute *color_attr;
    uiAttribute *size_attr;
    uiDrawTextLayoutParams params;
    uiDrawTextLayout *layout;
    uiFontDescriptor font;

    string = uiNewAttributedString(text != NULL ? text : "");
    color_attr = uiNewColorAttribute(
        ss_color_component(color, 24) / 255.0,
        ss_color_component(color, 16) / 255.0,
        ss_color_component(color, 8) / 255.0,
        ss_color_component(color, 0) / 255.0);
    size_attr = uiNewSizeAttribute(size);
    uiAttributedStringSetAttribute(string, color_attr, 0, uiAttributedStringLen(string));
    uiAttributedStringSetAttribute(string, size_attr, 0, uiAttributedStringLen(string));

    uiLoadControlFont(&font);
    params.String = string;
    params.DefaultFont = &font;
    params.Width = width;
    params.Align = uiDrawTextAlignLeft;
    layout = uiDrawNewTextLayout(&params);
    uiDrawText(context, layout, x, y);

    uiDrawFreeTextLayout(layout);
    uiFreeFontButtonFont(&font);
    uiFreeAttributedString(string);
}

static void ss_draw_rect(uiDrawContext *context, const SsRect *rect, uint32_t fill, uint32_t border, double thickness)
{
    uiDrawPath *path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawBrush brush;
    uiDrawStrokeParams stroke_params;

    uiDrawPathAddRectangle(path, rect->x, rect->y, rect->width, rect->height);
    uiDrawPathEnd(path);
    ss_set_solid_brush(&brush, fill, -1.0);
    uiDrawFill(context, path, &brush);

    memset(&stroke_params, 0, sizeof(stroke_params));
    stroke_params.Cap = uiDrawLineCapFlat;
    stroke_params.Join = uiDrawLineJoinMiter;
    stroke_params.Thickness = thickness;
    stroke_params.MiterLimit = uiDrawDefaultMiterLimit;
    ss_set_solid_brush(&brush, border, -1.0);
    uiDrawStroke(context, path, &brush, &stroke_params);
    uiDrawFreePath(path);
}

static void ss_offset_rect(const SsRect *source, SsRect *target, double offset_x, double offset_y)
{
    *target = *source;
    target->x += offset_x;
    target->y += offset_y;
}

static void ss_draw_badge(uiDrawContext *context, const char *text, double x, double y, const SsTheme *theme)
{
    SsRect badge = { x, y, 18.0 + (double) strlen(text) * 6.6, 18.0 };

    ss_draw_rect(context, &badge, theme->badge_fill, theme->badge_border, 1.0);
    ss_draw_text(context, text, badge.x + 6.0, badge.y + 3.0, badge.width - 10.0, 10.5, theme->badge_text);
}

static double ss_badge_width(const char *text)
{
    return 18.0 + (double) strlen(text != NULL ? text : "") * 6.6;
}

static double ss_badge_centered_x(const SsRect *anchor, const char *text)
{
    if (anchor == NULL) {
        return 0.0;
    }
    return anchor->x + anchor->width / 2.0 - ss_badge_width(text) / 2.0;
}

static void ss_draw_canvas_title_panel(uiDrawContext *context, const char *title, const SsTheme *theme)
{
    double width = 200.0 + (double) strlen(title != NULL ? title : "") * 6.4;
    SsRect panel;

    width = ss_clamp_double(width, 240.0, 620.0);
    panel.x = 16.0;
    panel.y = 10.0;
    panel.width = width;
    panel.height = 46.0;
    ss_draw_rect(context, &panel, ss_color_with_alpha(theme->badge_fill, 236), theme->badge_border, 1.0);
}

static void ss_draw_polyline(uiDrawContext *context, const SsPointD *points, size_t point_count, uint32_t color, double thickness)
{
    uiDrawPath *path;
    uiDrawBrush brush;
    uiDrawStrokeParams stroke_params;

    if (point_count < 2) {
        return;
    }

    path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawPathNewFigure(path, points[0].x, points[0].y);
    for (size_t index = 1; index < point_count; ++index) {
        uiDrawPathLineTo(path, points[index].x, points[index].y);
    }
    uiDrawPathEnd(path);

    memset(&stroke_params, 0, sizeof(stroke_params));
    stroke_params.Cap = uiDrawLineCapFlat;
    stroke_params.Join = uiDrawLineJoinMiter;
    stroke_params.Thickness = thickness;
    stroke_params.MiterLimit = uiDrawDefaultMiterLimit;
    ss_set_solid_brush(&brush, color, -1.0);
    uiDrawStroke(context, path, &brush, &stroke_params);
    uiDrawFreePath(path);
}

static void ss_draw_ellipse(uiDrawContext *context, const SsRect *rect, uint32_t fill, uint32_t border, double thickness)
{
    uiDrawPath *path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawBrush brush;
    uiDrawStrokeParams stroke_params;
    double radius = rect->width < rect->height ? rect->width / 2.0 : rect->height / 2.0;

    uiDrawPathNewFigureWithArc(path,
        rect->x + rect->width / 2.0,
        rect->y + rect->height / 2.0,
        radius,
        0.0,
        6.28318530718,
        0);
    uiDrawPathEnd(path);
    ss_set_solid_brush(&brush, fill, -1.0);
    uiDrawFill(context, path, &brush);

    memset(&stroke_params, 0, sizeof(stroke_params));
    stroke_params.Cap = uiDrawLineCapFlat;
    stroke_params.Join = uiDrawLineJoinMiter;
    stroke_params.Thickness = thickness;
    stroke_params.MiterLimit = uiDrawDefaultMiterLimit;
    ss_set_solid_brush(&brush, border, -1.0);
    uiDrawStroke(context, path, &brush, &stroke_params);
    uiDrawFreePath(path);
}

static void ss_draw_list_node(uiDrawContext *context, const SsStructure *structure, const SsNode *node, uint32_t fill, uint32_t border)
{
    double left_slot = ss_node_left_slot_width(structure);
    double right_slot = ss_node_right_slot_width(structure);

    ss_draw_rect(context, &node->visual, fill, border, 2.0);
    if (left_slot > 0.0) {
        ss_draw_line(
            context,
            node->visual.x + left_slot,
            node->visual.y,
            node->visual.x + left_slot,
            node->visual.y + node->visual.height,
            border,
            1.0);
    }
    if (right_slot > 0.0) {
        ss_draw_line(
            context,
            node->visual.x + node->visual.width - right_slot,
            node->visual.y,
            node->visual.x + node->visual.width - right_slot,
            node->visual.y + node->visual.height,
            border,
            1.0);
    }
}

static void ss_draw_map_node(uiDrawContext *context, const SsNode *node, uint32_t fill, uint32_t border)
{
    double divider_x = node->visual.x + node->visual.width * 0.42;

    ss_draw_rect(context, &node->visual, fill, border, 2.0);
    ss_draw_line(context, divider_x, node->visual.y, divider_x, node->visual.y + node->visual.height, border, 1.0);
}

static void ss_draw_line(uiDrawContext *context, double x1, double y1, double x2, double y2, uint32_t color, double thickness)
{
    uiDrawPath *path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawBrush brush;
    uiDrawStrokeParams stroke_params;

    uiDrawPathNewFigure(path, x1, y1);
    uiDrawPathLineTo(path, x2, y2);
    uiDrawPathEnd(path);

    memset(&stroke_params, 0, sizeof(stroke_params));
    stroke_params.Cap = uiDrawLineCapFlat;
    stroke_params.Join = uiDrawLineJoinMiter;
    stroke_params.Thickness = thickness;
    stroke_params.MiterLimit = uiDrawDefaultMiterLimit;
    ss_set_solid_brush(&brush, color, -1.0);
    uiDrawStroke(context, path, &brush, &stroke_params);
    uiDrawFreePath(path);
}

static void ss_draw_arrow(uiDrawContext *context, double from_x, double from_y, double to_x, double to_y, uint32_t color)
{
    double angle = atan2(to_y - from_y, to_x - from_x);
    double size = 10.0;
    SsRect tip = { to_x - 1.0, to_y - 1.0, 2.0, 2.0 };

    ss_draw_line(context, to_x, to_y, to_x - cos(angle - 0.42) * size, to_y - sin(angle - 0.42) * size, color, 2.0);
    ss_draw_line(context, to_x, to_y, to_x - cos(angle + 0.42) * size, to_y - sin(angle + 0.42) * size, color, 2.0);
    ss_draw_rect(context, &tip, color, color, 1.0);
}

static void ss_draw_background(uiAreaDrawParams *params, const SsEditorState *editor, const SsTheme *theme)
{
    uiDrawPath *path;
    uiDrawBrush brush;
    double grid_step = 24.0;
    double grid_thickness = ss_editor_is_direct_manipulation(editor) ? 0.55 : 0.65;
    double grid_origin_x = ss_grid_origin(editor->document.view_state.canvas_offset_x, grid_step);
    double grid_origin_y = ss_grid_origin(editor->document.view_state.canvas_offset_y, grid_step);

    path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawPathAddRectangle(path, 0, 0, params->AreaWidth, params->AreaHeight);
    uiDrawPathEnd(path);
    ss_set_solid_brush(&brush, theme->canvas_top, -1.0);
    uiDrawFill(params->Context, path, &brush);
    uiDrawFreePath(path);

    if (editor->document.view_state.show_grid) {
        for (double x = grid_origin_x; x < params->AreaWidth; x += grid_step) {
            ss_draw_line(params->Context, x, 0, x, params->AreaHeight, theme->canvas_grid, grid_thickness);
        }
        for (double y = grid_origin_y; y < params->AreaHeight; y += grid_step) {
            ss_draw_line(params->Context, 0, y, params->AreaWidth, y, theme->canvas_grid, grid_thickness);
        }
    }
}

static void ss_node_center(const SsNode *node, double *x, double *y)
{
    *x = node->visual.x + node->visual.width / 2.0;
    *y = node->visual.y + node->visual.height / 2.0;
}

static void ss_graph_anchor_point(const SsNode *source, const SsNode *target, int is_source, double *x, double *y)
{
    double source_cx;
    double source_cy;
    double target_cx;
    double target_cy;
    double angle;
    double radius_x;
    double radius_y;
    const SsNode *node = is_source ? source : target;

    ss_node_center(source, &source_cx, &source_cy);
    ss_node_center(target, &target_cx, &target_cy);
    angle = atan2(target_cy - source_cy, target_cx - source_cx);
    if (!is_source) {
        angle += 3.14159265359;
    }

    radius_x = node->visual.width / 2.0;
    radius_y = node->visual.height / 2.0;
    *x = node->visual.x + radius_x + cos(angle) * radius_x;
    *y = node->visual.y + radius_y + sin(angle) * radius_y;
}

static int ss_is_circular_closure_edge(const SsStructure *structure, const SsNode *source, const SsNode *target, const SsEdge *edge)
{
    if (!structure->config.is_circular) {
        return 0;
    }
    if (strcmp(edge->relation_kind, "next") == 0) {
        return target->data.index_hint <= source->data.index_hint;
    }
    if (strcmp(edge->relation_kind, "prev") == 0) {
        return target->data.index_hint >= source->data.index_hint;
    }
    return 0;
}

static void ss_edge_geometry_from_nodes(const SsStructure *structure, const SsNode *source, const SsNode *target, const SsEdge *edge, SsEdgeGeometry *geometry)
{
    double x1;
    double y1;
    double x2;
    double y2;

    memset(geometry, 0, sizeof(*geometry));
    geometry->show_arrow = ss_edge_should_draw_arrow(structure, edge);

    if (structure->family == SS_FAMILY_TREE || structure->family == SS_FAMILY_HEAP) {
        x1 = source->visual.x + source->visual.width / 2.0;
        y1 = source->visual.y + source->visual.height;
        x2 = target->visual.x + target->visual.width / 2.0;
        y2 = target->visual.y;
    } else if (structure->family == SS_FAMILY_GRAPH || ss_structure_uses_round_nodes(structure)) {
        ss_graph_anchor_point(source, target, 1, &x1, &y1);
        ss_graph_anchor_point(source, target, 0, &x2, &y2);
    } else if (structure->family == SS_FAMILY_LIST) {
        double offset = ss_structure_uses_double_slots(structure) ? (strcmp(edge->relation_kind, "prev") == 0 ? 10.0 : -10.0) : 0.0;
        if (strcmp(edge->relation_kind, "prev") == 0) {
            x1 = source->visual.x;
            y1 = source->visual.y + source->visual.height / 2.0 + offset;
            x2 = target->visual.x + target->visual.width;
            y2 = target->visual.y + target->visual.height / 2.0 + offset;
        } else {
            x1 = source->visual.x + source->visual.width;
            y1 = source->visual.y + source->visual.height / 2.0 + offset;
            x2 = target->visual.x;
            y2 = target->visual.y + target->visual.height / 2.0 + offset;
        }
    } else if (structure->family == SS_FAMILY_QUEUE) {
        x1 = source->visual.x + source->visual.width;
        y1 = source->visual.y + source->visual.height / 2.0;
        x2 = target->visual.x;
        y2 = target->visual.y + target->visual.height / 2.0;
    } else if (structure->family == SS_FAMILY_STACK) {
        x1 = source->visual.x + source->visual.width / 2.0;
        y1 = source->visual.y;
        x2 = target->visual.x + target->visual.width / 2.0;
        y2 = target->visual.y + target->visual.height;
    } else {
        ss_node_center(source, &x1, &y1);
        ss_node_center(target, &x2, &y2);
    }

    if (ss_is_circular_closure_edge(structure, source, target, edge)) {
        double lane_y = strcmp(edge->relation_kind, "prev") == 0
            ? fmax(source->visual.y + source->visual.height, target->visual.y + target->visual.height) + 34.0
            : fmin(source->visual.y, target->visual.y) - 34.0;
        geometry->point_count = 4;
        geometry->points[0].x = x1;
        geometry->points[0].y = y1;
        geometry->points[1].x = x1;
        geometry->points[1].y = lane_y;
        geometry->points[2].x = x2;
        geometry->points[2].y = lane_y;
        geometry->points[3].x = x2;
        geometry->points[3].y = y2;
    } else {
        geometry->point_count = 2;
        geometry->points[0].x = x1;
        geometry->points[0].y = y1;
        geometry->points[1].x = x2;
        geometry->points[1].y = y2;
    }

    ss_polyline_label_position(geometry, &geometry->label_x, &geometry->label_y);
}

static void ss_draw_edges(uiAreaDrawParams *params, const SsEditorState *editor, const SsStructure *structure, const SsTheme *theme)
{
    int direct_manipulation = ss_editor_is_direct_manipulation(editor);
    int playback_autoplay = ss_editor_playback_autoplay_enabled(editor);

    for (size_t index = 0; index < structure->edge_count; ++index) {
        const SsEdge *edge = &structure->edges[index];
        const SsNode *source = ss_structure_find_node_const(structure, edge->source_id);
        const SsNode *target = ss_structure_find_node_const(structure, edge->target_id);
        SsNode source_visual;
        SsNode target_visual;
        SsEdgeGeometry geometry;
        uint32_t color;
        double thickness = 2.0;
        double pulse = 0.0;
        int playback_visited = 0;
        int playback_current = 0;

        if (source == NULL || target == NULL) {
            continue;
        }

        source_visual = *source;
        target_visual = *target;
        ss_editor_get_node_visual(editor, source, &source_visual.visual);
        ss_editor_get_node_visual(editor, target, &target_visual.visual);
        ss_edge_geometry_from_nodes(structure, &source_visual, &target_visual, edge, &geometry);
        color = theme->edge_color;
        ss_editor_playback_edge_state(editor, edge->id, &playback_visited, &playback_current);
        if (ss_validation_has_entity(editor, edge->id)) {
            color = theme->error_color;
        } else if (playback_current) {
            color = theme->playback_edge_current;
            pulse = ss_editor_playback_pulse_amount(editor);
            thickness = 2.4 + pulse * 1.6;
        } else if (editor->selection.type == SS_SELECTION_EDGE && strcmp(editor->selection.edge_id, edge->id) == 0) {
            color = theme->selected_border;
        } else if (playback_visited) {
            color = theme->playback_edge;
        }
        ss_draw_polyline(params->Context, geometry.points, geometry.point_count, color, thickness);
        if (geometry.show_arrow && geometry.point_count >= 2) {
            size_t last = geometry.point_count - 1;
            ss_draw_arrow(
                params->Context,
                geometry.points[last - 1].x,
                geometry.points[last - 1].y,
                geometry.points[last].x,
                geometry.points[last].y,
                color);
        }
        if (!direct_manipulation && !playback_autoplay && edge->has_weight && edge->visual.show_weight) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.0f", edge->weight);
            ss_draw_badge(params->Context, buffer, geometry.label_x + 4.0, geometry.label_y - 22.0, theme);
        }
    }
}

static void ss_draw_connection_preview(uiAreaDrawParams *params, const SsEditorState *editor, const SsStructure *structure, const SsTheme *theme)
{
    const SsNode *source;
    SsNode source_visual;

    if (!editor->connection.is_connecting || !editor->connection.has_preview) {
        return;
    }

    source = ss_structure_find_node_const(structure, editor->connection.source_node_id);
    if (source == NULL) {
        return;
    }

    source_visual = *source;
    ss_editor_get_node_visual(editor, source, &source_visual.visual);
    {
        SsPointD points[2];
        points[0].x = source_visual.visual.x + source_visual.visual.width / 2.0;
        points[0].y = source_visual.visual.y + source_visual.visual.height / 2.0;
        points[1].x = editor->connection.preview_x;
        points[1].y = editor->connection.preview_y;
        ss_draw_polyline(params->Context, points, 2, theme->accent_color, 1.5);
        if (structure->family != SS_FAMILY_TREE && structure->family != SS_FAMILY_HEAP) {
            ss_draw_arrow(params->Context, points[0].x, points[0].y, points[1].x, points[1].y, theme->accent_color);
        }
    }
}

static void ss_draw_node_marker(uiAreaDrawParams *params, const char *text, double x, double y, const SsTheme *theme)
{
    ss_draw_badge(params->Context, text, x, y, theme);
}

static void ss_format_graph_node_id_badge(const SsNode *node, char *buffer, size_t capacity)
{
    if (buffer == NULL || capacity == 0) {
        return;
    }

    snprintf(buffer, capacity, "%s", node->id);
}

static void ss_draw_nodes(uiAreaDrawParams *params, const SsEditorState *editor, const SsStructure *structure, const SsTheme *theme)
{
    int direct_manipulation = ss_editor_is_direct_manipulation(editor);
    int playback_autoplay = ss_editor_playback_autoplay_enabled(editor);
    int reduce_detail = (direct_manipulation && structure->node_count > 96) || playback_autoplay;

    for (size_t index = 0; index < structure->node_count; ++index) {
        const SsNode *node = &structure->nodes[index];
        SsNode display_node = *node;
        SsNode shadow_node;
        uint32_t border = theme->node_border;
        uint32_t fill = index % 2 == 0 ? theme->node_fill : theme->node_fill_alt;
        SsRect content_bounds;
        SsRect shadow_rect;
        SsRect highlight_rect;
        double pulse = 0.0;
        int playback_visited = 0;
        int playback_current = 0;

        ss_editor_get_node_visual(editor, node, &display_node.visual);
        shadow_node = display_node;
        ss_editor_playback_node_state(editor, node->id, &playback_visited, &playback_current);
        if (playback_current) {
            fill = theme->playback_fill_current;
            border = theme->playback_border_current;
            pulse = ss_editor_playback_pulse_amount(editor);
        } else if (playback_visited) {
            fill = theme->playback_fill;
            border = theme->playback_border;
        }
        if (editor->selection.type == SS_SELECTION_NODE && strcmp(editor->selection.node_id, node->id) == 0 && !playback_current) {
            border = theme->selected_border;
        }
        if (ss_validation_has_entity(editor, node->id)) {
            border = theme->error_color;
        }

        if (!reduce_detail && playback_current && pulse > 0.0) {
            SsRect halo_rect = display_node.visual;
            halo_rect.x -= 6.0 + pulse * 4.0;
            halo_rect.y -= 6.0 + pulse * 4.0;
            halo_rect.width += 12.0 + pulse * 8.0;
            halo_rect.height += 12.0 + pulse * 8.0;
            if (ss_structure_uses_round_nodes(structure)) {
                ss_draw_ellipse(
                    params->Context,
                    &halo_rect,
                    ss_color_with_alpha(theme->playback_fill_current, (uint8_t) (34 + pulse * 56.0)),
                    ss_color_with_alpha(theme->playback_border_current, (uint8_t) (72 + pulse * 72.0)),
                    1.0 + pulse);
            } else {
                ss_draw_rect(
                    params->Context,
                    &halo_rect,
                    ss_color_with_alpha(theme->playback_fill_current, (uint8_t) (30 + pulse * 50.0)),
                    ss_color_with_alpha(theme->playback_border_current, (uint8_t) (72 + pulse * 72.0)),
                    1.0 + pulse);
            }
        }

        ss_offset_rect(&display_node.visual, &shadow_rect, 4.0, 4.0);
        shadow_node.visual = shadow_rect;
        if (!reduce_detail) {
            if (ss_structure_uses_map_cards(structure)) {
                ss_draw_map_node(params->Context, &shadow_node, theme->node_shadow, theme->node_shadow);
            } else if (ss_structure_uses_list_slots(structure)) {
                ss_draw_list_node(params->Context, structure, &shadow_node, theme->node_shadow, theme->node_shadow);
            } else if (ss_structure_uses_round_nodes(structure)) {
                ss_draw_ellipse(params->Context, &shadow_rect, theme->node_shadow, theme->node_shadow, 1.0);
            } else {
                ss_draw_rect(params->Context, &shadow_rect, theme->node_shadow, theme->node_shadow, 1.0);
            }
        }

        if (ss_structure_uses_map_cards(structure)) {
            ss_draw_map_node(params->Context, &display_node, fill, border);
        } else if (ss_structure_uses_list_slots(structure)) {
            ss_draw_list_node(params->Context, structure, &display_node, fill, border);
        } else if (ss_structure_uses_round_nodes(structure)) {
            ss_draw_ellipse(params->Context, &display_node.visual, fill, border, playback_current ? 2.2 + pulse * 1.2 : 2.0);
        } else {
            ss_draw_rect(params->Context, &display_node.visual, fill, border, playback_current ? 2.2 + pulse * 1.2 : 2.0);
        }

        highlight_rect = display_node.visual;
        highlight_rect.y += 4.0;
        highlight_rect.x += 4.0;
        highlight_rect.width -= 8.0;
        highlight_rect.height = ss_structure_uses_round_nodes(structure) ? 16.0 : 10.0;
        if (!reduce_detail && highlight_rect.width > 0.0 && highlight_rect.height > 0.0) {
            if (ss_structure_uses_round_nodes(structure)) {
                ss_draw_ellipse(params->Context, &highlight_rect, theme->node_highlight, theme->node_highlight, 0.5);
            } else {
                ss_draw_rect(params->Context, &highlight_rect, theme->node_highlight, theme->node_highlight, 0.5);
            }
        }

        ss_node_content_bounds(structure, &display_node, &content_bounds);
        if (ss_structure_uses_map_cards(structure)) {
            double divider_x = display_node.visual.x + display_node.visual.width * 0.42;
            ss_draw_text(params->Context, node->data.key, display_node.visual.x + 8.0, display_node.visual.y + 12.0, divider_x - display_node.visual.x - 12.0, 11.75, theme->accent_color);
            ss_draw_text(params->Context, node->value[0] != '\0' ? node->value : node->label, divider_x + 8.0, display_node.visual.y + 12.0, display_node.visual.x + display_node.visual.width - divider_x - 12.0, 12.25, theme->text_color);
        } else {
            char value_text[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];
            ss_format_node_value_text(value_text, sizeof(value_text), structure, node);
            ss_draw_text(params->Context, value_text, content_bounds.x + 8.0, content_bounds.y + 12.0, content_bounds.width - 12.0, 12.25, theme->text_color);
        }

        if (!reduce_detail && structure->family == SS_FAMILY_VECTOR) {
            char idx[16];
            snprintf(idx, sizeof(idx), "[%d]", node->data.index_hint);
            ss_draw_node_marker(params, idx, display_node.visual.x + 6.0, display_node.visual.y + display_node.visual.height + 6.0, theme);
        }
        if (!reduce_detail && structure->family == SS_FAMILY_STACK && index == structure->node_count - 1) {
            ss_draw_node_marker(params, "TOP", display_node.visual.x + display_node.visual.width + 8.0, display_node.visual.y + 12.0, theme);
        }
        if (!reduce_detail && structure->family == SS_FAMILY_QUEUE && index == 0) {
            ss_draw_node_marker(params, "FRONT", display_node.visual.x, display_node.visual.y - 18.0, theme);
        }
        if (!reduce_detail && structure->family == SS_FAMILY_QUEUE && index == structure->node_count - 1) {
            ss_draw_node_marker(params, "REAR", display_node.visual.x, display_node.visual.y + display_node.visual.height + 6.0, theme);
        }
        if (!reduce_detail &&
            (structure->family == SS_FAMILY_TREE || structure->family == SS_FAMILY_HEAP) &&
            strcmp(structure->root_id, node->id) == 0) {
            ss_draw_node_marker(
                params,
                "ROOT",
                ss_badge_centered_x(&display_node.visual, "ROOT"),
                display_node.visual.y - 24.0,
                theme);
        }
        if (!reduce_detail && !playback_autoplay && ss_structure_shows_graph_ids(structure)) {
            char badge[SS_ID_CAPACITY];

            ss_format_graph_node_id_badge(node, badge, sizeof(badge));
            ss_draw_node_marker(
                params,
                badge,
                display_node.visual.x + display_node.visual.width / 2.0 - (18.0 + (double) strlen(badge) * 6.6) / 2.0,
                display_node.visual.y + display_node.visual.height + 6.0,
                theme);
        }
    }
}

void ss_render_draw(uiAreaDrawParams *params, const SsEditorState *editor, const SsTheme *theme)
{
    const SsStructure *structure = ss_active_structure(editor);

    ss_draw_background(params, editor, theme);
    if (structure == NULL) {
        ss_draw_text(params->Context, "Sin estructura activa", 36.0, 36.0, 220.0, 12.0, theme->text_color);
        return;
    }

    ss_draw_edges(params, editor, structure, theme);
    ss_draw_connection_preview(params, editor, structure, theme);
    ss_draw_nodes(params, editor, structure, theme);
    {
        char canvas_title[128];
        ss_structure_canvas_title(structure, canvas_title, sizeof(canvas_title));
        ss_draw_canvas_title_panel(params->Context, canvas_title, theme);
        ss_draw_text(params->Context, canvas_title, 24.0, 16.0, 460.0, 16.5, theme->accent_color);
    }
    ss_draw_text(params->Context, "Canvas interactivo", 24.0, 38.0, 240.0, 10.25, theme->badge_text);
    if (ss_editor_analysis_playback_has_steps(editor)) {
        char badge[64];
        snprintf(badge, sizeof(badge), "PASO %zu/%zu", editor->playback.current_step + 1, editor->playback.step_count);
        ss_draw_badge(params->Context, badge, 236.0, 18.0, theme);
        if (ss_editor_playback_autoplay_enabled(editor)) {
            ss_draw_badge(params->Context, "AUTO", 334.0, 18.0, theme);
        }
    }
}

static int ss_point_in_rect(double x, double y, const SsRect *rect)
{
    return x >= rect->x && x <= rect->x + rect->width && y >= rect->y && y <= rect->y + rect->height;
}

static int ss_point_in_round_node(double x, double y, const SsRect *rect)
{
    double radius_x = rect->width / 2.0;
    double radius_y = rect->height / 2.0;
    double center_x = rect->x + radius_x;
    double center_y = rect->y + radius_y;
    double dx;
    double dy;

    if (radius_x <= 0.0 || radius_y <= 0.0) {
        return 0;
    }

    dx = (x - center_x) / radius_x;
    dy = (y - center_y) / radius_y;
    return dx * dx + dy * dy <= 1.0;
}

static double ss_distance_to_segment(double px, double py, double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double length_sq = dx * dx + dy * dy;
    double t;
    double proj_x;
    double proj_y;

    if (length_sq == 0.0) {
        return hypot(px - x1, py - y1);
    }

    t = ((px - x1) * dx + (py - y1) * dy) / length_sq;
    t = ss_clamp_double(t, 0.0, 1.0);
    proj_x = x1 + t * dx;
    proj_y = y1 + t * dy;
    return hypot(px - proj_x, py - proj_y);
}

void ss_render_hit_test(const SsEditorState *editor, double x, double y, SsHitResult *result)
{
    const SsStructure *structure = ss_active_structure(editor);

    memset(result, 0, sizeof(*result));
    if (structure == NULL) {
        return;
    }

    for (size_t index = structure->node_count; index-- > 0;) {
        SsRect visual;
        int hit_node;

        ss_editor_get_node_visual(editor, &structure->nodes[index], &visual);
        hit_node = ss_structure_uses_round_nodes(structure)
            ? ss_point_in_round_node(x, y, &visual)
            : ss_point_in_rect(x, y, &visual);
        if (hit_node) {
            result->type = SS_HIT_NODE;
            ss_str_copy(result->id, sizeof(result->id), structure->nodes[index].id);
            return;
        }
    }

    for (size_t index = 0; index < structure->edge_count; ++index) {
        const SsEdge *edge = &structure->edges[index];
        const SsNode *source = ss_structure_find_node_const(structure, edge->source_id);
        const SsNode *target = ss_structure_find_node_const(structure, edge->target_id);
        SsNode source_visual;
        SsNode target_visual;
        SsEdgeGeometry geometry;

        if (source == NULL || target == NULL) {
            continue;
        }

        source_visual = *source;
        target_visual = *target;
        ss_editor_get_node_visual(editor, source, &source_visual.visual);
        ss_editor_get_node_visual(editor, target, &target_visual.visual);
        ss_edge_geometry_from_nodes(structure, &source_visual, &target_visual, edge, &geometry);
        for (size_t segment = 1; segment < geometry.point_count; ++segment) {
            if (ss_distance_to_segment(
                    x,
                    y,
                    geometry.points[segment - 1].x,
                    geometry.points[segment - 1].y,
                    geometry.points[segment].x,
                    geometry.points[segment].y) < 6.0) {
                result->type = SS_HIT_EDGE;
                ss_str_copy(result->id, sizeof(result->id), edge->id);
                return;
            }
        }
    }
}

static void ss_surface_set_pixel(SsSurface *surface, int x, int y, uint32_t color)
{
    unsigned char *pixel;

    if (x < 0 || y < 0 || x >= surface->width || y >= surface->height) {
        return;
    }
    pixel = &surface->pixels[(y * surface->width + x) * 4];
    pixel[0] = ss_color_component(color, 24);
    pixel[1] = ss_color_component(color, 16);
    pixel[2] = ss_color_component(color, 8);
    pixel[3] = ss_color_component(color, 0);
}

static void ss_surface_fill_rect(SsSurface *surface, int x, int y, int width, int height, uint32_t color)
{
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            ss_surface_set_pixel(surface, column, row, color);
        }
    }
}

static void ss_surface_draw_rect(SsSurface *surface, int x, int y, int width, int height, uint32_t fill, uint32_t border)
{
    ss_surface_fill_rect(surface, x, y, width, height, fill);
    for (int column = x; column < x + width; ++column) {
        ss_surface_set_pixel(surface, column, y, border);
        ss_surface_set_pixel(surface, column, y + height - 1, border);
    }
    for (int row = y; row < y + height; ++row) {
        ss_surface_set_pixel(surface, x, row, border);
        ss_surface_set_pixel(surface, x + width - 1, row, border);
    }
}

static void ss_surface_draw_badge(SsSurface *surface, int x, int y, const char *text, const SsTheme *theme)
{
    int width = 18 + (int) strlen(text) * 6;

    ss_surface_draw_rect(surface, x, y, width, 18, theme->badge_fill, theme->badge_border);
    ss_surface_draw_text(surface, x + 5, y + 4, text, theme->badge_text);
}

static void ss_surface_draw_canvas_title_panel(SsSurface *surface, const char *title, const SsTheme *theme)
{
    int width = 200 + (int) strlen(title != NULL ? title : "") * 6;

    if (width < 240) {
        width = 240;
    }
    if (width > 620) {
        width = 620;
    }
    ss_surface_draw_rect(surface, 16, 12, width, 46, theme->badge_fill, theme->badge_border);
}

static void ss_surface_draw_polyline(SsSurface *surface, const SsPointD *points, size_t point_count, uint32_t color)
{
    if (point_count < 2) {
        return;
    }

    for (size_t index = 1; index < point_count; ++index) {
        ss_surface_draw_line(
            surface,
            (int) points[index - 1].x,
            (int) points[index - 1].y,
            (int) points[index].x,
            (int) points[index].y,
            color);
    }
}

static void ss_surface_draw_line(SsSurface *surface, int x1, int y1, int x2, int y2, uint32_t color)
{
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        ss_surface_set_pixel(surface, x1, y1, color);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        {
            int e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x1 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y1 += sy;
            }
        }
    }
}

static void ss_surface_draw_arrow(SsSurface *surface, int from_x, int from_y, int to_x, int to_y, uint32_t color)
{
    double angle = atan2((double) (to_y - from_y), (double) (to_x - from_x));
    double size = 9.0;

    ss_surface_draw_line(
        surface,
        to_x,
        to_y,
        (int) (to_x - cos(angle - 0.42) * size),
        (int) (to_y - sin(angle - 0.42) * size),
        color);
    ss_surface_draw_line(
        surface,
        to_x,
        to_y,
        (int) (to_x - cos(angle + 0.42) * size),
        (int) (to_y - sin(angle + 0.42) * size),
        color);
}

static void ss_surface_draw_circle(SsSurface *surface, int cx, int cy, int radius, uint32_t fill, uint32_t border)
{
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            int distance_sq = x * x + y * y;
            if (distance_sq <= radius * radius) {
                ss_surface_set_pixel(surface, cx + x, cy + y, fill);
            }
            if (distance_sq >= (radius - 1) * (radius - 1) && distance_sq <= radius * radius) {
                ss_surface_set_pixel(surface, cx + x, cy + y, border);
            }
        }
    }
}

static void ss_surface_draw_list_node(SsSurface *surface, const SsStructure *structure, const SsNode *node, uint32_t fill, uint32_t border)
{
    int x = (int) node->visual.x;
    int y = (int) node->visual.y;
    int width = (int) node->visual.width;
    int height = (int) node->visual.height;
    int left_slot = (int) ss_node_left_slot_width(structure);
    int right_slot = (int) ss_node_right_slot_width(structure);

    ss_surface_draw_rect(surface, x, y, width, height, fill, border);
    if (left_slot > 0) {
        ss_surface_draw_line(surface, x + left_slot, y, x + left_slot, y + height, border);
    }
    if (right_slot > 0) {
        ss_surface_draw_line(surface, x + width - right_slot, y, x + width - right_slot, y + height, border);
    }
}

static void ss_surface_draw_map_node(SsSurface *surface, const SsNode *node, uint32_t fill, uint32_t border)
{
    int x = (int) node->visual.x;
    int y = (int) node->visual.y;
    int width = (int) node->visual.width;
    int height = (int) node->visual.height;
    int divider_x = x + (int) (node->visual.width * 0.42);

    ss_surface_draw_rect(surface, x, y, width, height, fill, border);
    ss_surface_draw_line(surface, divider_x, y, divider_x, y + height, border);
}

static void ss_surface_draw_text(SsSurface *surface, int x, int y, const char *text, uint32_t color)
{
    char buffer[8192];
    int quads;
    SsTextVertex *vertices = (SsTextVertex *) buffer;

    quads = stb_easy_font_print((float) x, (float) y, (char *) (text != NULL ? text : ""), NULL, buffer, sizeof(buffer));
    for (int quad = 0; quad < quads; ++quad) {
        SsTextVertex *v = &vertices[quad * 4];
        int min_x = (int) v[0].x;
        int min_y = (int) v[0].y;
        int max_x = (int) v[2].x;
        int max_y = (int) v[2].y;
        ss_surface_fill_rect(surface, min_x, min_y, max_x - min_x, max_y - min_y, color);
    }
}

int ss_render_export_png(const SsEditorState *editor, const SsTheme *theme, const char *path, int width, int height, SsError *error)
{
    const SsStructure *structure = ss_active_structure(editor);
    SsSurface surface;

    if (path == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Ruta PNG invalida.");
        return 0;
    }

    surface.width = width;
    surface.height = height;
    surface.pixels = (unsigned char *) calloc((size_t) width * (size_t) height * 4, 1);
    if (surface.pixels == NULL) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para el PNG.");
        return 0;
    }

    ss_surface_fill_rect(&surface, 0, 0, width, height, theme->canvas_top);

    if (editor->document.view_state.show_grid) {
        for (int x = 0; x < width; x += 24) {
            ss_surface_draw_line(&surface, x, 0, x, height - 1, theme->canvas_grid);
        }
        for (int y = 0; y < height; y += 24) {
            ss_surface_draw_line(&surface, 0, y, width - 1, y, theme->canvas_grid);
        }
    }

    if (structure != NULL) {
        for (size_t index = 0; index < structure->edge_count; ++index) {
            const SsEdge *edge = &structure->edges[index];
            const SsNode *source = ss_structure_find_node_const(structure, edge->source_id);
            const SsNode *target = ss_structure_find_node_const(structure, edge->target_id);
            if (source != NULL && target != NULL) {
                SsEdgeGeometry geometry;
                uint32_t color = theme->edge_color;
                int playback_visited = 0;
                int playback_current = 0;

                ss_edge_geometry_from_nodes(structure, source, target, edge, &geometry);
                ss_editor_playback_edge_state(editor, edge->id, &playback_visited, &playback_current);
                if (ss_validation_has_entity(editor, edge->id)) {
                    color = theme->error_color;
                } else if (playback_current) {
                    color = theme->playback_edge_current;
                } else if (editor->selection.type == SS_SELECTION_EDGE && strcmp(editor->selection.edge_id, edge->id) == 0) {
                    color = theme->selected_border;
                } else if (playback_visited) {
                    color = theme->playback_edge;
                }
                ss_surface_draw_polyline(&surface, geometry.points, geometry.point_count, color);
                if (geometry.show_arrow && geometry.point_count >= 2) {
                    size_t last = geometry.point_count - 1;
                    ss_surface_draw_arrow(
                        &surface,
                        (int) geometry.points[last - 1].x,
                        (int) geometry.points[last - 1].y,
                        (int) geometry.points[last].x,
                        (int) geometry.points[last].y,
                        color);
                }
                if (edge->has_weight) {
                    char buffer[24];
                    snprintf(buffer, sizeof(buffer), "%.0f", edge->weight);
                    ss_surface_draw_badge(&surface, (int) geometry.label_x + 4, (int) geometry.label_y - 22, buffer, theme);
                }
            }
        }

        for (size_t index = 0; index < structure->node_count; ++index) {
            const SsNode *node = &structure->nodes[index];
            SsNode shadow_node = *node;
            uint32_t border = theme->node_border;
            uint32_t fill = index % 2 == 0 ? theme->node_fill : theme->node_fill_alt;
            int shadow_x = (int) node->visual.x + 4;
            int shadow_y = (int) node->visual.y + 4;
            int playback_visited = 0;
            int playback_current = 0;

            ss_editor_playback_node_state(editor, node->id, &playback_visited, &playback_current);
            if (playback_current) {
                fill = theme->playback_fill_current;
                border = theme->playback_border_current;
            } else if (playback_visited) {
                fill = theme->playback_fill;
                border = theme->playback_border;
            }
            if (editor->selection.type == SS_SELECTION_NODE && strcmp(editor->selection.node_id, node->id) == 0 && !playback_current) {
                border = theme->selected_border;
            }
            if (ss_validation_has_entity(editor, node->id)) {
                border = theme->error_color;
            }
            shadow_node.visual.x += 4.0;
            shadow_node.visual.y += 4.0;
            if (ss_structure_uses_map_cards(structure)) {
                ss_surface_draw_map_node(&surface, &shadow_node, theme->node_shadow, theme->node_shadow);
                ss_surface_draw_map_node(&surface, node, fill, border);
            } else if (ss_structure_uses_list_slots(structure)) {
                ss_surface_draw_list_node(&surface, structure, &shadow_node, theme->node_shadow, theme->node_shadow);
                ss_surface_draw_list_node(&surface, structure, node, fill, border);
            } else if (ss_structure_uses_round_nodes(structure)) {
                int radius = (int) ((node->visual.width < node->visual.height ? node->visual.width : node->visual.height) / 2.0);
                ss_surface_draw_circle(&surface, shadow_x + radius, shadow_y + radius, radius, theme->node_shadow, theme->node_shadow);
                ss_surface_draw_circle(&surface, (int) (node->visual.x + node->visual.width / 2.0), (int) (node->visual.y + node->visual.height / 2.0), radius, fill, border);
            } else {
                ss_surface_draw_rect(&surface, shadow_x, shadow_y, (int) node->visual.width, (int) node->visual.height, theme->node_shadow, theme->node_shadow);
                ss_surface_draw_rect(&surface, (int) node->visual.x, (int) node->visual.y, (int) node->visual.width, (int) node->visual.height, fill, border);
            }

            if (ss_structure_uses_map_cards(structure)) {
                int divider_x = (int) (node->visual.x + node->visual.width * 0.42);
                ss_surface_draw_text(&surface, (int) node->visual.x + 6, (int) node->visual.y + 14, node->data.key, theme->accent_color);
                ss_surface_draw_text(&surface, divider_x + 6, (int) node->visual.y + 14, node->value[0] != '\0' ? node->value : node->label, theme->text_color);
            } else {
                SsRect content_bounds;
                char buffer[SS_VALUE_CAPACITY + SS_LABEL_CAPACITY];
                ss_node_content_bounds(structure, node, &content_bounds);
                ss_format_node_value_text(buffer, sizeof(buffer), structure, node);
                ss_surface_draw_text(&surface, (int) content_bounds.x + 6, (int) content_bounds.y + 14, buffer, theme->text_color);
            }

            if (structure->family == SS_FAMILY_VECTOR) {
                char idx[16];
                snprintf(idx, sizeof(idx), "[%d]", node->data.index_hint);
                ss_surface_draw_badge(&surface, (int) node->visual.x + 6, (int) node->visual.y + (int) node->visual.height + 6, idx, theme);
            }
            if (structure->family == SS_FAMILY_STACK && index == structure->node_count - 1) {
                ss_surface_draw_badge(&surface, (int) node->visual.x + (int) node->visual.width + 8, (int) node->visual.y + 12, "TOP", theme);
            }
            if (structure->family == SS_FAMILY_QUEUE && index == 0) {
                ss_surface_draw_badge(&surface, (int) node->visual.x, (int) node->visual.y - 18, "FRONT", theme);
            }
            if (structure->family == SS_FAMILY_QUEUE && index == structure->node_count - 1) {
                ss_surface_draw_badge(&surface, (int) node->visual.x, (int) node->visual.y + (int) node->visual.height + 6, "REAR", theme);
            }
            if ((structure->family == SS_FAMILY_TREE || structure->family == SS_FAMILY_HEAP) && strcmp(structure->root_id, node->id) == 0) {
                ss_surface_draw_badge(
                    &surface,
                    (int) ss_badge_centered_x(&node->visual, "ROOT"),
                    (int) node->visual.y - 24,
                    "ROOT",
                    theme);
            }
            if (ss_structure_shows_graph_ids(structure)) {
                char badge[SS_ID_CAPACITY];
                int badge_width;

                ss_format_graph_node_id_badge(node, badge, sizeof(badge));
                badge_width = 18 + (int) strlen(badge) * 6;
                ss_surface_draw_badge(
                    &surface,
                    (int) (node->visual.x + node->visual.width / 2.0) - badge_width / 2,
                    (int) node->visual.y + (int) node->visual.height + 6,
                    badge,
                    theme);
            }
        }
    }

    if (structure != NULL) {
        char canvas_title[128];

        ss_structure_canvas_title(structure, canvas_title, sizeof(canvas_title));
        ss_surface_draw_canvas_title_panel(&surface, canvas_title, theme);
        ss_surface_draw_text(&surface, 24, 20, canvas_title, theme->accent_color);
        ss_surface_draw_text(&surface, 24, 40, "Canvas interactivo", theme->badge_text);
        if (ss_editor_analysis_playback_has_steps(editor)) {
            char badge[64];
            snprintf(badge, sizeof(badge), "PASO %zu/%zu", editor->playback.current_step + 1, editor->playback.step_count);
            ss_surface_draw_badge(&surface, 236, 18, badge, theme);
        }
    }

    if (!stbi_write_png(path, width, height, 4, surface.pixels, width * 4)) {
        free(surface.pixels);
        ss_error_set(error, SS_ERROR_IO, "No se pudo escribir el PNG.");
        return 0;
    }

    free(surface.pixels);
    ss_error_clear(error);
    return 1;
}
