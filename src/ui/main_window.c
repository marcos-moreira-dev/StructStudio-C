/*
 * StructStudio C
 * --------------
 * Native main window and canvas interaction handlers.
 *
 * This file translates libui-ng widget events into editor actions while
 * keeping domain rules delegated to the editor/core layers.
 */

#include "ui/main_window.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ui.h>

#include "editor/editor.h"
#include "render/render.h"
#include "ui/system_dialogs.h"

typedef struct SsMainWindow {
    SsEditorState editor;
    SsTheme theme;
    uiWindow *window;
    uiBox *main_row;
    uiBox *canvas_row;
    uiControl *left_panel;
    uiControl *right_panel;
    uiBox *left_slot;
    uiBox *right_slot;
    uiBox *center_panel;
    uiBox *structure_tabs_host;
    uiBox *primary_row;
    uiBox *secondary_row;
    uiBox *numeric_row;
    uiTab *structure_tabs;
    uiArea *canvas;
    uiAreaHandler area_handler;
    uiCombobox *variant_combo;
    uiEntry *primary_input;
    uiEntry *secondary_input;
    uiEntry *numeric_input;
    uiEntry *analysis_input;
    uiEntry *prop_id;
    uiEntry *prop_label;
    uiEntry *prop_value;
    uiEntry *prop_secondary;
    uiEntry *prop_numeric;
    uiLabel *primary_input_label;
    uiLabel *secondary_input_label;
    uiLabel *numeric_input_label;
    uiLabel *status_label;
    uiLabel *operation_hint_label;
    uiLabel *operation_context_label;
    uiLabel *analysis_step_label;
    uiMultilineEntry *guide_box;
    uiMultilineEntry *theory_box;
    uiButton *primary_button;
    uiButton *secondary_button;
    uiButton *tertiary_button;
    uiButton *analysis_button;
    uiButton *analysis_prepare_button;
    uiButton *analysis_play_button;
    uiButton *analysis_prev_button;
    uiButton *analysis_next_button;
    uiButton *analysis_reset_button;
    uiButton *analysis_tree_button;
    uiButton *load_example_button;
    uiButton *tool_select_button;
    uiButton *tool_insert_button;
    uiButton *tool_connect_button;
    uiButton *tool_delete_button;
    uiButton *reset_view_button;
    uiButton *focus_canvas_button;
    uiButton *toggle_right_panel_button;
    uiButton *toggle_optional_numeric_button;
    uiButton *clear_operation_inputs_button;
    uiButton *rotate_left_button;
    uiButton *rotate_right_button;
    uiButton *undo_rotation_button;
    uiButton *redo_rotation_button;
    uiButton *apply_properties_button;
    uiButton *show_operations_button;
    uiButton *show_analysis_button;
    uiButton *show_guide_button;
    uiCombobox *analysis_combo;
    SsAnalysisKind analysis_options[8];
    size_t analysis_option_count;
    uiCheckbox *grid_checkbox;
    uiMenuItem *grid_menu_item;
    char (*structure_tab_titles)[128];
    size_t structure_tab_title_count;
    uiControl *operations_group_control;
    uiControl *analysis_group_control;
    uiControl *guide_group_control;
    uint64_t last_animation_tick_ms;
    int left_panel_attached;
    int right_panel_attached;
    int left_section_mode;
    int show_optional_numeric_fields;
    int syncing;
} SsMainWindow;

typedef struct SsOperationUiConfig {
    const char *primary_label;
    const char *secondary_label;
    const char *numeric_label;
    const char *hint;
    int show_primary;
    int show_secondary;
    int show_numeric;
    int numeric_optional;
} SsOperationUiConfig;

static SsMainWindow g_main_window;
enum {
    SS_LEFT_SECTION_OPERATIONS = 0,
    SS_LEFT_SECTION_ANALYSIS = 1,
    SS_LEFT_SECTION_GUIDE = 2
};
static void ss_after_state_change(void);
static void ss_after_animation_state_change(void);
static void ss_sync_full_ui_state(void);
static void ss_sync_animation_ui_state(void);
static SsAnalysisKind ss_selected_analysis_kind(void);
static void ss_sync_structure_tabs(void);
static void ss_try_or_show(int ok, SsError *error);
static void ss_structure_display_name(const SsStructure *structure, char *buffer, size_t capacity);
static void ss_sync_canvas_chrome(void);
static void ss_sync_left_section_buttons(void);

static void ss_queue_canvas_refresh(void)
{
    if (g_main_window.canvas != NULL) {
        uiAreaQueueRedrawAll(g_main_window.canvas);
    }
}

static void ss_free_structure_tab_cache(void)
{
    free(g_main_window.structure_tab_titles);
    g_main_window.structure_tab_titles = NULL;
    g_main_window.structure_tab_title_count = 0;
    g_main_window.structure_tabs = NULL;
}

static int ss_animation_timer_tick(void *data)
{
    int result;
    uint64_t now_ms;
    int elapsed_ms;

    (void) data;
    now_ms = ss_monotonic_millis();
    if (g_main_window.last_animation_tick_ms == 0 || now_ms <= g_main_window.last_animation_tick_ms) {
        elapsed_ms = 16;
    } else {
        uint64_t delta = now_ms - g_main_window.last_animation_tick_ms;
        if (delta > 33ULL) {
            delta = 33ULL;
        }
        elapsed_ms = (int) delta;
    }
    g_main_window.last_animation_tick_ms = now_ms;
    /* The timer must keep the UI thread light: playback/status updates are
     * refreshed incrementally, while full widget/layout synchronization is
     * reserved for semantic state changes triggered by user actions. */
    result = ss_editor_tick_animations(&g_main_window.editor, elapsed_ms);
    if ((result & SS_EDITOR_ANIMATION_STATE_CHANGED) != 0) {
        ss_after_animation_state_change();
    } else if ((result & SS_EDITOR_ANIMATION_REDRAW) != 0) {
        ss_queue_canvas_refresh();
    }
    return 1;
}

static void ss_show_error(const char *message)
{
    ss_dialog_show_error(g_main_window.window, "StructStudio C", message);
}

static int ss_entry_try_int(uiEntry *entry, int *value_out, int *has_text_out)
{
    char *text = uiEntryText(entry);
    char *start = text;
    char *end;
    int value = 0;
    int ok = 0;
    int has_text = 0;

    while (start != NULL && *start != '\0' && isspace((unsigned char) *start)) {
        ++start;
    }
    end = start != NULL ? start + strlen(start) : NULL;
    while (end != NULL && end > start && isspace((unsigned char) end[-1])) {
        --end;
    }
    if (end != NULL) {
        *end = '\0';
    }
    has_text = start != NULL && start[0] != '\0';
    ok = ss_parse_int(start, &value);
    uiFreeText(text);
    if (value_out != NULL) {
        *value_out = ok ? value : 0;
    }
    if (has_text_out != NULL) {
        *has_text_out = has_text;
    }
    return ok;
}

static int ss_entry_int(uiEntry *entry)
{
    int value = 0;

    ss_entry_try_int(entry, &value, NULL);
    return value;
}

static char *ss_entry_dup(uiEntry *entry)
{
    return uiEntryText(entry);
}

static size_t ss_capture_node_ids(const SsStructure *structure, char ids[][SS_ID_CAPACITY], size_t capacity)
{
    size_t count = 0;

    if (structure == NULL) {
        return 0;
    }

    count = structure->node_count < capacity ? structure->node_count : capacity;
    for (size_t index = 0; index < count; ++index) {
        ss_str_copy(ids[index], SS_ID_CAPACITY, structure->nodes[index].id);
    }
    return count;
}

static SsNode *ss_find_new_node(SsStructure *structure, char ids[][SS_ID_CAPACITY], size_t count)
{
    size_t index;

    if (structure == NULL) {
        return NULL;
    }

    for (index = 0; index < structure->node_count; ++index) {
        int known = 0;
        for (size_t previous = 0; previous < count; ++previous) {
            if (strcmp(structure->nodes[index].id, ids[previous]) == 0) {
                known = 1;
                break;
            }
        }
        if (!known) {
            return &structure->nodes[index];
        }
    }
    return NULL;
}

static void ss_position_canvas_node(SsStructure *structure, SsNode *node, double x, double y)
{
    if (structure == NULL || node == NULL) {
        return;
    }

    node->visual.x = ss_clamp_double(x - node->visual.width / 2.0, 20.0, 3200.0);
    node->visual.y = ss_clamp_double(y - node->visual.height / 2.0, 20.0, 2400.0);
    structure->dirty_layout = 0;
}

static const char *ss_connection_relation_for_click(const SsStructure *structure, const SsNode *source, const SsNode *target, double target_x)
{
    double source_center_x;
    double target_center_x;

    if (structure == NULL) {
        return "graph_link";
    }
    if (structure->variant != SS_VARIANT_BINARY_TREE) {
        return ss_default_relation_for_variant(structure->variant);
    }
    if (source == NULL) {
        return "left";
    }

    source_center_x = source->visual.x + source->visual.width / 2.0;
    target_center_x = target != NULL ? target->visual.x + target->visual.width / 2.0 : target_x;
    return target_center_x >= source_center_x ? "right" : "left";
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

static void ss_append_line(char *buffer, size_t capacity, const char *text)
{
    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    if (buffer[0] != '\0') {
        ss_append_text(buffer, capacity, "\r\n");
    }
    ss_append_text(buffer, capacity, text);
}

static const char *ss_tool_name(SsTool tool)
{
    switch (tool) {
        case SS_TOOL_SELECT:
            return "Seleccionar";
        case SS_TOOL_INSERT:
            return "Insertar";
        case SS_TOOL_CONNECT:
            return "Conectar";
        case SS_TOOL_DELETE:
            return "Eliminar";
        default:
            return "Seleccionar";
    }
}

static int ss_structure_supports_tree_rotations(const SsStructure *structure)
{
    return structure != NULL &&
        (structure->variant == SS_VARIANT_BST || structure->variant == SS_VARIANT_AVL);
}

static int ss_structure_supports_graph_rotation(const SsStructure *structure)
{
    return structure != NULL && structure->family == SS_FAMILY_GRAPH;
}

static int ss_mouse_event_starts_pan(const uiAreaMouseEvent *event)
{
    return event != NULL &&
        (event->Down == 2 || (event->Down == 1 && (event->Modifiers & uiModifierShift) != 0));
}

static const char *ss_structure_layout_suffix(const SsStructure *structure)
{
    if (structure == NULL) {
        return "";
    }
    if (strcmp(structure->visual_state.layout_mode, "tree_bfs") == 0) {
        return " [Árbol BFS]";
    }
    if (strcmp(structure->visual_state.layout_mode, "tree_dfs") == 0) {
        return " [Árbol DFS]";
    }
    if (strcmp(structure->visual_state.layout_mode, "tree_prim") == 0) {
        return " [Árbol Prim]";
    }
    if (strcmp(structure->visual_state.layout_mode, "tree_kruskal") == 0) {
        return " [Árbol Kruskal]";
    }
    return "";
}

static void ss_structure_tab_title(const SsStructure *structure, size_t index, char *buffer, size_t capacity)
{
    char display_name[96];

    if (buffer == NULL || capacity == 0) {
        return;
    }

    if (structure == NULL) {
        ss_str_copy(buffer, capacity, "Sin estructura");
        return;
    }

    ss_structure_display_name(structure, display_name, sizeof(display_name));
    snprintf(buffer, capacity, "%zu. %s", index + 1, display_name);
}

static int ss_structure_tabs_need_rebuild(void)
{
    if (g_main_window.structure_tabs_host == NULL) {
        return 0;
    }
    if (g_main_window.editor.document.structure_count != g_main_window.structure_tab_title_count) {
        return 1;
    }
    for (size_t index = 0; index < g_main_window.editor.document.structure_count; ++index) {
        char expected[128];

        ss_structure_tab_title(&g_main_window.editor.document.structures[index], index, expected, sizeof(expected));
        if (g_main_window.structure_tab_titles == NULL || strcmp(g_main_window.structure_tab_titles[index], expected) != 0) {
            return 1;
        }
    }
    return g_main_window.structure_tabs == NULL && g_main_window.editor.document.structure_count > 0;
}

static void ss_structure_tab_selected(uiTab *sender, void *data)
{
    SsError error;
    int selected;

    (void) data;
    if (g_main_window.syncing || sender == NULL) {
        return;
    }

    selected = uiTabSelected(sender);
    if (selected < 0) {
        return;
    }

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_activate_structure(&g_main_window.editor, (size_t) selected, &error), &error);
}

static void ss_rebuild_structure_tabs(void)
{
    size_t structure_count = g_main_window.editor.document.structure_count;
    char (*titles)[128];
    uiTab *tabs;

    if (g_main_window.structure_tabs_host == NULL) {
        return;
    }

    while (uiBoxNumChildren(g_main_window.structure_tabs_host) > 0) {
        uiBoxDelete(g_main_window.structure_tabs_host, 0);
    }
    ss_free_structure_tab_cache();

    if (structure_count == 0) {
        return;
    }

    titles = (char (*)[128]) calloc(structure_count, sizeof(*titles));
    if (titles == NULL) {
        ss_show_error("No se pudo construir la barra de pestanas del lienzo.");
        return;
    }

    tabs = uiNewTab();
    for (size_t index = 0; index < structure_count; ++index) {
        uiBox *page = uiNewHorizontalBox();

        ss_structure_tab_title(&g_main_window.editor.document.structures[index], index, titles[index], sizeof(titles[index]));
        uiTabAppend(tabs, titles[index], uiControl(page));
        uiTabSetMargined(tabs, (int) index, 0);
    }
    uiTabOnSelected(tabs, ss_structure_tab_selected, NULL);

    g_main_window.syncing = 1;
    uiBoxAppend(g_main_window.structure_tabs_host, uiControl(tabs), 0);
    g_main_window.structure_tabs = tabs;
    g_main_window.structure_tab_titles = titles;
    g_main_window.structure_tab_title_count = structure_count;
    uiTabSetSelected(tabs, (int) g_main_window.editor.document.active_structure_index);
    g_main_window.syncing = 0;
}

static void ss_structure_display_name(const SsStructure *structure, char *buffer, size_t capacity)
{
    const SsVariantDescriptor *descriptor;

    if (buffer == NULL || capacity == 0) {
        return;
    }

    buffer[0] = '\0';
    if (structure == NULL) {
        return;
    }

    descriptor = ss_variant_descriptor(structure->variant);
    snprintf(buffer, capacity, "%s%s", descriptor->display_name, ss_structure_layout_suffix(structure));
}

static SsAnalysisKind ss_selected_analysis_kind(void)
{
    int selected;

    if (g_main_window.analysis_combo == NULL) {
        return SS_ANALYSIS_NONE;
    }

    selected = uiComboboxSelected(g_main_window.analysis_combo);
    if (selected < 0 || (size_t) selected >= g_main_window.analysis_option_count) {
        return SS_ANALYSIS_NONE;
    }

    return g_main_window.analysis_options[selected];
}

static int ss_find_analysis_index(SsAnalysisKind kind)
{
    size_t index;

    for (index = 0; index < g_main_window.analysis_option_count; ++index) {
        if (g_main_window.analysis_options[index] == kind) {
            return (int) index;
        }
    }

    return -1;
}

static void ss_set_control_enabled(uiControl *control, int enabled)
{
    if (control == NULL) {
        return;
    }

    if (enabled) {
        uiControlEnable(control);
    } else {
        uiControlDisable(control);
    }
}

static void ss_set_control_visible(uiControl *control, int visible)
{
    if (control == NULL) {
        return;
    }

    if (visible) {
        uiControlShow(control);
    } else {
        uiControlHide(control);
    }
}

static void ss_sync_left_section_buttons(void)
{
    ss_set_control_visible(
        g_main_window.operations_group_control,
        g_main_window.left_section_mode == SS_LEFT_SECTION_OPERATIONS);
    ss_set_control_visible(
        g_main_window.analysis_group_control,
        g_main_window.left_section_mode == SS_LEFT_SECTION_ANALYSIS);
    ss_set_control_visible(
        g_main_window.guide_group_control,
        g_main_window.left_section_mode == SS_LEFT_SECTION_GUIDE);

    if (g_main_window.show_operations_button != NULL) {
        uiButtonSetText(
            g_main_window.show_operations_button,
            g_main_window.left_section_mode == SS_LEFT_SECTION_OPERATIONS ? "[Operaciones]" : "Operaciones");
    }
    if (g_main_window.show_analysis_button != NULL) {
        uiButtonSetText(
            g_main_window.show_analysis_button,
            g_main_window.left_section_mode == SS_LEFT_SECTION_ANALYSIS ? "[Análisis]" : "Análisis");
    }
    if (g_main_window.show_guide_button != NULL) {
        uiButtonSetText(
            g_main_window.show_guide_button,
            g_main_window.left_section_mode == SS_LEFT_SECTION_GUIDE ? "[Guía]" : "Guía");
    }
}

static void ss_clear_entry(uiEntry *entry)
{
    if (entry != NULL) {
        uiEntrySetText(entry, "");
    }
}

static uiBox *ss_build_labeled_entry_row(const char *label_text, uiLabel **label_out, uiEntry **entry_out)
{
    uiBox *row = uiNewVerticalBox();
    uiLabel *label = uiNewLabel(label_text != NULL ? label_text : "");
    uiEntry *entry = uiNewEntry();

    uiBoxSetPadded(row, 0);
    uiBoxAppend(row, uiControl(label), 0);
    uiBoxAppend(row, uiControl(entry), 0);

    if (label_out != NULL) {
        *label_out = label;
    }
    if (entry_out != NULL) {
        *entry_out = entry;
    }

    return row;
}

static int ss_variant_uses_secondary_input(SsVariant variant)
{
    return variant == SS_VARIANT_VECTOR ||
        variant == SS_VARIANT_MAP ||
        variant == SS_VARIANT_DIRECTED_GRAPH ||
        variant == SS_VARIANT_UNDIRECTED_GRAPH ||
        variant == SS_VARIANT_DIRECTED_WEIGHTED_GRAPH ||
        variant == SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH;
}

static int ss_variant_uses_numeric_input(const SsStructure *structure)
{
    if (structure == NULL) {
        return 0;
    }

    return structure->variant == SS_VARIANT_VECTOR ||
        structure->variant == SS_VARIANT_PRIORITY_QUEUE ||
        structure->variant == SS_VARIANT_BST ||
        structure->variant == SS_VARIANT_AVL ||
        structure->variant == SS_VARIANT_HEAP ||
        structure->config.is_weighted;
}

static int ss_variant_numeric_is_optional(const SsStructure *structure)
{
    if (structure == NULL) {
        return 0;
    }

    return structure->variant == SS_VARIANT_VECTOR ||
        structure->variant == SS_VARIANT_BST ||
        structure->variant == SS_VARIANT_AVL ||
        structure->variant == SS_VARIANT_HEAP;
}

static void ss_operation_ui_config_init(SsOperationUiConfig *config)
{
    memset(config, 0, sizeof(*config));
    config->primary_label = "Valor";
    config->secondary_label = "Dato adicional";
    config->numeric_label = "Número";
    config->hint = "Paso rápido:\r\n1. Completa los campos visibles.\r\n2. Ejecuta la acción.";
    config->show_primary = 1;
}

static void ss_describe_operation_ui(const SsStructure *structure, SsOperationUiConfig *config)
{
    ss_operation_ui_config_init(config);

    if (structure == NULL) {
        config->hint = "Paso rápido:\r\n1. Abre o crea una pestaña.\r\n2. Luego usa las acciones.";
        config->show_primary = 0;
        return;
    }

    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            config->primary_label = "Indice";
            config->secondary_label = "Valor";
            config->numeric_label = "Índice alterno";
            config->hint = "Paso rápido:\r\n1. Escribe índice.\r\n2. Escribe valor.\r\n3. Ejecuta la acción.";
            config->show_secondary = 1;
            config->show_numeric = 1;
            config->numeric_optional = 1;
            break;

        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
        case SS_VARIANT_STACK:
        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
        case SS_VARIANT_SET:
            config->primary_label = "Valor";
            config->hint = "Paso rápido:\r\n1. Escribe un valor.\r\n2. Inserta.\r\n3. Las demás acciones usan el estado actual.";
            break;

        case SS_VARIANT_PRIORITY_QUEUE:
            config->primary_label = "Valor";
            config->numeric_label = "Prioridad";
            config->hint = "Paso rápido:\r\n1. Escribe valor y prioridad.\r\n2. Inserta.\r\n3. Selecciona un nodo para editar.";
            config->show_numeric = 1;
            break;

        case SS_VARIANT_BINARY_TREE:
            config->primary_label = "Valor del nodo";
            config->hint = "Paso rápido:\r\n1. Crea la raíz.\r\n2. Selecciona un padre.\r\n3. Agrega hijo izq. o der.";
            break;

        case SS_VARIANT_BST:
            config->primary_label = "Valor";
            config->numeric_label = "Entero auxiliar";
            config->hint = "Paso rápido:\r\n1. Escribe un valor.\r\n2. Inserta, busca o elimina.\r\n3. Selecciona un nodo para rotar.";
            config->show_numeric = 1;
            config->numeric_optional = 1;
            break;

        case SS_VARIANT_AVL:
            config->primary_label = "Valor";
            config->numeric_label = "Entero auxiliar";
            config->hint = "Paso rápido:\r\n1. Escribe un valor.\r\n2. Inserta o elimina.\r\n3. Rebalancea o rota el árbol.";
            config->show_numeric = 1;
            config->numeric_optional = 1;
            break;

        case SS_VARIANT_HEAP:
            config->primary_label = "Valor";
            config->numeric_label = "Entero auxiliar";
            config->hint = "Paso rápido:\r\n1. Escribe un valor.\r\n2. Inserta o aplica heapify.\r\n3. El entero extra es opcional.";
            config->show_numeric = 1;
            config->numeric_optional = 1;
            break;

        case SS_VARIANT_MAP:
            config->primary_label = "Clave";
            config->secondary_label = "Valor";
            config->hint = "Paso rápido:\r\n1. Escribe clave.\r\n2. Escribe valor.\r\n3. Inserta, actualiza o busca.";
            config->show_secondary = 1;
            break;

        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
            config->primary_label = "Vértice nuevo";
            config->secondary_label = "Destino";
            config->hint = "Paso rápido:\r\n1. Agrega vértices.\r\n2. Selecciona origen.\r\n3. Escribe destino y crea arista.";
            config->show_secondary = 1;
            break;

        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            config->primary_label = "Vértice nuevo";
            config->secondary_label = "Destino";
            config->numeric_label = "Peso";
            config->hint = "Paso rápido:\r\n1. Agrega vértices.\r\n2. Selecciona origen.\r\n3. Escribe destino y peso.";
            config->show_secondary = 1;
            config->show_numeric = 1;
            break;

        default:
            break;
    }
}

static int ss_secondary_requires_node_selection(SsVariant variant)
{
    return variant == SS_VARIANT_BINARY_TREE ||
        variant == SS_VARIANT_DIRECTED_GRAPH ||
        variant == SS_VARIANT_UNDIRECTED_GRAPH ||
        variant == SS_VARIANT_DIRECTED_WEIGHTED_GRAPH ||
        variant == SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH;
}

static int ss_tertiary_requires_node_selection(SsVariant variant)
{
    return variant == SS_VARIANT_SINGLY_LINKED_LIST ||
        variant == SS_VARIANT_DOUBLY_LINKED_LIST ||
        variant == SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST ||
        variant == SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST ||
        variant == SS_VARIANT_PRIORITY_QUEUE ||
        variant == SS_VARIANT_BINARY_TREE;
}

static int ss_tertiary_requires_edge_selection(SsVariant variant)
{
    return variant == SS_VARIANT_DIRECTED_GRAPH ||
        variant == SS_VARIANT_UNDIRECTED_GRAPH ||
        variant == SS_VARIANT_DIRECTED_WEIGHTED_GRAPH ||
        variant == SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH;
}

static void ss_set_grid_visible(int visible)
{
    ss_editor_set_grid_visible(&g_main_window.editor, visible);
    if (g_main_window.grid_checkbox != NULL && uiCheckboxChecked(g_main_window.grid_checkbox) != visible) {
        uiCheckboxSetChecked(g_main_window.grid_checkbox, visible);
    }
    if (g_main_window.grid_menu_item != NULL && uiMenuItemChecked(g_main_window.grid_menu_item) != visible) {
        uiMenuItemSetChecked(g_main_window.grid_menu_item, visible);
    }
    ss_after_state_change();
}

static void ss_force_window_layout_refresh(void)
{
    int width;
    int height;

    if (g_main_window.window == NULL) {
        return;
    }

    uiWindowContentSize(g_main_window.window, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }

    uiWindowSetContentSize(g_main_window.window, width + 1, height);
    uiWindowSetContentSize(g_main_window.window, width, height);
}

static void ss_set_workspace_panels(int left_visible, int right_visible, const char *status)
{
    int normalized_left_visible = 1;
    int normalized_right_visible = right_visible ? 1 : 0;
    int changed =
        g_main_window.editor.document.view_state.left_panel_visible != normalized_left_visible ||
        g_main_window.editor.document.view_state.right_panel_visible != normalized_right_visible;

    (void) left_visible;
    g_main_window.editor.document.view_state.left_panel_visible = normalized_left_visible;
    g_main_window.editor.document.view_state.right_panel_visible = normalized_right_visible;
    if (status != NULL && status[0] != '\0') {
        ss_editor_set_status(&g_main_window.editor, "%s", status);
    }
    ss_after_state_change();
    if (changed) {
        ss_force_window_layout_refresh();
    }
}

static void ss_sync_panel_visibility(void)
{
    int right_visible;

    g_main_window.editor.document.view_state.left_panel_visible = 1;
    if (g_main_window.left_slot != NULL) {
        uiControlShow(uiControl(g_main_window.left_slot));
    }
    if (g_main_window.left_panel != NULL) {
        uiControlShow(g_main_window.left_panel);
    }

    if (g_main_window.canvas_row == NULL || g_main_window.center_panel == NULL || g_main_window.right_slot == NULL || g_main_window.right_panel == NULL) {
        return;
    }

    uiControlShow(uiControl(g_main_window.center_panel));
    right_visible = g_main_window.editor.document.view_state.right_panel_visible ? 1 : 0;

    if (right_visible) {
        if (!g_main_window.right_panel_attached) {
            uiBoxAppend(g_main_window.canvas_row, uiControl(g_main_window.right_slot), 0);
            g_main_window.right_panel_attached = 1;
        }
        uiControlShow(g_main_window.right_panel);
        uiControlShow(uiControl(g_main_window.right_slot));
        return;
    }

    if (g_main_window.right_panel_attached && uiBoxNumChildren(g_main_window.canvas_row) > 1) {
        uiBoxDelete(g_main_window.canvas_row, 1);
        g_main_window.right_panel_attached = 0;
    }
    uiControlHide(g_main_window.right_panel);
    uiControlHide(uiControl(g_main_window.right_slot));
}

static void ss_sync_canvas_chrome(void)
{
    int right_visible = g_main_window.editor.document.view_state.right_panel_visible;

    if (g_main_window.focus_canvas_button != NULL) {
        uiButtonSetText(
            g_main_window.focus_canvas_button,
            !right_visible ? "[Lienzo + herramientas]" : "Lienzo + herramientas");
    }
    if (g_main_window.toggle_right_panel_button != NULL) {
        uiButtonSetText(
            g_main_window.toggle_right_panel_button,
            right_visible ? "[Lienzo + herramientas + teoría]" : "Lienzo + herramientas + teoría");
    }
}

static void ss_sync_tool_buttons(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    const SsVariantDescriptor *descriptor = structure != NULL ? ss_variant_descriptor(structure->variant) : NULL;
    int connect_supported = descriptor != NULL ? descriptor->supports_connect : 0;

    if (!connect_supported && g_main_window.editor.tool == SS_TOOL_CONNECT) {
        ss_editor_set_tool(&g_main_window.editor, SS_TOOL_SELECT);
        ss_editor_set_status(&g_main_window.editor, "Conectar no aplica a esta estructura. Se activo Seleccionar.");
    }

    uiButtonSetText(g_main_window.tool_select_button, g_main_window.editor.tool == SS_TOOL_SELECT ? "[Seleccionar]" : "Seleccionar");
    uiButtonSetText(g_main_window.tool_insert_button, g_main_window.editor.tool == SS_TOOL_INSERT ? "[Insertar]" : "Insertar");
    uiButtonSetText(g_main_window.tool_connect_button, g_main_window.editor.tool == SS_TOOL_CONNECT ? "[Conectar]" : "Conectar");
    uiButtonSetText(g_main_window.tool_delete_button, g_main_window.editor.tool == SS_TOOL_DELETE ? "[Eliminar]" : "Eliminar");
    ss_set_control_enabled(uiControl(g_main_window.tool_connect_button), connect_supported);
}

static void ss_sync_operation_hint(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    char buffer[256] = "";
    char context[256] = "";
    char summary[640] = "";
    SsOperationUiConfig config;

    ss_describe_operation_ui(structure, &config);
    ss_str_copy(buffer, sizeof(buffer), config.hint);

    if (structure == NULL) {
        ss_str_copy(context, sizeof(context), "Sin estructura activa.");
    } else if (g_main_window.editor.selection.type == SS_SELECTION_NODE) {
        snprintf(context, sizeof(context), "Selección actual: nodo %s.", g_main_window.editor.selection.node_id);
        if (ss_structure_supports_tree_rotations(structure)) {
            ss_append_text(context, sizeof(context), " Puedes rotar el árbol desde este nodo.");
        } else if (ss_structure_supports_graph_rotation(structure)) {
            ss_append_text(context, sizeof(context), " Puedes crear aristas desde este origen o girar la vista.");
        }
    } else if (g_main_window.editor.selection.type == SS_SELECTION_EDGE) {
        snprintf(context, sizeof(context), "Selección actual: arista %s.", g_main_window.editor.selection.edge_id);
        if (ss_tertiary_requires_edge_selection(structure->variant)) {
            ss_append_text(context, sizeof(context), " La accion contextual elimina esa arista.");
        }
    } else if (ss_structure_supports_tree_rotations(structure) || ss_secondary_requires_node_selection(structure->variant)) {
        ss_str_copy(context, sizeof(context), "Selección actual: ninguna. Selecciona un nodo del lienzo para habilitar acciones contextuales.");
    } else if (ss_tertiary_requires_edge_selection(structure->variant)) {
        ss_str_copy(context, sizeof(context), "Selección actual: ninguna. Selecciona una arista para usar la acción contextual.");
    } else {
        ss_str_copy(context, sizeof(context), "Selección actual: ninguna. Solo completa los campos visibles y pulsa una acción.");
    }

    if (strncmp(buffer, "Paso rápido:", 12) == 0) {
        ss_str_copy(summary, sizeof(summary), buffer);
    } else {
        snprintf(summary, sizeof(summary), "Paso rápido: %s", buffer);
    }
    if (g_main_window.operation_hint_label != NULL) {
        uiLabelSetText(g_main_window.operation_hint_label, summary);
    }
    if (g_main_window.operation_context_label != NULL) {
        uiLabelSetText(g_main_window.operation_context_label, context);
    }
}

static void ss_sync_operation_fields(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    SsOperationUiConfig config;
    int show_primary;
    int show_secondary;
    int show_numeric;
    int show_toggle;

    ss_describe_operation_ui(structure, &config);
    if (structure == NULL || !config.numeric_optional) {
        g_main_window.show_optional_numeric_fields = 0;
    }

    show_primary = structure != NULL && config.show_primary;
    show_secondary = structure != NULL && config.show_secondary;
    show_numeric = structure != NULL && config.show_numeric &&
        (!config.numeric_optional || g_main_window.show_optional_numeric_fields);
    show_toggle = structure != NULL && config.numeric_optional;

    if (g_main_window.primary_input_label != NULL) {
        uiLabelSetText(g_main_window.primary_input_label, config.primary_label);
    }
    if (g_main_window.secondary_input_label != NULL) {
        uiLabelSetText(g_main_window.secondary_input_label, config.secondary_label);
    }
    if (g_main_window.numeric_input_label != NULL) {
        if (config.numeric_optional) {
            char label[96];
            snprintf(label, sizeof(label), "%s (opcional)", config.numeric_label);
            uiLabelSetText(g_main_window.numeric_input_label, label);
        } else {
            uiLabelSetText(g_main_window.numeric_input_label, config.numeric_label);
        }
    }

    ss_set_control_visible(uiControl(g_main_window.primary_row), show_primary);
    ss_set_control_visible(uiControl(g_main_window.secondary_row), show_secondary);
    ss_set_control_visible(uiControl(g_main_window.numeric_row), show_numeric);
    ss_set_control_visible(uiControl(g_main_window.toggle_optional_numeric_button), show_toggle);

    if (!show_secondary) {
        ss_clear_entry(g_main_window.secondary_input);
    }
    if (!show_numeric) {
        ss_clear_entry(g_main_window.numeric_input);
    }

    if (g_main_window.toggle_optional_numeric_button != NULL) {
        uiButtonSetText(
            g_main_window.toggle_optional_numeric_button,
            g_main_window.show_optional_numeric_fields ? "Ocultar parámetro" : "Ver parámetro");
    }
}

static void ss_sync_operation_controls(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    int has_node = g_main_window.editor.selection.type == SS_SELECTION_NODE;
    int has_edge = g_main_window.editor.selection.type == SS_SELECTION_EDGE;
    int secondary_enabled = structure != NULL;
    int tertiary_enabled = structure != NULL;
    int allow_secondary_input = 0;
    int allow_numeric_input = 0;
    int rotation_enabled = 0;
    int rotation_history_visible = 0;
    int can_undo_rotation = ss_editor_can_undo_rotation(&g_main_window.editor);
    int can_redo_rotation = ss_editor_can_redo_rotation(&g_main_window.editor);

    if (structure == NULL) {
        ss_set_control_enabled(uiControl(g_main_window.primary_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.secondary_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.tertiary_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.rotate_left_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.rotate_right_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.undo_rotation_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.redo_rotation_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.primary_input), 0);
        ss_set_control_enabled(uiControl(g_main_window.secondary_input), 0);
        ss_set_control_enabled(uiControl(g_main_window.numeric_input), 0);
        ss_set_control_enabled(uiControl(g_main_window.clear_operation_inputs_button), 0);
        ss_set_control_enabled(uiControl(g_main_window.toggle_optional_numeric_button), 0);
        return;
    }

    allow_secondary_input = ss_variant_uses_secondary_input(structure->variant);
    allow_numeric_input = ss_variant_uses_numeric_input(structure);

    if (ss_secondary_requires_node_selection(structure->variant) && !has_node) {
        secondary_enabled = 0;
    }
    if (ss_tertiary_requires_node_selection(structure->variant) && !has_node) {
        tertiary_enabled = 0;
    }
    if (ss_tertiary_requires_edge_selection(structure->variant) && !has_edge) {
        tertiary_enabled = 0;
    }
    if (ss_structure_supports_tree_rotations(structure)) {
        rotation_enabled = has_node;
        rotation_history_visible = 1;
    } else if (ss_structure_supports_graph_rotation(structure)) {
        rotation_enabled = structure->node_count > 0;
        rotation_history_visible = 1;
    }

    ss_set_control_enabled(uiControl(g_main_window.primary_button), 1);
    ss_set_control_enabled(uiControl(g_main_window.secondary_button), secondary_enabled);
    ss_set_control_enabled(uiControl(g_main_window.tertiary_button), tertiary_enabled);
    ss_set_control_enabled(uiControl(g_main_window.rotate_left_button), rotation_enabled);
    ss_set_control_enabled(uiControl(g_main_window.rotate_right_button), rotation_enabled);
    ss_set_control_enabled(uiControl(g_main_window.undo_rotation_button), rotation_history_visible && can_undo_rotation);
    ss_set_control_enabled(uiControl(g_main_window.redo_rotation_button), rotation_history_visible && can_redo_rotation);
    ss_set_control_enabled(uiControl(g_main_window.primary_input), 1);
    ss_set_control_enabled(uiControl(g_main_window.secondary_input), allow_secondary_input);
    ss_set_control_enabled(
        uiControl(g_main_window.numeric_input),
        allow_numeric_input &&
            (g_main_window.show_optional_numeric_fields || !ss_variant_numeric_is_optional(structure)));
    ss_set_control_enabled(uiControl(g_main_window.clear_operation_inputs_button), 1);
    ss_set_control_enabled(uiControl(g_main_window.toggle_optional_numeric_button), ss_variant_numeric_is_optional(structure));
}

static void ss_sync_analysis_options(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    SsAnalysisKind previous_kind = ss_selected_analysis_kind();
    int selected_index = -1;

    if (g_main_window.analysis_combo == NULL) {
        return;
    }

    g_main_window.analysis_option_count = 0;
    g_main_window.syncing = 1;
    uiComboboxClear(g_main_window.analysis_combo);

    if (structure != NULL) {
        g_main_window.analysis_option_count = ss_analysis_kinds_for_variant(
            structure->variant,
            g_main_window.analysis_options,
            sizeof(g_main_window.analysis_options) / sizeof(g_main_window.analysis_options[0]));
    }

    for (size_t index = 0; index < g_main_window.analysis_option_count; ++index) {
        uiComboboxAppend(g_main_window.analysis_combo, ss_analysis_kind_label(g_main_window.analysis_options[index]));
    }

    selected_index = ss_find_analysis_index(previous_kind);
    if (selected_index < 0 && g_main_window.analysis_option_count > 0) {
        selected_index = 0;
    }
    if (selected_index >= 0) {
        uiComboboxSetSelected(g_main_window.analysis_combo, selected_index);
    }

    g_main_window.syncing = 0;
}

static void ss_sync_analysis_controls(void)
{
    int has_analysis = g_main_window.analysis_combo != NULL && g_main_window.analysis_option_count > 0;
    SsAnalysisKind kind = ss_selected_analysis_kind();
    SsAnalysisStartMode start_mode = ss_analysis_start_mode(kind);
    int supports_playback = has_analysis && kind != SS_ANALYSIS_NONE && ss_analysis_supports_playback(kind);
    int supports_tree_generation = has_analysis && kind != SS_ANALYSIS_NONE && ss_analysis_supports_tree_generation(kind);
    int has_playback = ss_editor_analysis_playback_has_steps(&g_main_window.editor);
    int autoplay = ss_editor_playback_autoplay_enabled(&g_main_window.editor);
    char button_text[96];
    char step_text[384];

    ss_set_control_enabled(uiControl(g_main_window.analysis_combo), has_analysis);
    ss_set_control_enabled(uiControl(g_main_window.analysis_input), has_analysis && start_mode != SS_ANALYSIS_START_NONE);
    ss_set_control_enabled(uiControl(g_main_window.analysis_button), has_analysis);
    ss_set_control_enabled(uiControl(g_main_window.analysis_prepare_button), supports_playback);
    ss_set_control_enabled(uiControl(g_main_window.analysis_tree_button), supports_tree_generation);
    ss_set_control_enabled(uiControl(g_main_window.analysis_play_button), has_playback);
    ss_set_control_enabled(uiControl(g_main_window.analysis_prev_button), has_playback && !autoplay && g_main_window.editor.playback.current_step > 0);
    ss_set_control_enabled(uiControl(g_main_window.analysis_next_button), has_playback && !autoplay && g_main_window.editor.playback.current_step + 1 < g_main_window.editor.playback.step_count);
    ss_set_control_enabled(uiControl(g_main_window.analysis_reset_button), has_playback);
    uiButtonSetText(g_main_window.analysis_play_button, autoplay ? "Pausar animacion" : "Auto-reproducir");

    if (!has_analysis || kind == SS_ANALYSIS_NONE) {
        uiButtonSetText(g_main_window.analysis_button, "Resumen");
        uiButtonSetText(g_main_window.analysis_prepare_button, "Simular");
        uiButtonSetText(g_main_window.analysis_tree_button, "Arbol derivado");
        uiButtonSetText(g_main_window.analysis_play_button, "Auto-reproducir");
        uiLabelSetText(g_main_window.analysis_step_label, "Sin analisis disponible.");
        return;
    }

    snprintf(button_text, sizeof(button_text), "Resumen: %s", ss_analysis_kind_label(kind));
    uiButtonSetText(g_main_window.analysis_button, button_text);
    snprintf(button_text, sizeof(button_text), "Simular: %s", ss_analysis_kind_label(kind));
    uiButtonSetText(g_main_window.analysis_prepare_button, button_text);
    if (supports_tree_generation) {
        snprintf(button_text, sizeof(button_text), "Arbol: %s", ss_analysis_kind_label(kind));
        uiButtonSetText(g_main_window.analysis_tree_button, button_text);
    } else {
        uiButtonSetText(g_main_window.analysis_tree_button, "Arbol derivado");
    }

    if (has_playback) {
        const SsAnalysisStep *step = ss_editor_current_analysis_step(&g_main_window.editor);
        if (step != NULL) {
            snprintf(
                step_text,
                sizeof(step_text),
                "Paso %zu de %zu | %s%s%s | %s",
                g_main_window.editor.playback.current_step + 1,
                g_main_window.editor.playback.step_count,
                ss_analysis_step_kind_label(step->kind),
                autoplay ? " | " : "",
                autoplay ? "animacion activa" : "",
                step->message);
            uiLabelSetText(g_main_window.analysis_step_label, step_text);
            return;
        }
    }

    if (supports_playback) {
        uiLabelSetText(g_main_window.analysis_step_label, "Simular anima. Resumen solo reporta.");
    } else if (supports_tree_generation) {
        uiLabelSetText(g_main_window.analysis_step_label, "Puede abrir un arbol derivado.");
    } else {
        uiLabelSetText(g_main_window.analysis_step_label, "Este analisis es solo textual.");
    }
}

static void ss_sync_theory_panel(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    SsAnalysisKind kind = ss_selected_analysis_kind();
    SsError error;
    char theory[SS_THEORY_TEXT_CAPACITY];
    char fallback[512];

    if (g_main_window.theory_box == NULL) {
        return;
    }

    if (structure == NULL) {
        uiMultilineEntrySetText(
            g_main_window.theory_box,
            "No hay pestaña activa.\r\n\r\nCree o abra una estructura para ver su teoría, variantes y recorridos relacionados.");
        return;
    }

    ss_error_clear(&error);
    if (!ss_build_theory_summary(structure, kind, theory, sizeof(theory), &error)) {
        snprintf(fallback, sizeof(fallback), "No se pudo cargar la teoría contextual.\r\n\r\nDetalle: %s", error.message);
        uiMultilineEntrySetText(g_main_window.theory_box, fallback);
        return;
    }

    uiMultilineEntrySetText(g_main_window.theory_box, theory);
}

static void ss_sync_property_controls(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    int node_selected = g_main_window.editor.selection.type == SS_SELECTION_NODE;
    int edge_selected = g_main_window.editor.selection.type == SS_SELECTION_EDGE;
    int map_node_selected = node_selected && structure != NULL && structure->variant == SS_VARIANT_MAP;
    int priority_node_selected = node_selected && structure != NULL && structure->variant == SS_VARIANT_PRIORITY_QUEUE;

    uiEntrySetReadOnly(g_main_window.prop_id, 1);
    uiEntrySetReadOnly(g_main_window.prop_label, !(node_selected || edge_selected));
    uiEntrySetReadOnly(g_main_window.prop_value, !node_selected);
    uiEntrySetReadOnly(g_main_window.prop_secondary, !map_node_selected);
    uiEntrySetReadOnly(g_main_window.prop_numeric, !(priority_node_selected || edge_selected));
    ss_set_control_enabled(uiControl(g_main_window.apply_properties_button), node_selected || edge_selected);
}

static void ss_sync_guide(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    char buffer[SS_ANALYSIS_REPORT_CAPACITY + 2048] = "";

    if (structure == NULL) {
        uiMultilineEntrySetText(g_main_window.guide_box, "Sin pestaña activa.");
        return;
    }

    {
        char display_name[96];
        ss_structure_display_name(structure, display_name, sizeof(display_name));
        ss_append_line(buffer, sizeof(buffer), display_name);
    }
    {
        char line[256];
        snprintf(line, sizeof(line), "Herramienta activa: %s", ss_tool_name(g_main_window.editor.tool));
        ss_append_line(buffer, sizeof(buffer), line);
    }
    ss_append_line(buffer, sizeof(buffer), "Pestañas: use la barra superior del lienzo para cambiar entre estructuras abiertas.");
    ss_append_line(buffer, sizeof(buffer), "Atajo didáctico: use Ejemplo en pestaña para poblar la vista actual con un caso base.");
    ss_append_line(buffer, sizeof(buffer), "Navegación: arrastre con botón central o Shift+arrastre para paneo. Recentrar vista vuelve al origen.");

    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            ss_append_line(buffer, sizeof(buffer), "Use Principal como índice y Secundario como valor.");
            ss_append_line(buffer, sizeof(buffer), "Insertar crea o actualiza la celda; Reemplazar cambia el valor existente.");
            break;
        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            ss_append_line(buffer, sizeof(buffer), "Primaria inserta al inicio y Secundaria al final.");
            ss_append_line(buffer, sizeof(buffer), "Seleccione un nodo antes de usar la operación terciaria.");
            break;
        case SS_VARIANT_STACK:
            ss_append_line(buffer, sizeof(buffer), "Primaria hace Push, Secundaria hace Pop y Terciaria muestra TOP.");
            break;
        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
            ss_append_line(buffer, sizeof(buffer), "Primaria hace Enqueue y Secundaria hace Dequeue.");
            break;
        case SS_VARIANT_PRIORITY_QUEUE:
            ss_append_line(buffer, sizeof(buffer), "Número define la prioridad del elemento.");
            ss_append_line(buffer, sizeof(buffer), "Seleccione un nodo para editar prioridad con la operación terciaria.");
            break;
        case SS_VARIANT_BINARY_TREE:
            ss_append_line(buffer, sizeof(buffer), "Primaria crea la raíz. Seleccione un nodo para agregar hijo izquierdo o derecho.");
            ss_append_line(buffer, sizeof(buffer), "Conectar permite unir padre e hijo con dos clics; el lado izquierdo o derecho depende de la posición destino.");
            break;
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
        case SS_VARIANT_HEAP:
            ss_append_line(buffer, sizeof(buffer), "Trabaje con valores enteros usando Principal o Número.");
            ss_append_line(buffer, sizeof(buffer), "El editor mantiene el layout coherente después de cada operación.");
            if (ss_structure_supports_tree_rotations(structure)) {
                ss_append_line(buffer, sizeof(buffer), "Con un nodo seleccionado puede aplicar rotación izquierda o derecha para estudiar reestructuración local.");
                ss_append_line(buffer, sizeof(buffer), "Didáctica rápida: rotación izquierda promociona al hijo derecho; rotación derecha promociona al hijo izquierdo.");
            }
            break;
        case SS_VARIANT_SET:
            ss_append_line(buffer, sizeof(buffer), "Solo se admiten valores unicos.");
            break;
        case SS_VARIANT_MAP:
            ss_append_line(buffer, sizeof(buffer), "Principal es la clave y Secundario el valor.");
            ss_append_line(buffer, sizeof(buffer), "Seleccione un nodo para editar clave/valor desde Propiedades.");
            break;
        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            ss_append_line(buffer, sizeof(buffer), "Insertar en canvas crea vértices donde haga clic.");
            ss_append_line(buffer, sizeof(buffer), "Conectar usa dos clics; en grafos ponderados Número define el peso.");
            ss_append_line(buffer, sizeof(buffer), "La operación secundaria conecta el vértice seleccionado usando un destino por ID, etiqueta o valor.");
            ss_append_line(buffer, sizeof(buffer), "Cada vértice muestra su identificador técnico bajo el nodo para facilitar referencia y depuración.");
            ss_append_line(buffer, sizeof(buffer), "Rot. izq y Rot. der giran la geometría del grafo; con un nodo seleccionado ese vértice actúa como pivote y sin selección se usa el centro visual.");
            ss_append_line(buffer, sizeof(buffer), "Desde Análisis puede abrir árboles BFS/DFS y, en grafos ponderados no dirigidos, árboles de Prim o Kruskal en pestañas derivadas.");
            break;
        default:
            break;
    }

    if (g_main_window.editor.connection.is_connecting) {
        char line[256];
        snprintf(line, sizeof(line), "Conexión en curso desde %s. Haga clic en el destino o pulse Esc.", g_main_window.editor.connection.source_node_id);
        ss_append_line(buffer, sizeof(buffer), line);
    } else if (g_main_window.editor.selection.type == SS_SELECTION_NODE) {
        char line[256];
        snprintf(line, sizeof(line), "Nodo seleccionado: %s", g_main_window.editor.selection.node_id);
        ss_append_line(buffer, sizeof(buffer), line);
        if (ss_structure_supports_tree_rotations(structure)) {
            const SsNode *selected = ss_structure_find_node_const(structure, g_main_window.editor.selection.node_id);

            if (selected != NULL) {
                if (selected->data.ref_b[0] != '\0') {
                    snprintf(line, sizeof(line), "Rot. izq sobre %s: sube %s y el pivote baja a su rama izquierda.", selected->id, selected->data.ref_b);
                } else {
                    snprintf(line, sizeof(line), "Rot. izq sobre %s: no aplica porque falta hijo derecho.", selected->id);
                }
                ss_append_line(buffer, sizeof(buffer), line);
                if (selected->data.ref_a[0] != '\0') {
                    snprintf(line, sizeof(line), "Rot. der sobre %s: sube %s y el pivote baja a su rama derecha.", selected->id, selected->data.ref_a);
                } else {
                    snprintf(line, sizeof(line), "Rot. der sobre %s: no aplica porque falta hijo izquierdo.", selected->id);
                }
                ss_append_line(buffer, sizeof(buffer), line);
            }
        }
    } else if (g_main_window.editor.selection.type == SS_SELECTION_EDGE) {
        char line[256];
        snprintf(line, sizeof(line), "Arista seleccionada: %s", g_main_window.editor.selection.edge_id);
        ss_append_line(buffer, sizeof(buffer), line);
    } else {
        ss_append_line(buffer, sizeof(buffer), "Accesos rápidos: Delete elimina la selección y Esc cancela la interacción actual.");
    }

    if (g_main_window.analysis_option_count > 0) {
        SsAnalysisKind kind = ss_selected_analysis_kind();
        SsAnalysisStartMode start_mode = ss_analysis_start_mode(kind);

        ss_append_line(buffer, sizeof(buffer), "-----");
        if (kind != SS_ANALYSIS_NONE) {
            char line[256];
            int supports_playback = ss_analysis_supports_playback(kind);

            snprintf(line, sizeof(line), "Análisis activo: %s", ss_analysis_kind_label(kind));
            ss_append_line(buffer, sizeof(buffer), line);
            switch (start_mode) {
                case SS_ANALYSIS_START_REQUIRED:
                    ss_append_line(buffer, sizeof(buffer), "Origen: seleccione un nodo o escriba su ID, etiqueta o valor.");
                    break;
                case SS_ANALYSIS_START_OPTIONAL:
                    ss_append_line(buffer, sizeof(buffer), "Origen opcional: puede seleccionar un nodo o escribir ID, etiqueta o valor.");
                    break;
                case SS_ANALYSIS_START_NONE:
                default:
                    ss_append_line(buffer, sizeof(buffer), "Este análisis no requiere vértice de origen.");
                    break;
            }
            if (supports_playback) {
                ss_append_line(buffer, sizeof(buffer), "Simular paso a paso prepara la animación didáctica; Ver resumen textual deja solo el reporte del algoritmo.");
            } else {
                ss_append_line(buffer, sizeof(buffer), "Ver resumen textual genera el reporte de este análisis.");
            }
            if (ss_analysis_supports_tree_generation(kind)) {
                ss_append_line(buffer, sizeof(buffer), "Puede abrir una nueva pestaña derivada con el botón de árbol del panel Análisis.");
            }
        } else {
            ss_append_line(buffer, sizeof(buffer), "Analisis: seleccione una opcion disponible.");
        }
    }

    if (ss_editor_analysis_playback_has_steps(&g_main_window.editor)) {
        const SsAnalysisStep *step = ss_editor_current_analysis_step(&g_main_window.editor);
        char line[384];

        ss_append_line(buffer, sizeof(buffer), "-----");
        snprintf(
            line,
            sizeof(line),
            "Recorrido guiado: paso %zu de %zu",
            g_main_window.editor.playback.current_step + 1,
            g_main_window.editor.playback.step_count);
        ss_append_line(buffer, sizeof(buffer), line);
        if (step != NULL) {
            snprintf(line, sizeof(line), "%s: %s", ss_analysis_step_kind_label(step->kind), step->message);
            ss_append_line(buffer, sizeof(buffer), line);
        }
        ss_append_line(buffer, sizeof(buffer), "Use Paso anterior, Paso siguiente o las flechas izquierda/derecha sobre el canvas.");
    }

    if (g_main_window.editor.analysis_report[0] != '\0') {
        ss_append_line(buffer, sizeof(buffer), "-----");
        ss_append_line(buffer, sizeof(buffer), "Resumen textual del analisis:");
        ss_append_line(buffer, sizeof(buffer), g_main_window.editor.analysis_report);
    }

    uiMultilineEntrySetText(g_main_window.guide_box, buffer);
}

static void ss_refresh_status(void)
{
    char status[512];
    const char *validation = g_main_window.editor.validation.count > 0
        ? g_main_window.editor.validation.items[0].message
        : "Estructura consistente";

    if (g_main_window.editor.status[0] != '\0') {
        snprintf(status, sizeof(status), "Estado: %.220s | Validacion: %.220s", g_main_window.editor.status, validation);
    } else {
        snprintf(status, sizeof(status), "Validacion: %.220s", validation);
    }

    uiLabelSetText(g_main_window.status_label, status);
}

static void ss_sync_operation_buttons(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    const SsVariantDescriptor *descriptor = structure != NULL ? ss_variant_descriptor(structure->variant) : ss_variant_descriptor(SS_VARIANT_VECTOR);
    const char *primary_text = descriptor->operation_primary;
    const char *secondary_text = descriptor->operation_secondary;
    const char *tertiary_text = descriptor->operation_tertiary;

    if (structure != NULL) {
        switch (structure->variant) {
            case SS_VARIANT_BINARY_TREE:
                primary_text = "Crear raíz";
                secondary_text = "Agregar hijo izq";
                tertiary_text = "Agregar hijo der";
                break;

            case SS_VARIANT_DIRECTED_GRAPH:
            case SS_VARIANT_UNDIRECTED_GRAPH:
            case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
            case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
                primary_text = "Agregar vértice";
                secondary_text = "Crear arista";
                tertiary_text = "Eliminar arista";
                break;

            default:
                break;
        }
    }

    uiButtonSetText(g_main_window.primary_button, primary_text);
    uiButtonSetText(g_main_window.secondary_button, secondary_text);
    uiButtonSetText(g_main_window.tertiary_button, tertiary_text);
    if (ss_structure_supports_graph_rotation(structure)) {
        uiButtonSetText(g_main_window.rotate_left_button, "Girar izq");
        uiButtonSetText(g_main_window.rotate_right_button, "Girar der");
    } else {
        uiButtonSetText(g_main_window.rotate_left_button, "Rot. izq");
        uiButtonSetText(g_main_window.rotate_right_button, "Rot. der");
    }
}

static void ss_sync_structure_tabs(void)
{
    if (ss_structure_tabs_need_rebuild()) {
        ss_rebuild_structure_tabs();
    }

    if (g_main_window.structure_tabs != NULL &&
        g_main_window.editor.document.active_structure_index < g_main_window.editor.document.structure_count &&
        uiTabSelected(g_main_window.structure_tabs) != (int) g_main_window.editor.document.active_structure_index) {
        g_main_window.syncing = 1;
        uiTabSetSelected(g_main_window.structure_tabs, (int) g_main_window.editor.document.active_structure_index);
        g_main_window.syncing = 0;
    }
}

static void ss_sync_structure_actions(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);

    ss_set_control_enabled(uiControl(g_main_window.load_example_button), structure != NULL);
}

static void ss_sync_properties(void)
{
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);

    uiEntrySetText(g_main_window.prop_id, "");
    uiEntrySetText(g_main_window.prop_label, "");
    uiEntrySetText(g_main_window.prop_value, "");
    uiEntrySetText(g_main_window.prop_secondary, "");
    uiEntrySetText(g_main_window.prop_numeric, "");

    if (structure == NULL) {
        return;
    }

    if (g_main_window.editor.selection.type == SS_SELECTION_NODE) {
        const SsNode *node = ss_structure_find_node_const(structure, g_main_window.editor.selection.node_id);
        char numeric[32];
        if (node == NULL) {
            return;
        }
        snprintf(numeric, sizeof(numeric), "%d", node->data.priority);
        uiEntrySetText(g_main_window.prop_id, node->id);
        uiEntrySetText(g_main_window.prop_label, node->label);
        uiEntrySetText(g_main_window.prop_value, node->value);
        uiEntrySetText(g_main_window.prop_secondary, structure->variant == SS_VARIANT_MAP ? node->data.key : node->data.aux_text);
        uiEntrySetText(g_main_window.prop_numeric, numeric);
    } else if (g_main_window.editor.selection.type == SS_SELECTION_EDGE) {
        const SsEdge *edge = ss_structure_find_edge_const(structure, g_main_window.editor.selection.edge_id);
        char weight[32];
        if (edge == NULL) {
            return;
        }
        snprintf(weight, sizeof(weight), "%.0f", edge->weight);
        uiEntrySetText(g_main_window.prop_id, edge->id);
        uiEntrySetText(g_main_window.prop_label, edge->relation_kind);
        uiEntrySetText(g_main_window.prop_numeric, weight);
    } else {
        char info[64];
        char display_name[96];
        snprintf(info, sizeof(info), "%zu nodos / %zu aristas", structure->node_count, structure->edge_count);
        ss_structure_display_name(structure, display_name, sizeof(display_name));
        uiEntrySetText(g_main_window.prop_id, structure->id);
        uiEntrySetText(g_main_window.prop_label, display_name);
        uiEntrySetText(g_main_window.prop_value, info);
    }

    ss_sync_property_controls();
}

static void ss_after_state_change(void)
{
    ss_sync_full_ui_state();
    ss_sync_animation_ui_state();
}

static void ss_after_animation_state_change(void)
{
    ss_sync_animation_ui_state();
}

static void ss_sync_full_ui_state(void)
{
    if (g_main_window.grid_checkbox != NULL) {
        uiCheckboxSetChecked(g_main_window.grid_checkbox, g_main_window.editor.document.view_state.show_grid);
    }
    if (g_main_window.grid_menu_item != NULL) {
        uiMenuItemSetChecked(g_main_window.grid_menu_item, g_main_window.editor.document.view_state.show_grid);
    }
    ss_sync_panel_visibility();
    ss_sync_canvas_chrome();
    ss_sync_tool_buttons();
    ss_sync_structure_tabs();
    ss_sync_structure_actions();
    ss_sync_left_section_buttons();
    ss_sync_operation_buttons();
    ss_sync_operation_fields();
    ss_sync_operation_controls();
    ss_sync_operation_hint();
    ss_sync_analysis_options();
    ss_sync_theory_panel();
    ss_sync_properties();
    ss_sync_guide();
}

static void ss_sync_animation_ui_state(void)
{
    ss_sync_analysis_controls();
    ss_refresh_status();
    ss_queue_canvas_refresh();
}

static void ss_try_or_show(int ok, SsError *error)
{
    if (!ok) {
        ss_show_error(error->message);
    }
    ss_after_state_change();
}

static void ss_canvas_insert(double x, double y)
{
    const SsStructure *before = ss_document_active_structure_const(&g_main_window.editor.document);
    char previous_ids[256][SS_ID_CAPACITY];
    size_t previous_count = ss_capture_node_ids(before, previous_ids, 256);
    char *primary = ss_entry_dup(g_main_window.primary_input);
    char *secondary = ss_entry_dup(g_main_window.secondary_input);
    int numeric = ss_entry_int(g_main_window.numeric_input);
    char auto_primary[32];
    const char *effective_primary = primary;
    SsError error;

    if (before == NULL) {
        uiFreeText(primary);
        uiFreeText(secondary);
        return;
    }

    ss_error_clear(&error);
    if (before->family == SS_FAMILY_VECTOR && primary[0] == '\0') {
        snprintf(auto_primary, sizeof(auto_primary), "%zu", before->node_count);
        effective_primary = auto_primary;
    }

    if (ss_editor_apply_primary(&g_main_window.editor, effective_primary, secondary, numeric, &error)) {
        SsStructure *after = ss_document_active_structure(&g_main_window.editor.document);
        if (after != NULL) {
            SsNode *new_node = ss_find_new_node(after, previous_ids, previous_count);
            /* Manual placement is applied only to free-form or quasi-free-form
             * families; tree-like structures keep their layout semantics. */
            if (new_node != NULL) {
                if (after->family == SS_FAMILY_GRAPH ||
                    after->family == SS_FAMILY_LIST ||
                    after->family == SS_FAMILY_STACK ||
                    after->family == SS_FAMILY_QUEUE ||
                    after->family == SS_FAMILY_SET ||
                    after->family == SS_FAMILY_MAP) {
                    ss_position_canvas_node(after, new_node, x, y);
                    /* Canvas-driven placement is already an intentional visual
                     * action, so we drop the semantic transition captured
                     * before insertion to avoid animating toward stale
                     * intermediate coordinates. */
                    ss_editor_clear_layout_transition(&g_main_window.editor);
                }
                ss_editor_select_node(&g_main_window.editor, new_node->id);
            }
        }
    } else {
        ss_show_error(error.message);
    }

    uiFreeText(primary);
    uiFreeText(secondary);
    ss_after_state_change();
}

static void ss_do_primary(uiButton *button, void *data)
{
    SsError error;
    char *primary = ss_entry_dup(g_main_window.primary_input);
    char *secondary = ss_entry_dup(g_main_window.secondary_input);
    int numeric = ss_entry_int(g_main_window.numeric_input);
    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_apply_primary(&g_main_window.editor, primary, secondary, numeric, &error), &error);
    uiFreeText(primary);
    uiFreeText(secondary);
}

static void ss_do_secondary(uiButton *button, void *data)
{
    SsError error;
    char *primary = ss_entry_dup(g_main_window.primary_input);
    char *secondary = ss_entry_dup(g_main_window.secondary_input);
    int numeric = ss_entry_int(g_main_window.numeric_input);
    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_apply_secondary(&g_main_window.editor, primary, secondary, numeric, &error), &error);
    uiFreeText(primary);
    uiFreeText(secondary);
}

static void ss_do_tertiary(uiButton *button, void *data)
{
    SsError error;
    char *primary = ss_entry_dup(g_main_window.primary_input);
    char *secondary = ss_entry_dup(g_main_window.secondary_input);
    int numeric = ss_entry_int(g_main_window.numeric_input);
    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_apply_tertiary(&g_main_window.editor, primary, secondary, numeric, &error), &error);
    uiFreeText(primary);
    uiFreeText(secondary);
}

static void ss_run_analysis(uiButton *button, void *data)
{
    SsError error;
    SsAnalysisKind kind = ss_selected_analysis_kind();
    char *start_token = ss_entry_dup(g_main_window.analysis_input);

    (void) button;
    (void) data;

    ss_error_clear(&error);
    if (kind == SS_ANALYSIS_NONE) {
        ss_error_set(&error, SS_ERROR_INVALID_STATE, "No hay un analisis disponible para la estructura activa.");
        ss_try_or_show(0, &error);
        uiFreeText(start_token);
        return;
    }

    ss_try_or_show(ss_editor_run_analysis(&g_main_window.editor, kind, start_token, &error), &error);
    uiFreeText(start_token);
}

static void ss_prepare_analysis(uiButton *button, void *data)
{
    SsError error;
    SsAnalysisKind kind = ss_selected_analysis_kind();
    char *start_token = ss_entry_dup(g_main_window.analysis_input);
    int ok;

    (void) button;
    (void) data;

    ss_error_clear(&error);
    if (kind == SS_ANALYSIS_NONE) {
        ss_error_set(&error, SS_ERROR_INVALID_STATE, "No hay un analisis disponible para preparar.");
        ss_try_or_show(0, &error);
        uiFreeText(start_token);
        return;
    }

    ok = ss_editor_prepare_analysis_playback(&g_main_window.editor, kind, start_token, &error);
    if (ok && ss_editor_analysis_playback_has_steps(&g_main_window.editor)) {
        ss_editor_set_playback_autoplay(&g_main_window.editor, 1);
        ss_editor_set_status(&g_main_window.editor, "Simulacion automatica iniciada.");
    }
    ss_try_or_show(ok, &error);
    uiFreeText(start_token);
}

static void ss_analysis_autoplay(uiButton *button, void *data)
{
    int enable;

    (void) button;
    (void) data;
    if (!ss_editor_analysis_playback_has_steps(&g_main_window.editor)) {
        return;
    }

    enable = !ss_editor_playback_autoplay_enabled(&g_main_window.editor);
    ss_editor_set_playback_autoplay(&g_main_window.editor, enable);
    ss_editor_set_status(
        &g_main_window.editor,
        enable ? "Reproducción automática activada." : "Reproducción automática en pausa.");
    ss_after_state_change();
}

static void ss_analysis_previous(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    if (ss_editor_analysis_playback_previous(&g_main_window.editor)) {
        ss_after_state_change();
    }
}

static void ss_analysis_next(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    if (ss_editor_analysis_playback_next(&g_main_window.editor)) {
        ss_after_state_change();
    }
}

static void ss_analysis_reset(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_editor_reset_analysis_playback(&g_main_window.editor);
    ss_after_state_change();
}

static void ss_generate_analysis_tree(uiButton *button, void *data)
{
    SsError error;
    SsAnalysisKind kind = ss_selected_analysis_kind();
    char *start_token = ss_entry_dup(g_main_window.analysis_input);

    (void) button;
    (void) data;

    ss_error_clear(&error);
    if (kind == SS_ANALYSIS_NONE) {
        ss_error_set(&error, SS_ERROR_INVALID_STATE, "No hay un analisis activo para derivar un arbol.");
        ss_try_or_show(0, &error);
        uiFreeText(start_token);
        return;
    }

    ss_try_or_show(ss_editor_generate_graph_tree(&g_main_window.editor, kind, start_token, &error), &error);
    uiFreeText(start_token);
}

static void ss_rotate_left(uiButton *button, void *data)
{
    SsError error;
    const SsStructure *structure;

    (void) button;
    (void) data;
    ss_error_clear(&error);
    structure = ss_document_active_structure_const(&g_main_window.editor.document);
    if (ss_structure_supports_graph_rotation(structure)) {
        ss_try_or_show(ss_editor_rotate_graph_left(&g_main_window.editor, &error), &error);
        return;
    }
    ss_try_or_show(ss_editor_rotate_selection_left(&g_main_window.editor, &error), &error);
}

static void ss_rotate_right(uiButton *button, void *data)
{
    SsError error;
    const SsStructure *structure;

    (void) button;
    (void) data;
    ss_error_clear(&error);
    structure = ss_document_active_structure_const(&g_main_window.editor.document);
    if (ss_structure_supports_graph_rotation(structure)) {
        ss_try_or_show(ss_editor_rotate_graph_right(&g_main_window.editor, &error), &error);
        return;
    }
    ss_try_or_show(ss_editor_rotate_selection_right(&g_main_window.editor, &error), &error);
}

static void ss_undo_rotation(uiButton *button, void *data)
{
    SsError error;

    (void) button;
    (void) data;
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_undo_rotation(&g_main_window.editor, &error), &error);
}

static void ss_redo_rotation(uiButton *button, void *data)
{
    SsError error;

    (void) button;
    (void) data;
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_redo_rotation(&g_main_window.editor, &error), &error);
}

static void ss_undo_rotation_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) item;
    (void) window;
    (void) data;
    ss_undo_rotation(NULL, NULL);
}

static void ss_redo_rotation_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) item;
    (void) window;
    (void) data;
    ss_redo_rotation(NULL, NULL);
}

static void ss_reset_canvas_view(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_editor_reset_canvas_view(&g_main_window.editor);
    ss_after_state_change();
}

static void ss_toggle_optional_numeric(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    g_main_window.show_optional_numeric_fields = !g_main_window.show_optional_numeric_fields;
    ss_editor_set_status(
        &g_main_window.editor,
        g_main_window.show_optional_numeric_fields ? "Parámetro opcional visible." : "Parámetro opcional oculto.");
    ss_after_state_change();
}

static void ss_show_operations_section(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    g_main_window.left_section_mode = SS_LEFT_SECTION_OPERATIONS;
    ss_after_state_change();
}

static void ss_show_analysis_section(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    g_main_window.left_section_mode = SS_LEFT_SECTION_ANALYSIS;
    ss_after_state_change();
}

static void ss_show_guide_section(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    g_main_window.left_section_mode = SS_LEFT_SECTION_GUIDE;
    ss_after_state_change();
}

static void ss_clear_operation_inputs(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_clear_entry(g_main_window.primary_input);
    ss_clear_entry(g_main_window.secondary_input);
    ss_clear_entry(g_main_window.numeric_input);
    ss_editor_set_status(&g_main_window.editor, "Parámetros de la operación limpiados.");
    ss_after_state_change();
}

static void ss_focus_canvas(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_set_workspace_panels(1, 0, "Modo lienzo + herramientas activado.");
}

static void ss_toggle_right_panel_button(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_set_workspace_panels(1, 1, "Modo lienzo + herramientas + teoría activado.");
}

static void ss_apply_properties(uiButton *button, void *data)
{
    SsError error;
    char *label = ss_entry_dup(g_main_window.prop_label);
    char *value = ss_entry_dup(g_main_window.prop_value);
    char *secondary = ss_entry_dup(g_main_window.prop_secondary);
    int numeric = ss_entry_int(g_main_window.prop_numeric);
    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_apply_property_update(&g_main_window.editor, label, value, secondary, numeric, &error), &error);
    uiFreeText(label);
    uiFreeText(value);
    uiFreeText(secondary);
}

static void ss_set_tool_select(uiButton *button, void *data) { (void) button; (void) data; ss_editor_set_tool(&g_main_window.editor, SS_TOOL_SELECT); ss_editor_set_status(&g_main_window.editor, "Herramienta: seleccionar"); ss_after_state_change(); }
static void ss_set_tool_insert(uiButton *button, void *data) { (void) button; (void) data; ss_editor_set_tool(&g_main_window.editor, SS_TOOL_INSERT); ss_editor_set_status(&g_main_window.editor, "Herramienta: insertar"); ss_after_state_change(); }
static void ss_set_tool_connect(uiButton *button, void *data) { (void) button; (void) data; ss_editor_set_tool(&g_main_window.editor, SS_TOOL_CONNECT); ss_editor_set_status(&g_main_window.editor, "Herramienta: conectar"); ss_after_state_change(); }
static void ss_set_tool_delete(uiButton *button, void *data) { (void) button; (void) data; ss_editor_set_tool(&g_main_window.editor, SS_TOOL_DELETE); ss_editor_set_status(&g_main_window.editor, "Herramienta: eliminar"); ss_after_state_change(); }

static void ss_analysis_selected(uiCombobox *combo, void *data)
{
    SsAnalysisKind kind;

    (void) combo;
    (void) data;

    if (g_main_window.syncing) {
        return;
    }

    kind = ss_selected_analysis_kind();
    ss_editor_clear_analysis(&g_main_window.editor);
    if (kind != SS_ANALYSIS_NONE) {
        ss_editor_set_status(&g_main_window.editor, "Análisis seleccionado: %s", ss_analysis_kind_label(kind));
    }
    ss_after_state_change();
}

static void ss_new_structure(uiButton *button, void *data)
{
    SsError error;
    const SsVariantDescriptor *descriptor = ss_variant_descriptor_at((size_t) uiComboboxSelected(g_main_window.variant_combo));
    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_add_structure(&g_main_window.editor, descriptor->variant, &error), &error);
}

static void ss_replace_structure(uiButton *button, void *data)
{
    SsError error;
    const SsVariantDescriptor *descriptor = ss_variant_descriptor_at((size_t) uiComboboxSelected(g_main_window.variant_combo));
    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_replace_active_structure(&g_main_window.editor, descriptor->variant, &error), &error);
}

static void ss_load_example(uiButton *button, void *data)
{
    SsError error;

    (void) button;
    (void) data;

    ss_error_clear(&error);
    ss_try_or_show(ss_editor_load_example(&g_main_window.editor, &error), &error);
}

static void ss_validate_clicked(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_editor_validate(&g_main_window.editor);
    ss_editor_set_status(&g_main_window.editor, "Validación ejecutada.");
    ss_after_state_change();
}

static void ss_auto_layout_clicked(uiButton *button, void *data)
{
    (void) button;
    (void) data;
    ss_editor_auto_layout(&g_main_window.editor, 720.0);
    ss_after_state_change();
}

static void ss_new_document_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    SsError error;
    (void) item;
    (void) window;
    (void) data;
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_new_document(&g_main_window.editor, "StructStudio C", SS_VARIANT_VECTOR, &error), &error);
}

static void ss_open_document_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    SsError error;
    char *path;
    (void) item;
    (void) data;
    path = uiOpenFile(window);
    if (path == NULL) {
        return;
    }
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_load_document(&g_main_window.editor, path, &error), &error);
    uiFreeText(path);
}

static void ss_save_document_internal(int save_as)
{
    SsError error;
    char *path = NULL;

    if (!save_as && g_main_window.editor.document.save_path[0] != '\0') {
        ss_error_clear(&error);
        ss_try_or_show(ss_editor_save_document(&g_main_window.editor, g_main_window.editor.document.save_path, &error), &error);
        return;
    }

    path = uiSaveFile(g_main_window.window);
    if (path == NULL) {
        return;
    }
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_save_document(&g_main_window.editor, path, &error), &error);
    uiFreeText(path);
}

static void ss_save_document_menu(uiMenuItem *item, uiWindow *window, void *data) { (void) item; (void) window; (void) data; ss_save_document_internal(0); }
static void ss_save_as_menu(uiMenuItem *item, uiWindow *window, void *data) { (void) item; (void) window; (void) data; ss_save_document_internal(1); }

static void ss_export_png_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    SsError error;
    char *path;
    (void) item;
    (void) data;
    path = uiSaveFile(window);
    if (path == NULL) {
        return;
    }
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_export_png(&g_main_window.editor, path, 1280, 720, &error), &error);
    uiFreeText(path);
}

static void ss_delete_selection_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    SsError error;
    (void) item;
    (void) window;
    (void) data;
    ss_error_clear(&error);
    ss_editor_delete_selection(&g_main_window.editor, &error);
    if (!ss_error_is_ok(&error)) {
        ss_show_error(error.message);
    }
    ss_after_state_change();
}

static void ss_deselect_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) item;
    (void) window;
    (void) data;
    ss_editor_cancel_connection(&g_main_window.editor);
    ss_editor_clear_selection(&g_main_window.editor);
    ss_editor_set_status(&g_main_window.editor, "Seleccion limpiada.");
    ss_after_state_change();
}

static void ss_clear_structure_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    SsError error;

    (void) item;
    (void) window;
    (void) data;
    ss_error_clear(&error);
    ss_try_or_show(ss_editor_clear_active_structure(&g_main_window.editor, &error), &error);
}

static void ss_toggle_grid_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) window;
    (void) data;
    ss_set_grid_visible(uiMenuItemChecked(item));
}

static void ss_canvas_only_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) item;
    (void) window;
    (void) data;
    ss_set_workspace_panels(1, 0, "Modo lienzo + herramientas activado.");
}

static void ss_right_workspace_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) item;
    (void) window;
    (void) data;
    ss_set_workspace_panels(1, 1, "Modo lienzo + herramientas + teoría activado.");
}

static void ss_grid_checkbox_toggled(uiCheckbox *checkbox, void *data)
{
    (void) data;
    ss_set_grid_visible(uiCheckboxChecked(checkbox));
}

static void ss_document_info_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    char buffer[768];
    char line[256];
    const SsDocument *document = &g_main_window.editor.document;
    const SsStructure *structure = ss_document_active_structure_const(document);

    (void) item;
    (void) data;
    buffer[0] = '\0';
    snprintf(line, sizeof(line), "Nombre: %s", document->metadata.name);
    ss_append_line(buffer, sizeof(buffer), line);
    snprintf(line, sizeof(line), "ID: %s", document->metadata.document_id);
    ss_append_line(buffer, sizeof(buffer), line);
    snprintf(line, sizeof(line), "Estructuras: %zu", document->structure_count);
    ss_append_line(buffer, sizeof(buffer), line);
    snprintf(line, sizeof(line), "Activa: %s", structure != NULL ? ss_variant_descriptor(structure->variant)->display_name : "Sin estructura");
    ss_append_line(buffer, sizeof(buffer), line);
    snprintf(line, sizeof(line), "Estado: %s", document->dirty ? "Modificado" : "Sin cambios pendientes");
    ss_append_line(buffer, sizeof(buffer), line);
    ss_append_line(buffer, sizeof(buffer), "Ruta:");
    ss_append_line(buffer, sizeof(buffer), document->save_path[0] != '\0' ? document->save_path : "(sin guardar)");
    snprintf(line, sizeof(line), "Creado: %s", document->metadata.created_at);
    ss_append_line(buffer, sizeof(buffer), line);
    snprintf(line, sizeof(line), "Actualizado: %s", document->metadata.updated_at);
    ss_append_line(buffer, sizeof(buffer), line);
    ss_dialog_show_info(window, "Información del documento", buffer);
}

static void ss_about_menu(uiMenuItem *item, uiWindow *window, void *data)
{
    (void) item;
    (void) data;
    ss_dialog_show_info(window, "StructStudio C", "Editor visual de estructuras de datos en C con estética clásica tipo Windows 7.");
}

static int ss_on_closing(uiWindow *window, void *data)
{
    (void) window;
    (void) data;
    uiQuit();
    return 1;
}

static int ss_on_should_quit(void *data)
{
    (void) data;
    uiControlDestroy(uiControl(g_main_window.window));
    return 1;
}

static void ss_area_draw(uiAreaHandler *handler, uiArea *area, uiAreaDrawParams *params)
{
    (void) handler;
    (void) area;
    ss_render_draw(params, &g_main_window.editor, &g_main_window.theme);
}

static void ss_handle_canvas_click(double x, double y)
{
    SsHitResult hit;
    SsError error;
    const SsStructure *structure = ss_document_active_structure_const(&g_main_window.editor.document);
    double world_x = x;
    double world_y = y;
    int edge_weight = 0;
    int has_edge_weight = 0;

    ss_editor_canvas_to_world(&g_main_window.editor, x, y, &world_x, &world_y);

    ss_render_hit_test(&g_main_window.editor, x, y, &hit);
    ss_error_clear(&error);

    if (g_main_window.editor.tool == SS_TOOL_DELETE) {
        if (hit.type == SS_HIT_NODE) {
            ss_editor_select_node(&g_main_window.editor, hit.id);
            ss_editor_delete_selection(&g_main_window.editor, &error);
        } else if (hit.type == SS_HIT_EDGE) {
            ss_editor_select_edge(&g_main_window.editor, hit.id);
            ss_editor_delete_selection(&g_main_window.editor, &error);
        } else {
            ss_error_set(&error, SS_ERROR_INVALID_STATE, "Nada que eliminar en esa posicion.");
        }
        if (!ss_error_is_ok(&error)) {
            ss_show_error(error.message);
        }
        ss_after_state_change();
        return;
    }

    if (hit.type == SS_HIT_NODE) {
        if (g_main_window.editor.tool == SS_TOOL_CONNECT) {
            if (!g_main_window.editor.connection.is_connecting) {
                ss_editor_select_node(&g_main_window.editor, hit.id);
                ss_try_or_show(
                    ss_editor_connect(
                        &g_main_window.editor,
                        hit.id,
                        ss_default_relation_for_variant(structure->variant),
                        (double) ss_entry_int(g_main_window.numeric_input),
                        &error),
                    &error);
            } else {
                const SsNode *source = ss_structure_find_node_const(structure, g_main_window.editor.connection.source_node_id);
                const SsNode *target = ss_structure_find_node_const(structure, hit.id);
                ss_entry_try_int(g_main_window.numeric_input, &edge_weight, &has_edge_weight);
                /* Manual graph connections are one of the few canvas actions that
                 * depend on an auxiliary form field. Validate the weight here so
                 * the user gets immediate feedback from the UI layer instead of a
                 * generic model error after the click sequence. */
                if (structure != NULL && structure->config.is_weighted && (!has_edge_weight || edge_weight == 0)) {
                    ss_show_error("Escribe un peso valido antes de conectar la arista.");
                    return;
                }
                if (ss_editor_connect(
                        &g_main_window.editor,
                        hit.id,
                        ss_connection_relation_for_click(structure, source, target, x),
                        (double) edge_weight,
                        &error)) {
                    ss_editor_select_node(&g_main_window.editor, hit.id);
                } else {
                    ss_show_error(error.message);
                }
                ss_after_state_change();
            }
            return;
        }
        ss_editor_select_node(&g_main_window.editor, hit.id);
        ss_editor_begin_drag(&g_main_window.editor, hit.id, world_x, world_y);
    } else if (hit.type == SS_HIT_EDGE) {
        ss_editor_select_edge(&g_main_window.editor, hit.id);
    } else {
        if (g_main_window.editor.tool == SS_TOOL_INSERT && structure != NULL) {
            ss_canvas_insert(world_x, world_y);
            return;
        }
        if (g_main_window.editor.tool == SS_TOOL_CONNECT && g_main_window.editor.connection.is_connecting) {
            ss_editor_cancel_connection(&g_main_window.editor);
            ss_editor_set_status(&g_main_window.editor, "Conexion cancelada.");
        }
        ss_editor_clear_selection(&g_main_window.editor);
    }
    ss_after_state_change();
}

static void ss_area_mouse(uiAreaHandler *handler, uiArea *area, uiAreaMouseEvent *event)
{
    int was_dragging;
    int was_panning;
    double world_x = event->X;
    double world_y = event->Y;

    (void) handler;
    (void) area;
    if (ss_mouse_event_starts_pan(event)) {
        ss_editor_begin_pan(&g_main_window.editor, event->X, event->Y);
        return;
    }
    if (event->Down == 1) {
        ss_handle_canvas_click(event->X, event->Y);
        return;
    }
    if (g_main_window.editor.pan.is_panning) {
        ss_editor_pan_to(&g_main_window.editor, event->X, event->Y);
        ss_queue_canvas_refresh();
    }
    ss_editor_canvas_to_world(&g_main_window.editor, event->X, event->Y, &world_x, &world_y);
    if (g_main_window.editor.drag.is_dragging && (event->Held1To64 & 1ULL) != 0) {
        ss_editor_drag_to(&g_main_window.editor, world_x, world_y);
        ss_queue_canvas_refresh();
        return;
    }
    if (g_main_window.editor.tool == SS_TOOL_CONNECT && g_main_window.editor.connection.is_connecting) {
        ss_editor_update_connection_preview(&g_main_window.editor, event->X, event->Y);
        ss_queue_canvas_refresh();
    }
    if (event->Up > 0) {
        was_dragging = g_main_window.editor.drag.is_dragging;
        was_panning = g_main_window.editor.pan.is_panning;
        ss_editor_end_drag(&g_main_window.editor);
        ss_editor_end_pan(&g_main_window.editor);
        if (was_dragging) {
            ss_after_state_change();
        } else if (was_panning) {
            ss_after_state_change();
        }
    }
}

static void ss_area_crossed(uiAreaHandler *handler, uiArea *area, int left) { (void) handler; (void) area; (void) left; }
static void ss_area_drag_broken(uiAreaHandler *handler, uiArea *area)
{
    (void) handler;
    (void) area;
    ss_editor_end_drag(&g_main_window.editor);
    ss_editor_end_pan(&g_main_window.editor);
}
static int ss_area_key(uiAreaHandler *handler, uiArea *area, uiAreaKeyEvent *event)
{
    SsError error;

    (void) handler;
    (void) area;
    if (event->Up) {
        return 0;
    }

    if (event->ExtKey == uiExtKeyEscape) {
        ss_editor_end_drag(&g_main_window.editor);
        ss_editor_end_pan(&g_main_window.editor);
        ss_editor_cancel_connection(&g_main_window.editor);
        ss_editor_clear_selection(&g_main_window.editor);
        ss_editor_set_status(&g_main_window.editor, "Interacción cancelada.");
        ss_after_state_change();
        return 1;
    }

    if (event->ExtKey == uiExtKeyDelete) {
        ss_error_clear(&error);
        ss_editor_delete_selection(&g_main_window.editor, &error);
        if (!ss_error_is_ok(&error)) {
            ss_show_error(error.message);
        }
        ss_after_state_change();
        return 1;
    }

    if (event->ExtKey == uiExtKeyLeft) {
        if (ss_editor_analysis_playback_previous(&g_main_window.editor)) {
            ss_after_state_change();
            return 1;
        }
    }

    if (event->ExtKey == uiExtKeyRight) {
        if (ss_editor_analysis_playback_next(&g_main_window.editor)) {
            ss_after_state_change();
            return 1;
        }
    }

    if (event->Key == ' ' && ss_editor_analysis_playback_has_steps(&g_main_window.editor)) {
        ss_editor_set_playback_autoplay(
            &g_main_window.editor,
            !ss_editor_playback_autoplay_enabled(&g_main_window.editor));
        ss_editor_set_status(
            &g_main_window.editor,
            ss_editor_playback_autoplay_enabled(&g_main_window.editor)
                ? "Reproducción automática activada."
                : "Reproducción automática en pausa.");
        ss_after_state_change();
        return 1;
    }

    return 0;
}

static void ss_build_menus(void)
{
    uiMenu *file = uiNewMenu("Archivo");
    uiMenuItem *item = uiMenuAppendItem(file, "Nuevo");
    uiMenuItemOnClicked(item, ss_new_document_menu, NULL);
    item = uiMenuAppendItem(file, "Abrir");
    uiMenuItemOnClicked(item, ss_open_document_menu, NULL);
    item = uiMenuAppendItem(file, "Guardar");
    uiMenuItemOnClicked(item, ss_save_document_menu, NULL);
    item = uiMenuAppendItem(file, "Guardar como");
    uiMenuItemOnClicked(item, ss_save_as_menu, NULL);
    item = uiMenuAppendItem(file, "Exportar PNG");
    uiMenuItemOnClicked(item, ss_export_png_menu, NULL);
    uiMenuAppendSeparator(file);
    uiMenuAppendQuitItem(file);

    uiMenu *edit = uiNewMenu("Editar");
    item = uiMenuAppendItem(edit, "Deseleccionar");
    uiMenuItemOnClicked(item, ss_deselect_menu, NULL);
    item = uiMenuAppendItem(edit, "Deshacer rotacion");
    uiMenuItemOnClicked(item, ss_undo_rotation_menu, NULL);
    item = uiMenuAppendItem(edit, "Rehacer rotacion");
    uiMenuItemOnClicked(item, ss_redo_rotation_menu, NULL);
    item = uiMenuAppendItem(edit, "Eliminar selección");
    uiMenuItemOnClicked(item, ss_delete_selection_menu, NULL);
    item = uiMenuAppendItem(edit, "Limpiar estructura");
    uiMenuItemOnClicked(item, ss_clear_structure_menu, NULL);

    uiMenu *view = uiNewMenu("Ver");
    item = uiMenuAppendItem(view, "Lienzo + herramientas");
    uiMenuItemOnClicked(item, ss_canvas_only_menu, NULL);
    item = uiMenuAppendItem(view, "Lienzo + herramientas + teoría");
    uiMenuItemOnClicked(item, ss_right_workspace_menu, NULL);
    uiMenuAppendSeparator(view);
    g_main_window.grid_menu_item = uiMenuAppendCheckItem(view, "Mostrar grilla");
    uiMenuItemSetChecked(g_main_window.grid_menu_item, g_main_window.editor.document.view_state.show_grid);
    uiMenuItemOnClicked(g_main_window.grid_menu_item, ss_toggle_grid_menu, NULL);

    uiMenu *help = uiNewMenu("Ayuda");
    item = uiMenuAppendItem(help, "Información del documento");
    uiMenuItemOnClicked(item, ss_document_info_menu, NULL);
    item = uiMenuAppendItem(help, "Acerca de");
    uiMenuItemOnClicked(item, ss_about_menu, NULL);
}

static uiControl *ss_build_left_panel(void)
{
    uiBox *box = uiNewVerticalBox();
    uiGroup *group;
    uiBox *tools;
    uiBox *tool_row_one;
    uiBox *tool_row_two;
    uiBox *tool_rotation_row;
    uiBox *tool_history_row;
    uiForm *form;
    uiButton *button;

    uiBoxSetPadded(box, 1);

    group = uiNewGroup("Estructura");
    uiGroupSetMargined(group, 1);
    form = uiNewForm();
    uiFormSetPadded(form, 1);
    g_main_window.variant_combo = uiNewCombobox();
    for (size_t index = 0; index < ss_variant_descriptor_count(); ++index) {
        uiComboboxAppend(g_main_window.variant_combo, ss_variant_descriptor_at(index)->display_name);
    }
    uiComboboxSetSelected(g_main_window.variant_combo, 0);
    uiFormAppend(form, "", uiControl(uiNewLabel("La activa se cambia desde las pestañas.")), 0);
    uiFormAppend(form, "Nueva", uiControl(g_main_window.variant_combo), 0);
    button = uiNewButton("Nueva");
    uiButtonOnClicked(button, ss_new_structure, NULL);
    uiFormAppend(form, "", uiControl(button), 0);
    button = uiNewButton("Reiniciar");
    uiButtonOnClicked(button, ss_replace_structure, NULL);
    uiFormAppend(form, "", uiControl(button), 0);
    g_main_window.load_example_button = uiNewButton("Cargar ejemplo");
    uiButtonOnClicked(g_main_window.load_example_button, ss_load_example, NULL);
    uiFormAppend(form, "", uiControl(g_main_window.load_example_button), 0);
    uiGroupSetChild(group, uiControl(form));
    uiBoxAppend(box, uiControl(group), 0);

    group = uiNewGroup("Herramientas");
    uiGroupSetMargined(group, 1);
    tools = uiNewVerticalBox();
    tool_row_one = uiNewHorizontalBox();
    tool_row_two = uiNewHorizontalBox();
    tool_rotation_row = uiNewHorizontalBox();
    tool_history_row = uiNewHorizontalBox();
    uiBoxSetPadded(tools, 1);
    uiBoxSetPadded(tool_row_one, 1);
    uiBoxSetPadded(tool_row_two, 1);
    uiBoxSetPadded(tool_rotation_row, 1);
    uiBoxSetPadded(tool_history_row, 1);
    g_main_window.tool_select_button = uiNewButton("Seleccionar");
    uiButtonOnClicked(g_main_window.tool_select_button, ss_set_tool_select, NULL);
    uiBoxAppend(tool_row_one, uiControl(g_main_window.tool_select_button), 1);
    g_main_window.tool_insert_button = uiNewButton("Insertar");
    uiButtonOnClicked(g_main_window.tool_insert_button, ss_set_tool_insert, NULL);
    uiBoxAppend(tool_row_one, uiControl(g_main_window.tool_insert_button), 1);
    g_main_window.tool_connect_button = uiNewButton("Conectar");
    uiButtonOnClicked(g_main_window.tool_connect_button, ss_set_tool_connect, NULL);
    uiBoxAppend(tool_row_two, uiControl(g_main_window.tool_connect_button), 1);
    g_main_window.tool_delete_button = uiNewButton("Eliminar");
    uiButtonOnClicked(g_main_window.tool_delete_button, ss_set_tool_delete, NULL);
    uiBoxAppend(tool_row_two, uiControl(g_main_window.tool_delete_button), 1);
    g_main_window.rotate_left_button = uiNewButton("Rot. izq");
    g_main_window.rotate_right_button = uiNewButton("Rot. der");
    g_main_window.undo_rotation_button = uiNewButton("Deshacer");
    g_main_window.redo_rotation_button = uiNewButton("Rehacer");
    uiButtonOnClicked(g_main_window.rotate_left_button, ss_rotate_left, NULL);
    uiButtonOnClicked(g_main_window.rotate_right_button, ss_rotate_right, NULL);
    uiButtonOnClicked(g_main_window.undo_rotation_button, ss_undo_rotation, NULL);
    uiButtonOnClicked(g_main_window.redo_rotation_button, ss_redo_rotation, NULL);
    uiBoxAppend(tool_rotation_row, uiControl(g_main_window.rotate_left_button), 1);
    uiBoxAppend(tool_rotation_row, uiControl(g_main_window.rotate_right_button), 1);
    uiBoxAppend(tool_history_row, uiControl(g_main_window.undo_rotation_button), 1);
    uiBoxAppend(tool_history_row, uiControl(g_main_window.redo_rotation_button), 1);
    uiBoxAppend(tools, uiControl(tool_row_one), 0);
    uiBoxAppend(tools, uiControl(tool_row_two), 0);
    uiBoxAppend(tools, uiControl(tool_rotation_row), 0);
    uiBoxAppend(tools, uiControl(tool_history_row), 0);
    g_main_window.grid_checkbox = uiNewCheckbox("Mostrar grilla");
    uiCheckboxOnToggled(g_main_window.grid_checkbox, ss_grid_checkbox_toggled, NULL);
    uiBoxAppend(tools, uiControl(g_main_window.grid_checkbox), 0);
    g_main_window.reset_view_button = uiNewButton("Recentrar vista");
    uiButtonOnClicked(g_main_window.reset_view_button, ss_reset_canvas_view, NULL);
    uiBoxAppend(tools, uiControl(g_main_window.reset_view_button), 0);
    uiGroupSetChild(group, uiControl(tools));
    uiBoxAppend(box, uiControl(group), 0);

    {
        uiBox *section_switcher = uiNewHorizontalBox();
        uiBoxSetPadded(section_switcher, 1);
        g_main_window.show_operations_button = uiNewButton("Operaciones");
        g_main_window.show_analysis_button = uiNewButton("Análisis");
        g_main_window.show_guide_button = uiNewButton("Guía");
        uiButtonOnClicked(g_main_window.show_operations_button, ss_show_operations_section, NULL);
        uiButtonOnClicked(g_main_window.show_analysis_button, ss_show_analysis_section, NULL);
        uiButtonOnClicked(g_main_window.show_guide_button, ss_show_guide_section, NULL);
        uiBoxAppend(section_switcher, uiControl(g_main_window.show_operations_button), 1);
        uiBoxAppend(section_switcher, uiControl(g_main_window.show_analysis_button), 1);
        uiBoxAppend(section_switcher, uiControl(g_main_window.show_guide_button), 1);
        uiBoxAppend(box, uiControl(section_switcher), 0);
    }

    group = uiNewGroup("Operaciones");
    uiGroupSetMargined(group, 1);
    uiBox *operations_box = uiNewVerticalBox();
    uiBox *parameter_actions = uiNewHorizontalBox();
    uiBox *action_row_one = uiNewHorizontalBox();
    uiBox *action_row_two = uiNewHorizontalBox();
    uiBox *utility_actions = uiNewHorizontalBox();

    uiBoxSetPadded(operations_box, 1);
    uiBoxSetPadded(parameter_actions, 1);
    uiBoxSetPadded(action_row_one, 1);
    uiBoxSetPadded(action_row_two, 1);
    uiBoxSetPadded(utility_actions, 1);

    g_main_window.operation_hint_label = uiNewLabel("Paso rápido:");
    g_main_window.operation_context_label = uiNewLabel("Selección actual:");
    uiBoxAppend(operations_box, uiControl(g_main_window.operation_hint_label), 0);
    uiBoxAppend(operations_box, uiControl(g_main_window.operation_context_label), 0);

    g_main_window.primary_row = ss_build_labeled_entry_row("Valor", &g_main_window.primary_input_label, &g_main_window.primary_input);
    g_main_window.secondary_row = ss_build_labeled_entry_row("Dato adicional", &g_main_window.secondary_input_label, &g_main_window.secondary_input);
    g_main_window.numeric_row = ss_build_labeled_entry_row("Número", &g_main_window.numeric_input_label, &g_main_window.numeric_input);
    uiBoxAppend(operations_box, uiControl(g_main_window.primary_row), 0);
    uiBoxAppend(operations_box, uiControl(g_main_window.secondary_row), 0);
    uiBoxAppend(operations_box, uiControl(g_main_window.numeric_row), 0);

    g_main_window.clear_operation_inputs_button = uiNewButton("Limpiar campos");
    g_main_window.toggle_optional_numeric_button = uiNewButton("Ver parámetro");
    uiButtonOnClicked(g_main_window.clear_operation_inputs_button, ss_clear_operation_inputs, NULL);
    uiButtonOnClicked(g_main_window.toggle_optional_numeric_button, ss_toggle_optional_numeric, NULL);
    uiBoxAppend(parameter_actions, uiControl(g_main_window.clear_operation_inputs_button), 1);
    uiBoxAppend(parameter_actions, uiControl(g_main_window.toggle_optional_numeric_button), 1);
    uiBoxAppend(operations_box, uiControl(parameter_actions), 0);

    g_main_window.primary_button = uiNewButton("Primaria");
    g_main_window.secondary_button = uiNewButton("Secundaria");
    g_main_window.tertiary_button = uiNewButton("Terciaria");
    uiButtonOnClicked(g_main_window.primary_button, ss_do_primary, NULL);
    uiButtonOnClicked(g_main_window.secondary_button, ss_do_secondary, NULL);
    uiButtonOnClicked(g_main_window.tertiary_button, ss_do_tertiary, NULL);
    uiBoxAppend(action_row_one, uiControl(g_main_window.primary_button), 1);
    uiBoxAppend(action_row_one, uiControl(g_main_window.secondary_button), 1);
    uiBoxAppend(action_row_two, uiControl(g_main_window.tertiary_button), 1);
    uiBoxAppend(operations_box, uiControl(action_row_one), 0);
    uiBoxAppend(operations_box, uiControl(action_row_two), 0);
    button = uiNewButton("Validar");
    uiButtonOnClicked(button, ss_validate_clicked, NULL);
    uiBoxAppend(utility_actions, uiControl(button), 1);
    button = uiNewButton("Auto-layout");
    uiButtonOnClicked(button, ss_auto_layout_clicked, NULL);
    uiBoxAppend(utility_actions, uiControl(button), 1);
    uiBoxAppend(operations_box, uiControl(utility_actions), 0);
    uiGroupSetChild(group, uiControl(operations_box));
    g_main_window.operations_group_control = uiControl(group);
    uiBoxAppend(box, uiControl(group), 0);

    group = uiNewGroup("Análisis");
    uiGroupSetMargined(group, 1);
    form = uiNewForm();
    uiBox *analysis_nav = uiNewHorizontalBox();
    uiFormSetPadded(form, 1);
    g_main_window.analysis_combo = uiNewCombobox();
    uiComboboxOnSelected(g_main_window.analysis_combo, ss_analysis_selected, NULL);
    g_main_window.analysis_input = uiNewEntry();
    g_main_window.analysis_button = uiNewButton("Resumen");
    g_main_window.analysis_prepare_button = uiNewButton("Simular");
    g_main_window.analysis_tree_button = uiNewButton("Arbol derivado");
    g_main_window.analysis_play_button = uiNewButton("Auto");
    g_main_window.analysis_prev_button = uiNewButton("Anterior");
    g_main_window.analysis_next_button = uiNewButton("Siguiente");
    g_main_window.analysis_reset_button = uiNewButton("Reiniciar");
    g_main_window.analysis_step_label = uiNewLabel("");
    uiButtonOnClicked(g_main_window.analysis_button, ss_run_analysis, NULL);
    uiButtonOnClicked(g_main_window.analysis_prepare_button, ss_prepare_analysis, NULL);
    uiButtonOnClicked(g_main_window.analysis_tree_button, ss_generate_analysis_tree, NULL);
    uiButtonOnClicked(g_main_window.analysis_play_button, ss_analysis_autoplay, NULL);
    uiButtonOnClicked(g_main_window.analysis_prev_button, ss_analysis_previous, NULL);
    uiButtonOnClicked(g_main_window.analysis_next_button, ss_analysis_next, NULL);
    uiButtonOnClicked(g_main_window.analysis_reset_button, ss_analysis_reset, NULL);
    uiBoxSetPadded(analysis_nav, 1);
    uiBoxAppend(analysis_nav, uiControl(g_main_window.analysis_play_button), 1);
    uiBoxAppend(analysis_nav, uiControl(g_main_window.analysis_prev_button), 1);
    uiBoxAppend(analysis_nav, uiControl(g_main_window.analysis_next_button), 1);
    uiBoxAppend(analysis_nav, uiControl(g_main_window.analysis_reset_button), 1);
    uiFormAppend(form, "Tipo", uiControl(g_main_window.analysis_combo), 0);
    uiFormAppend(form, "Origen", uiControl(g_main_window.analysis_input), 0);
    uiFormAppend(form, "", uiControl(g_main_window.analysis_prepare_button), 0);
    uiFormAppend(form, "", uiControl(g_main_window.analysis_button), 0);
    uiFormAppend(form, "", uiControl(g_main_window.analysis_tree_button), 0);
    uiFormAppend(form, "", uiControl(analysis_nav), 0);
    uiFormAppend(form, "", uiControl(g_main_window.analysis_step_label), 0);
    uiGroupSetChild(group, uiControl(form));
    g_main_window.analysis_group_control = uiControl(group);
    uiBoxAppend(box, uiControl(group), 0);

    group = uiNewGroup("Guía");
    uiGroupSetMargined(group, 1);
    g_main_window.guide_box = uiNewMultilineEntry();
    uiMultilineEntrySetReadOnly(g_main_window.guide_box, 1);
    uiGroupSetChild(group, uiControl(g_main_window.guide_box));
    g_main_window.guide_group_control = uiControl(group);
    uiBoxAppend(box, uiControl(group), 1);
    g_main_window.left_section_mode = SS_LEFT_SECTION_OPERATIONS;

    return uiControl(box);
}

static uiControl *ss_build_right_panel(void)
{
    uiBox *panel = uiNewVerticalBox();
    uiGroup *group = uiNewGroup("Propiedades");
    uiGroup *theory_group;
    uiBox *theory_host;
    uiForm *form = uiNewForm();

    uiBoxSetPadded(panel, 1);

    uiGroupSetMargined(group, 1);
    uiFormSetPadded(form, 1);

    g_main_window.prop_id = uiNewEntry();
    uiEntrySetReadOnly(g_main_window.prop_id, 1);
    g_main_window.prop_label = uiNewEntry();
    g_main_window.prop_value = uiNewEntry();
    g_main_window.prop_secondary = uiNewEntry();
    g_main_window.prop_numeric = uiNewEntry();

    uiFormAppend(form, "ID", uiControl(g_main_window.prop_id), 0);
    uiFormAppend(form, "Etiqueta", uiControl(g_main_window.prop_label), 0);
    uiFormAppend(form, "Valor", uiControl(g_main_window.prop_value), 0);
    uiFormAppend(form, "Extra", uiControl(g_main_window.prop_secondary), 0);
    uiFormAppend(form, "Número", uiControl(g_main_window.prop_numeric), 0);

    g_main_window.apply_properties_button = uiNewButton("Aplicar cambios");
    uiButtonOnClicked(g_main_window.apply_properties_button, ss_apply_properties, NULL);
    uiFormAppend(form, "", uiControl(g_main_window.apply_properties_button), 0);

    uiGroupSetChild(group, uiControl(form));
    uiBoxAppend(panel, uiControl(group), 0);

    theory_group = uiNewGroup("Teoría y recorridos");
    uiGroupSetMargined(theory_group, 1);
    theory_host = uiNewVerticalBox();
    uiBoxSetPadded(theory_host, 0);
    g_main_window.theory_box = uiNewMultilineEntry();
    uiMultilineEntrySetReadOnly(g_main_window.theory_box, 1);
    uiBoxAppend(theory_host, uiControl(g_main_window.theory_box), 1);
    uiGroupSetChild(theory_group, uiControl(theory_host));
    uiBoxAppend(panel, uiControl(theory_group), 1);

    return uiControl(panel);
}

static uiControl *ss_build_canvas_chrome(void)
{
    uiBox *row = uiNewHorizontalBox();
    uiBox *actions = uiNewHorizontalBox();

    uiBoxSetPadded(row, 1);
    uiBoxSetPadded(actions, 1);

    g_main_window.structure_tabs_host = uiNewVerticalBox();
    uiBoxSetPadded(g_main_window.structure_tabs_host, 0);
    uiBoxAppend(row, uiControl(g_main_window.structure_tabs_host), 1);

    g_main_window.focus_canvas_button = uiNewButton("Lienzo + herramientas");
    g_main_window.toggle_right_panel_button = uiNewButton("Lienzo + herramientas + teoría");
    uiButtonOnClicked(g_main_window.focus_canvas_button, ss_focus_canvas, NULL);
    uiButtonOnClicked(g_main_window.toggle_right_panel_button, ss_toggle_right_panel_button, NULL);
    uiBoxAppend(actions, uiControl(g_main_window.focus_canvas_button), 0);
    uiBoxAppend(actions, uiControl(g_main_window.toggle_right_panel_button), 0);
    uiBoxAppend(row, uiControl(actions), 0);
    return uiControl(row);
}

int ss_main_window_run(void)
{
    uiInitOptions options;
    const char *error_message;
    uiBox *root;
    uiBox *main_row;
    uiBox *canvas_row;
    uiBox *center_panel;
    uiBox *status_box;

    memset(&options, 0, sizeof(options));
    error_message = uiInit(&options);
    if (error_message != NULL) {
        fprintf(stderr, "error initializing libui-ng: %s\n", error_message);
        uiFreeInitError(error_message);
        return 1;
    }

    ss_theme_init(&g_main_window.theme);
    ss_editor_init(&g_main_window.editor);
    g_main_window.area_handler.Draw = ss_area_draw;
    g_main_window.area_handler.MouseEvent = ss_area_mouse;
    g_main_window.area_handler.MouseCrossed = ss_area_crossed;
    g_main_window.area_handler.DragBroken = ss_area_drag_broken;
    g_main_window.area_handler.KeyEvent = ss_area_key;

    ss_build_menus();
    uiOnShouldQuit(ss_on_should_quit, NULL);

    g_main_window.window = uiNewWindow("StructStudio C", 1480, 860, 1);
    uiWindowOnClosing(g_main_window.window, ss_on_closing, NULL);
    uiWindowSetMargined(g_main_window.window, 1);

    root = uiNewVerticalBox();
    uiBoxSetPadded(root, 1);
    uiWindowSetChild(g_main_window.window, uiControl(root));

    main_row = uiNewHorizontalBox();
    g_main_window.main_row = main_row;
    uiBoxSetPadded(main_row, 1);
    uiBoxAppend(root, uiControl(main_row), 1);
    g_main_window.left_panel = ss_build_left_panel();
    g_main_window.left_slot = uiNewVerticalBox();
    uiBoxSetPadded(g_main_window.left_slot, 0);
    uiBoxAppend(g_main_window.left_slot, g_main_window.left_panel, 0);
    g_main_window.left_panel_attached = 1;
    uiBoxAppend(main_row, uiControl(g_main_window.left_slot), 0);
    canvas_row = uiNewHorizontalBox();
    uiBoxSetPadded(canvas_row, 1);
    g_main_window.canvas_row = canvas_row;
    uiBoxAppend(main_row, uiControl(canvas_row), 1);
    center_panel = uiNewVerticalBox();
    uiBoxSetPadded(center_panel, 1);
    g_main_window.center_panel = center_panel;
    uiBoxAppend(center_panel, ss_build_canvas_chrome(), 0);
    g_main_window.canvas = uiNewArea(&g_main_window.area_handler);
    uiBoxAppend(center_panel, uiControl(g_main_window.canvas), 1);
    uiBoxAppend(canvas_row, uiControl(center_panel), 1);
    g_main_window.right_panel = ss_build_right_panel();
    g_main_window.right_slot = uiNewVerticalBox();
    uiBoxSetPadded(g_main_window.right_slot, 0);
    uiBoxAppend(g_main_window.right_slot, g_main_window.right_panel, 1);
    g_main_window.right_panel_attached = 1;
    uiBoxAppend(canvas_row, uiControl(g_main_window.right_slot), 0);

    status_box = uiNewVerticalBox();
    uiBoxSetPadded(status_box, 1);
    g_main_window.status_label = uiNewLabel("");
    uiBoxAppend(status_box, uiControl(g_main_window.status_label), 0);
    uiBoxAppend(root, uiControl(status_box), 0);

    uiTimer(10, ss_animation_timer_tick, NULL);
    ss_after_state_change();
    uiControlShow(uiControl(g_main_window.window));
    uiMain();
    ss_free_structure_tab_cache();
    ss_editor_dispose(&g_main_window.editor);
    uiUninit();
    return 0;
}
