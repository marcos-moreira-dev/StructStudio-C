/*
 * StructStudio C
 * --------------
 * Editor state coordination.
 *
 * The editor owns transient interaction state that should not live in the UI
 * widgets or in the core domain model directly.
 */

#include "editor/editor.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "persistence/document_io.h"
#include "render/render.h"

static const double SS_GRAPH_ROTATION_STEP_RADIANS = 0.2617993877991494;

static void ss_rotation_history_entry_dispose(SsRotationHistoryEntry *entry)
{
    if (entry == NULL) {
        return;
    }

    ss_structure_free(&entry->before_structure);
    ss_structure_free(&entry->after_structure);
    memset(entry, 0, sizeof(*entry));
}

static void ss_rotation_history_clear_stack(SsRotationHistoryEntry *entries, size_t *count)
{
    size_t index;

    if (entries == NULL || count == NULL) {
        return;
    }

    for (index = 0; index < *count; ++index) {
        ss_rotation_history_entry_dispose(&entries[index]);
    }
    *count = 0;
}

static void ss_editor_clear_rotation_history(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    ss_rotation_history_clear_stack(
        editor->rotation_history.undo_entries,
        &editor->rotation_history.undo_count);
    ss_rotation_history_clear_stack(
        editor->rotation_history.redo_entries,
        &editor->rotation_history.redo_count);
}

static void ss_rotation_history_push(
    SsRotationHistoryEntry *entries,
    size_t *count,
    SsRotationHistoryEntry *entry)
{
    if (entries == NULL || count == NULL || entry == NULL) {
        return;
    }

    if (*count >= SS_ROTATION_HISTORY_CAPACITY) {
        ss_rotation_history_entry_dispose(&entries[0]);
        memmove(
            &entries[0],
            &entries[1],
            (SS_ROTATION_HISTORY_CAPACITY - 1) * sizeof(entries[0]));
        *count = SS_ROTATION_HISTORY_CAPACITY - 1;
    }

    entries[*count] = *entry;
    memset(entry, 0, sizeof(*entry));
    (*count)++;
}

static int ss_rotation_history_pop(
    SsRotationHistoryEntry *entries,
    size_t *count,
    SsRotationHistoryEntry *entry)
{
    if (entries == NULL || count == NULL || entry == NULL || *count == 0) {
        return 0;
    }

    *entry = entries[*count - 1];
    memset(&entries[*count - 1], 0, sizeof(entries[*count - 1]));
    (*count)--;
    return 1;
}

static void ss_editor_sync_layout_if_needed(SsStructure *structure)
{
    const SsVariantDescriptor *descriptor;

    if (structure == NULL || !structure->dirty_layout) {
        return;
    }

    descriptor = ss_variant_descriptor(structure->variant);
    if (descriptor->supports_manual_drag) {
        return;
    }

    /* Semantic structures such as lists, trees, heaps or maps should remain
     * visually coherent after each operation without requiring manual relayout. */
    ss_structure_auto_layout(structure, 720.0);
}

static void ss_editor_clear_playback(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    free(editor->playback.steps);
    memset(&editor->playback, 0, sizeof(editor->playback));
    memset(&editor->playback_fx, 0, sizeof(editor->playback_fx));
}

static void ss_editor_capture_layout_before_change(const SsStructure *structure, SsLayoutSnapshot *snapshot)
{
    if (snapshot == NULL) {
        return;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    ss_editor_layout_snapshot_capture(structure, snapshot);
}

static void ss_editor_reset_structure_context(SsEditorState *editor, int clear_selection)
{
    if (editor == NULL) {
        return;
    }

    ss_editor_clear_analysis(editor);
    if (clear_selection) {
        ss_editor_clear_selection(editor);
    }
    ss_editor_validate(editor);
}

static int ss_editor_record_rotation_change(
    SsEditorState *editor,
    size_t structure_index,
    const SsStructure *before_structure,
    const SsStructure *after_structure,
    const char *selection_before,
    const char *selection_after,
    SsError *error)
{
    SsRotationHistoryEntry entry;

    /* Rotation history stores whole-structure snapshots on purpose. That is a
     * little heavier in memory, but much easier to reason about while studying
     * the code and much safer than inventing ad-hoc inverse operations. */
    if (editor == NULL || before_structure == NULL || after_structure == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No se pudo guardar el historial de rotacion.");
        return 0;
    }

    memset(&entry, 0, sizeof(entry));
    entry.structure_index = structure_index;
    ss_str_copy(entry.selection_before_node_id, sizeof(entry.selection_before_node_id), selection_before);
    ss_str_copy(entry.selection_after_node_id, sizeof(entry.selection_after_node_id), selection_after);

    if (!ss_structure_clone(&entry.before_structure, before_structure, error)) {
        ss_rotation_history_entry_dispose(&entry);
        return 0;
    }
    if (!ss_structure_clone(&entry.after_structure, after_structure, error)) {
        ss_rotation_history_entry_dispose(&entry);
        return 0;
    }

    ss_rotation_history_clear_stack(
        editor->rotation_history.redo_entries,
        &editor->rotation_history.redo_count);
    ss_rotation_history_push(
        editor->rotation_history.undo_entries,
        &editor->rotation_history.undo_count,
        &entry);
    ss_error_clear(error);
    return 1;
}

static int ss_editor_restore_rotation_structure(
    SsEditorState *editor,
    const SsRotationHistoryEntry *entry,
    int use_after,
    SsError *error)
{
    SsStructure *structure;
    SsLayoutSnapshot before_snapshot;
    const SsStructure *snapshot;
    const char *selection_id;

    /* Restoring history is still animated. The goal of undo/redo here is not
     * just correctness, but to preserve the teaching value of the transition. */
    if (editor == NULL || entry == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No se pudo restaurar la rotacion.");
        return 0;
    }
    if (entry->structure_index >= editor->document.structure_count) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "La pestaña de la rotacion ya no existe.");
        return 0;
    }

    editor->document.active_structure_index = entry->structure_index;
    structure = ss_document_active_structure(&editor->document);
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa para restaurar.");
        return 0;
    }

    snapshot = use_after ? &entry->after_structure : &entry->before_structure;
    selection_id = use_after ? entry->selection_after_node_id : entry->selection_before_node_id;
    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!ss_structure_clone(structure, snapshot, error)) {
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }

    ss_editor_begin_layout_transition(editor, structure, &before_snapshot, 300);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    ss_document_touch(&editor->document);
    ss_editor_clear_analysis(editor);
    if (selection_id[0] != '\0') {
        ss_editor_select_node(editor, selection_id);
    } else {
        ss_editor_clear_selection(editor);
    }
    ss_editor_validate(editor);
    ss_error_clear(error);
    return 1;
}

static void ss_editor_commit_structure_change(
    SsEditorState *editor,
    SsStructure *structure,
    int clear_selection,
    const SsLayoutSnapshot *before_snapshot,
    int preserve_rotation_history)
{
    if (editor == NULL) {
        return;
    }

    /* Most semantic commands in the editor share this exact lifecycle:
     * normalize layout -> start transition -> mark document dirty ->
     * clear transient analysis/selection context when needed. */
    if (!preserve_rotation_history) {
        ss_editor_clear_rotation_history(editor);
    }
    ss_editor_sync_layout_if_needed(structure);
    ss_editor_begin_layout_transition(editor, structure, before_snapshot, 300);
    ss_document_touch(&editor->document);
    ss_editor_reset_structure_context(editor, clear_selection);
}

static int ss_editor_append_built_structure(
    SsEditorState *editor,
    SsStructure *built,
    const char *status_message,
    SsError *error)
{
    SsDocument *document;
    SsStructure *slot;

    /* Derived structures are appended as new tabs so the source remains
     * visible for comparison. This is pedagogically better than replacing it. */
    if (editor == NULL || built == NULL) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "No se pudo anexar la estructura derivada.");
        return 0;
    }

    document = &editor->document;
    if (!ss_array_reserve(
            (void **) &document->structures,
            sizeof(*document->structures),
            &document->structure_capacity,
            document->structure_count + 1)) {
        ss_error_set(error, SS_ERROR_MEMORY, "No se pudo reservar memoria para la nueva estructura.");
        return 0;
    }

    slot = &document->structures[document->structure_count];
    *slot = *built;
    memset(built, 0, sizeof(*built));
    document->active_structure_index = document->structure_count;
    document->structure_count += 1;
    editor->document.view_state.canvas_offset_x = 0.0;
    editor->document.view_state.canvas_offset_y = 0.0;
    ss_editor_clear_rotation_history(editor);
    ss_editor_clear_layout_transition(editor);
    ss_document_touch(document);
    ss_editor_reset_structure_context(editor, 1);
    ss_editor_set_status(editor, "%s", status_message != NULL ? status_message : "Nueva estructura agregada.");
    ss_error_clear(error);
    return 1;
}

static int ss_editor_resolve_analysis_start(
    const SsEditorState *editor,
    SsAnalysisKind kind,
    const char *start_token,
    char *start_id,
    size_t start_id_capacity,
    SsError *error)
{
    const SsStructure *structure;
    const SsNode *start_node = NULL;
    SsAnalysisStartMode start_mode;

    if (start_id == NULL || start_id_capacity == 0) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Salida invalida para el origen del analisis.");
        return 0;
    }

    start_id[0] = '\0';
    if (editor == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay editor activo.");
        return 0;
    }

    structure = ss_document_active_structure_const(&editor->document);
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    start_mode = ss_analysis_start_mode(kind);
    if (start_mode != SS_ANALYSIS_START_NONE && editor->selection.type == SS_SELECTION_NODE) {
        start_node = ss_structure_find_node_const(structure, editor->selection.node_id);
    } else if (start_mode != SS_ANALYSIS_START_NONE && start_token != NULL && start_token[0] != '\0') {
        start_node = ss_structure_find_node_by_token_const(structure, start_token, error);
        if (start_node == NULL) {
            if (!ss_error_is_ok(error)) {
                return 0;
            }
            ss_error_set(error, SS_ERROR_NOT_FOUND, "No se encontro el nodo indicado para el analisis.");
            return 0;
        }
    } else if (start_mode == SS_ANALYSIS_START_OPTIONAL && structure->root_id[0] != '\0') {
        start_node = ss_structure_find_node_const(structure, structure->root_id);
    }

    if (start_node == NULL && start_mode == SS_ANALYSIS_START_REQUIRED) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo origen o escriba su ID, etiqueta o valor.");
        return 0;
    }

    if (start_node != NULL) {
        ss_str_copy(start_id, start_id_capacity, start_node->id);
    }

    ss_error_clear(error);
    return 1;
}

static void ss_editor_set_playback_status(SsEditorState *editor, const char *prefix)
{
    const SsAnalysisStep *step = ss_editor_current_analysis_step(editor);

    if (editor == NULL) {
        return;
    }
    if (step == NULL) {
        ss_editor_set_status(editor, "%s", prefix != NULL ? prefix : "Recorrido guiado.");
        return;
    }

    ss_editor_set_status(
        editor,
        "%s %s (%zu/%zu): %s",
        prefix != NULL ? prefix : "Recorrido",
        ss_analysis_kind_label(editor->playback.kind),
        editor->playback.current_step + 1,
        editor->playback.step_count,
        step->message);
}

void ss_editor_init(SsEditorState *editor)
{
    memset(editor, 0, sizeof(*editor));
    editor->tool = SS_TOOL_SELECT;
    ss_editor_new_document(editor, "StructStudio C", SS_VARIANT_VECTOR, NULL);
}

void ss_editor_dispose(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }
    ss_editor_clear_rotation_history(editor);
    ss_editor_clear_playback(editor);
    ss_editor_clear_layout_transition(editor);
    ss_document_free(&editor->document);
    memset(editor, 0, sizeof(*editor));
}

void ss_editor_set_status(SsEditorState *editor, const char *fmt, ...)
{
    va_list args;

    if (editor == NULL || fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(editor->status, sizeof(editor->status), fmt, args);
    va_end(args);
}

void ss_editor_clear_analysis(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    ss_editor_clear_playback(editor);
    editor->analysis_report[0] = '\0';
}

void ss_editor_clear_selection(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }
    memset(&editor->selection, 0, sizeof(editor->selection));
    memset(&editor->connection, 0, sizeof(editor->connection));
}

void ss_editor_select_node(SsEditorState *editor, const char *node_id)
{
    ss_editor_clear_selection(editor);
    editor->selection.type = SS_SELECTION_NODE;
    ss_str_copy(editor->selection.node_id, sizeof(editor->selection.node_id), node_id);
}

void ss_editor_select_edge(SsEditorState *editor, const char *edge_id)
{
    ss_editor_clear_selection(editor);
    editor->selection.type = SS_SELECTION_EDGE;
    ss_str_copy(editor->selection.edge_id, sizeof(editor->selection.edge_id), edge_id);
}

int ss_editor_new_document(SsEditorState *editor, const char *name, SsVariant variant, SsError *error)
{
    SsError local_error;
    SsStructure *structure;

    if (error == NULL) {
        error = &local_error;
    }

    ss_document_free(&editor->document);
    ss_editor_clear_rotation_history(editor);
    ss_document_init(&editor->document, name);
    structure = ss_document_add_structure(&editor->document, variant, NULL, error);
    if (structure == NULL) {
        return 0;
    }
    ss_structure_auto_layout(structure, 720.0);
    ss_editor_clear_rotation_history(editor);
    ss_editor_clear_layout_transition(editor);
    ss_editor_clear_rotation_history(editor);
    ss_editor_reset_structure_context(editor, 1);
    ss_editor_set_status(editor, "Documento nuevo creado: %s", editor->document.metadata.name);
    return 1;
}

int ss_editor_add_structure(SsEditorState *editor, SsVariant variant, SsError *error)
{
    SsStructure *structure = ss_document_add_structure(&editor->document, variant, NULL, error);
    if (structure == NULL) {
        return 0;
    }
    ss_structure_auto_layout(structure, 720.0);
    ss_editor_clear_rotation_history(editor);
    ss_editor_clear_layout_transition(editor);
    ss_editor_reset_structure_context(editor, 1);
    ss_editor_set_status(editor, "Nueva pestana abierta: %s", ss_variant_descriptor(variant)->display_name);
    return 1;
}

int ss_editor_replace_active_structure(SsEditorState *editor, SsVariant variant, SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    char structure_id[SS_ID_CAPACITY];

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    ss_str_copy(structure_id, sizeof(structure_id), structure->id);
    ss_structure_free(structure);
    ss_structure_init(structure, variant, structure_id);
    ss_structure_auto_layout(structure, 720.0);
    ss_editor_clear_rotation_history(editor);
    ss_editor_clear_layout_transition(editor);
    ss_document_touch(&editor->document);
    ss_editor_reset_structure_context(editor, 1);
    ss_editor_set_status(editor, "Pestana activa reinicializada como %s", ss_variant_descriptor(variant)->display_name);
    return 1;
}

int ss_editor_activate_structure(SsEditorState *editor, size_t index, SsError *error)
{
    if (!ss_document_set_active_structure(&editor->document, index, error)) {
        return 0;
    }
    ss_editor_clear_layout_transition(editor);
    ss_editor_reset_structure_context(editor, 1);
    ss_editor_set_status(editor, "Pestana activa cambiada.");
    return 1;
}

int ss_editor_clear_active_structure(SsEditorState *editor, SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    SsLayoutSnapshot before_snapshot;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    ss_structure_clear(structure);
    ss_editor_commit_structure_change(editor, structure, 1, &before_snapshot, 0);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    ss_editor_set_status(editor, "Pestana activa limpiada.");
    return 1;
}

int ss_editor_load_example(SsEditorState *editor, SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    char message[SS_MESSAGE_CAPACITY];
    SsLayoutSnapshot before_snapshot;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!ss_structure_load_example(structure, message, sizeof(message), error)) {
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }

    ss_editor_commit_structure_change(editor, structure, 1, &before_snapshot, 0);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    ss_editor_set_status(editor, "%s", message[0] != '\0' ? message : "Ejemplo cargado.");
    return 1;
}

int ss_editor_generate_graph_tree(SsEditorState *editor, SsAnalysisKind kind, const char *start_token, SsError *error)
{
    const SsStructure *source;
    SsStructure derived;
    char start_id[SS_ID_CAPACITY] = "";
    char message[SS_MESSAGE_CAPACITY] = "";

    if (editor == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay editor activo.");
        return 0;
    }

    if (!ss_analysis_supports_tree_generation(kind)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "El analisis seleccionado no genera un arbol derivado.");
        return 0;
    }

    source = ss_document_active_structure_const(&editor->document);
    if (source == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }
    if (!ss_editor_resolve_analysis_start(editor, kind, start_token, start_id, sizeof(start_id), error)) {
        return 0;
    }

    memset(&derived, 0, sizeof(derived));
    if (!ss_structure_build_graph_tree(source, kind, start_id, &derived, message, sizeof(message), error)) {
        return 0;
    }

    ss_structure_auto_layout(&derived, 720.0);
    if (!ss_editor_append_built_structure(editor, &derived, message, error)) {
        ss_structure_free(&derived);
        return 0;
    }

    return 1;
}

int ss_editor_save_document(SsEditorState *editor, const char *path, SsError *error)
{
    if (!ss_document_save_json(&editor->document, path, error)) {
        return 0;
    }
    ss_str_copy(editor->document.save_path, sizeof(editor->document.save_path), path);
    editor->document.dirty = 0;
    ss_editor_set_status(editor, "Documento guardado en %s", path);
    return 1;
}

int ss_editor_load_document(SsEditorState *editor, const char *path, SsError *error)
{
    ss_document_free(&editor->document);
    if (!ss_document_load_json(&editor->document, path, error)) {
        return 0;
    }
    ss_editor_clear_layout_transition(editor);
    ss_editor_clear_rotation_history(editor);
    ss_editor_reset_structure_context(editor, 1);
    ss_editor_set_status(editor, "Documento cargado desde %s", path);
    return 1;
}

int ss_editor_export_png(SsEditorState *editor, const char *path, int width, int height, SsError *error)
{
    SsTheme theme;

    ss_theme_init(&theme);
    if (!ss_render_export_png(editor, &theme, path, width, height, error)) {
        return 0;
    }
    ss_editor_set_status(editor, "Vista exportada a %s", path);
    return 1;
}

void ss_editor_set_tool(SsEditorState *editor, SsTool tool)
{
    editor->tool = tool;
    if (tool != SS_TOOL_CONNECT) {
        memset(&editor->connection, 0, sizeof(editor->connection));
    }
}

void ss_editor_validate(SsEditorState *editor)
{
    ss_structure_validate(ss_document_active_structure(&editor->document), &editor->validation);
}

void ss_editor_auto_layout(SsEditorState *editor, double canvas_width)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    SsLayoutSnapshot before_snapshot;
    if (structure == NULL) {
        return;
    }
    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    ss_structure_auto_layout(structure, canvas_width);
    ss_editor_clear_rotation_history(editor);
    ss_editor_begin_layout_transition(editor, structure, &before_snapshot, 340);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    ss_document_touch(&editor->document);
    ss_editor_validate(editor);
    ss_editor_set_status(editor, "Auto-layout aplicado.");
}

void ss_editor_set_grid_visible(SsEditorState *editor, int visible)
{
    if (editor == NULL) {
        return;
    }

    editor->document.view_state.show_grid = visible ? 1 : 0;
    ss_editor_set_status(editor, visible ? "Grilla visible." : "Grilla oculta.");
}

int ss_editor_run_analysis(SsEditorState *editor, SsAnalysisKind kind, const char *start_token, SsError *error)
{
    const SsStructure *structure = ss_document_active_structure_const(&editor->document);
    char start_id[SS_ID_CAPACITY] = "";
    char report[SS_ANALYSIS_REPORT_CAPACITY] = "";

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    if (!ss_editor_resolve_analysis_start(editor, kind, start_token, start_id, sizeof(start_id), error)) {
        return 0;
    }
    if (!ss_structure_run_analysis(structure, kind, start_id, report, sizeof(report), error)) {
        return 0;
    }

    ss_editor_clear_analysis(editor);
    ss_str_copy(editor->analysis_report, sizeof(editor->analysis_report), report);
    ss_editor_set_status(editor, "Resumen textual generado: %s", ss_analysis_kind_label(kind));
    return 1;
}

int ss_editor_prepare_analysis_playback(SsEditorState *editor, SsAnalysisKind kind, const char *start_token, SsError *error)
{
    const SsStructure *structure = ss_document_active_structure_const(&editor->document);
    SsAnalysisStep *steps = NULL;
    size_t step_count = 0;
    char start_id[SS_ID_CAPACITY] = "";
    char report[SS_ANALYSIS_REPORT_CAPACITY] = "";

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }
    if (!ss_editor_resolve_analysis_start(editor, kind, start_token, start_id, sizeof(start_id), error)) {
        return 0;
    }
    if (!ss_structure_build_analysis_playback(structure, kind, start_id, &steps, &step_count, report, sizeof(report), error)) {
        return 0;
    }

    ss_editor_clear_analysis(editor);
    ss_str_copy(editor->analysis_report, sizeof(editor->analysis_report), report);
    editor->playback.kind = kind;
    ss_str_copy(editor->playback.start_node_id, sizeof(editor->playback.start_node_id), start_id);
    editor->playback.steps = steps;
    editor->playback.step_count = step_count;
    editor->playback.current_step = 0;
    editor->playback.prepared = step_count > 0;
    editor->playback_fx.autoplay_enabled = 0;
    editor->playback_fx.autoplay_elapsed_ms = 0;
    if (step_count == 0) {
        ss_editor_set_status(editor, "La simulacion de %s no genero pasos para reproducir.", ss_analysis_kind_label(kind));
        return 1;
    }

    ss_editor_trigger_playback_pulse(editor);
    ss_editor_set_playback_status(editor, "Recorrido guiado listo:");
    return 1;
}

void ss_editor_reset_analysis_playback(SsEditorState *editor)
{
    if (!ss_editor_analysis_playback_has_steps(editor)) {
        return;
    }

    editor->playback_fx.autoplay_enabled = 0;
    editor->playback_fx.autoplay_elapsed_ms = 0;
    editor->playback.current_step = 0;
    ss_editor_trigger_playback_pulse(editor);
    ss_editor_set_playback_status(editor, "Recorrido reiniciado:");
}

int ss_editor_analysis_playback_previous(SsEditorState *editor)
{
    if (!ss_editor_analysis_playback_has_steps(editor) || editor->playback.current_step == 0) {
        return 0;
    }

    editor->playback_fx.autoplay_enabled = 0;
    editor->playback_fx.autoplay_elapsed_ms = 0;
    --editor->playback.current_step;
    ss_editor_trigger_playback_pulse(editor);
    ss_editor_set_playback_status(editor, "Paso anterior:");
    return 1;
}

int ss_editor_analysis_playback_next(SsEditorState *editor)
{
    if (!ss_editor_analysis_playback_has_steps(editor) || editor->playback.current_step + 1 >= editor->playback.step_count) {
        return 0;
    }

    editor->playback_fx.autoplay_enabled = 0;
    editor->playback_fx.autoplay_elapsed_ms = 0;
    ++editor->playback.current_step;
    ss_editor_trigger_playback_pulse(editor);
    ss_editor_set_playback_status(editor, "Paso siguiente:");
    return 1;
}

int ss_editor_analysis_playback_has_steps(const SsEditorState *editor)
{
    return editor != NULL &&
        editor->playback.prepared &&
        editor->playback.steps != NULL &&
        editor->playback.step_count > 0 &&
        editor->playback.current_step < editor->playback.step_count;
}

const SsAnalysisStep *ss_editor_current_analysis_step(const SsEditorState *editor)
{
    if (!ss_editor_analysis_playback_has_steps(editor)) {
        return NULL;
    }

    return &editor->playback.steps[editor->playback.current_step];
}

int ss_editor_playback_node_state(const SsEditorState *editor, const char *node_id, int *visited, int *current)
{
    int found = 0;

    if (visited != NULL) {
        *visited = 0;
    }
    if (current != NULL) {
        *current = 0;
    }
    if (!ss_editor_analysis_playback_has_steps(editor) || node_id == NULL || node_id[0] == '\0') {
        return 0;
    }

    for (size_t index = 0; index <= editor->playback.current_step && index < editor->playback.step_count; ++index) {
        if (strcmp(editor->playback.steps[index].node_id, node_id) == 0) {
            found = 1;
            if (visited != NULL) {
                *visited = 1;
            }
            if (current != NULL && index == editor->playback.current_step) {
                *current = 1;
            }
        }
    }

    return found;
}

int ss_editor_playback_edge_state(const SsEditorState *editor, const char *edge_id, int *visited, int *current)
{
    int found = 0;

    if (visited != NULL) {
        *visited = 0;
    }
    if (current != NULL) {
        *current = 0;
    }
    if (!ss_editor_analysis_playback_has_steps(editor) || edge_id == NULL || edge_id[0] == '\0') {
        return 0;
    }

    for (size_t index = 0; index <= editor->playback.current_step && index < editor->playback.step_count; ++index) {
        if (strcmp(editor->playback.steps[index].edge_id, edge_id) == 0) {
            found = 1;
            if (visited != NULL) {
                *visited = 1;
            }
            if (current != NULL && index == editor->playback.current_step) {
                *current = 1;
            }
        }
    }

    return found;
}

void ss_editor_delete_selection(SsEditorState *editor, SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    SsLayoutSnapshot before_snapshot;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return;
    }

    if (editor->selection.type == SS_SELECTION_NODE) {
        ss_editor_capture_layout_before_change(structure, &before_snapshot);
        if (ss_structure_delete_node(structure, editor->selection.node_id, error)) {
            ss_editor_commit_structure_change(editor, structure, 1, &before_snapshot, 0);
            ss_editor_set_status(editor, "Nodo eliminado.");
        }
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return;
    }

    if (editor->selection.type == SS_SELECTION_EDGE) {
        ss_editor_capture_layout_before_change(structure, &before_snapshot);
        if (ss_structure_delete_edge(structure, editor->selection.edge_id, error)) {
            ss_editor_commit_structure_change(editor, structure, 1, &before_snapshot, 0);
            ss_editor_set_status(editor, "Arista eliminada.");
        }
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return;
    }

    ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay seleccion para eliminar.");
}

static int ss_editor_apply_common(int (*operation)(
        SsStructure *, const char *, const char *, int, const char *, SsError *, char *, size_t),
    SsEditorState *editor,
    const char *primary,
    const char *secondary,
    int numeric_value,
    SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    char message[SS_MESSAGE_CAPACITY] = "";
    SsLayoutSnapshot before_snapshot;

    /* This helper behaves like a small transaction wrapper around core
     * mutations so primary/secondary/tertiary actions stay consistent. */
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!operation(structure, primary, secondary, numeric_value, editor->selection.node_id, error, message, sizeof(message))) {
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }

    ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 0);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    ss_editor_set_status(editor, "%s", message[0] != '\0' ? message : "Operacion aplicada.");
    return 1;
}

int ss_editor_apply_primary(SsEditorState *editor, const char *primary, const char *secondary, int numeric_value, SsError *error)
{
    return ss_editor_apply_common(ss_structure_apply_primary, editor, primary, secondary, numeric_value, error);
}

int ss_editor_apply_secondary(SsEditorState *editor, const char *primary, const char *secondary, int numeric_value, SsError *error)
{
    return ss_editor_apply_common(ss_structure_apply_secondary, editor, primary, secondary, numeric_value, error);
}

int ss_editor_apply_tertiary(SsEditorState *editor, const char *primary, const char *secondary, int numeric_value, SsError *error)
{
    return ss_editor_apply_common(ss_structure_apply_tertiary, editor, primary, secondary, numeric_value, error);
}

int ss_editor_apply_property_update(SsEditorState *editor, const char *label, const char *value, const char *secondary, int numeric_value, SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    SsLayoutSnapshot before_snapshot;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    if (editor->selection.type == SS_SELECTION_NODE) {
        ss_editor_capture_layout_before_change(structure, &before_snapshot);
        if (!ss_structure_update_node(structure, editor->selection.node_id, label, value, secondary, numeric_value, error)) {
            ss_editor_layout_snapshot_dispose(&before_snapshot);
            return 0;
        }
        ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 0);
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        ss_editor_set_status(editor, "Nodo actualizado.");
        return 1;
    }

    if (editor->selection.type == SS_SELECTION_EDGE) {
        ss_editor_capture_layout_before_change(structure, &before_snapshot);
        if (!ss_structure_update_edge(structure, editor->selection.edge_id, label, (double) numeric_value, error)) {
            ss_editor_layout_snapshot_dispose(&before_snapshot);
            return 0;
        }
        ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 0);
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        ss_editor_set_status(editor, "Arista actualizada.");
        return 1;
    }

    ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo o una arista.");
    return 0;
}

int ss_editor_rotate_selection_left(SsEditorState *editor, SsError *error)
{
    SsStructure *structure;
    SsLayoutSnapshot before_snapshot;
    SsStructure before_structure;
    char pivot_id[SS_ID_CAPACITY];
    char promoted_id[SS_ID_CAPACITY];
    const SsNode *pivot;

    /* The left-rotation path is semantic for trees: references are rewired in
     * the core, then the editor only handles animation/history/selection. */
    if (editor == NULL || editor->selection.type != SS_SELECTION_NODE) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo para rotar.");
        return 0;
    }

    structure = ss_document_active_structure(&editor->document);
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    memset(&before_structure, 0, sizeof(before_structure));
    ss_str_copy(pivot_id, sizeof(pivot_id), editor->selection.node_id);
    pivot = ss_structure_find_node_const(structure, pivot_id);
    ss_str_copy(promoted_id, sizeof(promoted_id), pivot != NULL ? pivot->data.ref_b : "");
    if (!ss_structure_clone(&before_structure, structure, error)) {
        return 0;
    }
    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!ss_structure_rotate_left(structure, pivot_id, error)) {
        ss_structure_free(&before_structure);
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }

    ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 1);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    if (promoted_id[0] != '\0') {
        ss_editor_select_node(editor, promoted_id);
        ss_editor_set_status(
            editor,
            "Rotacion izquierda: %s sube y %s baja a su rama izquierda.",
            promoted_id,
            pivot_id);
    } else {
        ss_editor_set_status(editor, "Rotacion izquierda aplicada sobre %s.", pivot_id);
    }
    if (!ss_editor_record_rotation_change(
            editor,
            editor->document.active_structure_index,
            &before_structure,
            structure,
            pivot_id,
            promoted_id[0] != '\0' ? promoted_id : pivot_id,
            error)) {
        ss_structure_free(&before_structure);
        return 0;
    }
    ss_structure_free(&before_structure);
    return 1;
}

int ss_editor_rotate_selection_right(SsEditorState *editor, SsError *error)
{
    SsStructure *structure;
    SsLayoutSnapshot before_snapshot;
    SsStructure before_structure;
    char pivot_id[SS_ID_CAPACITY];
    char promoted_id[SS_ID_CAPACITY];
    const SsNode *pivot;

    /* The right rotation mirrors the left rotation and exists separately
     * because the promoted child and status message differ. */
    if (editor == NULL || editor->selection.type != SS_SELECTION_NODE) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo para rotar.");
        return 0;
    }

    structure = ss_document_active_structure(&editor->document);
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    memset(&before_structure, 0, sizeof(before_structure));
    ss_str_copy(pivot_id, sizeof(pivot_id), editor->selection.node_id);
    pivot = ss_structure_find_node_const(structure, pivot_id);
    ss_str_copy(promoted_id, sizeof(promoted_id), pivot != NULL ? pivot->data.ref_a : "");
    if (!ss_structure_clone(&before_structure, structure, error)) {
        return 0;
    }
    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!ss_structure_rotate_right(structure, pivot_id, error)) {
        ss_structure_free(&before_structure);
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }

    ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 1);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    if (promoted_id[0] != '\0') {
        ss_editor_select_node(editor, promoted_id);
        ss_editor_set_status(
            editor,
            "Rotacion derecha: %s sube y %s baja a su rama derecha.",
            promoted_id,
            pivot_id);
    } else {
        ss_editor_set_status(editor, "Rotacion derecha aplicada sobre %s.", pivot_id);
    }
    if (!ss_editor_record_rotation_change(
            editor,
            editor->document.active_structure_index,
            &before_structure,
            structure,
            pivot_id,
            promoted_id[0] != '\0' ? promoted_id : pivot_id,
            error)) {
        ss_structure_free(&before_structure);
        return 0;
    }
    ss_structure_free(&before_structure);
    return 1;
}

static int ss_editor_rotate_graph_common(SsEditorState *editor, double radians, const char *direction_label, SsError *error)
{
    SsStructure *structure;
    SsLayoutSnapshot before_snapshot;
    SsStructure before_structure;
    const char *pivot_id = NULL;

    /* Graph rotation changes only coordinates, not topology. Even so, it goes
     * through the same history/animation infrastructure as tree rotations. */
    if (editor == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay editor activo.");
        return 0;
    }

    structure = ss_document_active_structure(&editor->document);
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }
    if (editor->selection.type == SS_SELECTION_NODE) {
        pivot_id = editor->selection.node_id;
    }

    memset(&before_structure, 0, sizeof(before_structure));
    if (!ss_structure_clone(&before_structure, structure, error)) {
        return 0;
    }
    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!ss_structure_rotate_graph_layout(structure, radians, pivot_id, error)) {
        ss_structure_free(&before_structure);
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }

    ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 1);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    if (pivot_id != NULL && pivot_id[0] != '\0') {
        ss_editor_set_status(
            editor,
            "Grafo girado %s usando %s como pivote visual.",
            direction_label != NULL ? direction_label : "a la izquierda",
            pivot_id);
    } else {
        ss_editor_set_status(
            editor,
            "Grafo girado %s alrededor del centro visual.",
            direction_label != NULL ? direction_label : "a la izquierda");
    }
    if (!ss_editor_record_rotation_change(
            editor,
            editor->document.active_structure_index,
            &before_structure,
            structure,
            pivot_id != NULL ? pivot_id : "",
            pivot_id != NULL ? pivot_id : "",
            error)) {
        ss_structure_free(&before_structure);
        return 0;
    }
    ss_structure_free(&before_structure);
    return 1;
}

int ss_editor_rotate_graph_left(SsEditorState *editor, SsError *error)
{
    return ss_editor_rotate_graph_common(editor, -SS_GRAPH_ROTATION_STEP_RADIANS, "a la izquierda", error);
}

int ss_editor_rotate_graph_right(SsEditorState *editor, SsError *error)
{
    return ss_editor_rotate_graph_common(editor, SS_GRAPH_ROTATION_STEP_RADIANS, "a la derecha", error);
}

int ss_editor_can_undo_rotation(const SsEditorState *editor)
{
    return editor != NULL && editor->rotation_history.undo_count > 0;
}

int ss_editor_can_redo_rotation(const SsEditorState *editor)
{
    return editor != NULL && editor->rotation_history.redo_count > 0;
}

int ss_editor_undo_rotation(SsEditorState *editor, SsError *error)
{
    SsRotationHistoryEntry entry;

    if (editor == NULL || editor->rotation_history.undo_count == 0) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay rotacion para deshacer.");
        return 0;
    }

    memset(&entry, 0, sizeof(entry));
    if (!ss_rotation_history_pop(
            editor->rotation_history.undo_entries,
            &editor->rotation_history.undo_count,
            &entry)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay rotacion para deshacer.");
        return 0;
    }

    if (!ss_editor_restore_rotation_structure(editor, &entry, 0, error)) {
        ss_rotation_history_push(
            editor->rotation_history.undo_entries,
            &editor->rotation_history.undo_count,
            &entry);
        return 0;
    }

    ss_rotation_history_push(
        editor->rotation_history.redo_entries,
        &editor->rotation_history.redo_count,
        &entry);
    ss_editor_set_status(editor, "Rotacion deshecha.");
    ss_error_clear(error);
    return 1;
}

int ss_editor_redo_rotation(SsEditorState *editor, SsError *error)
{
    SsRotationHistoryEntry entry;

    if (editor == NULL || editor->rotation_history.redo_count == 0) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay rotacion para rehacer.");
        return 0;
    }

    memset(&entry, 0, sizeof(entry));
    if (!ss_rotation_history_pop(
            editor->rotation_history.redo_entries,
            &editor->rotation_history.redo_count,
            &entry)) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay rotacion para rehacer.");
        return 0;
    }

    if (!ss_editor_restore_rotation_structure(editor, &entry, 1, error)) {
        ss_rotation_history_push(
            editor->rotation_history.redo_entries,
            &editor->rotation_history.redo_count,
            &entry);
        return 0;
    }

    ss_rotation_history_push(
        editor->rotation_history.undo_entries,
        &editor->rotation_history.undo_count,
        &entry);
    ss_editor_set_status(editor, "Rotacion rehecha.");
    ss_error_clear(error);
    return 1;
}

void ss_editor_begin_pan(SsEditorState *editor, double mouse_x, double mouse_y)
{
    if (editor == NULL) {
        return;
    }

    editor->pan.is_panning = 1;
    editor->pan.moved = 0;
    editor->pan.anchor_x = mouse_x;
    editor->pan.anchor_y = mouse_y;
    editor->pan.origin_offset_x = editor->document.view_state.canvas_offset_x;
    editor->pan.origin_offset_y = editor->document.view_state.canvas_offset_y;
}

void ss_editor_pan_to(SsEditorState *editor, double mouse_x, double mouse_y)
{
    double next_x;
    double next_y;

    /* Panning is in the mouse-move hot path, so it only updates camera
     * offsets and avoids any heavy document synchronization. */
    if (editor == NULL || !editor->pan.is_panning) {
        return;
    }

    next_x = editor->pan.origin_offset_x + (mouse_x - editor->pan.anchor_x);
    next_y = editor->pan.origin_offset_y + (mouse_y - editor->pan.anchor_y);
    if (next_x != editor->document.view_state.canvas_offset_x ||
        next_y != editor->document.view_state.canvas_offset_y) {
        editor->document.view_state.canvas_offset_x = next_x;
        editor->document.view_state.canvas_offset_y = next_y;
        editor->pan.moved = 1;
    }
}

void ss_editor_end_pan(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    if (editor->pan.is_panning && editor->pan.moved) {
        ss_document_touch(&editor->document);
        ss_editor_set_status(editor, "Vista desplazada.");
    }
    memset(&editor->pan, 0, sizeof(editor->pan));
}

void ss_editor_reset_canvas_view(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    editor->document.view_state.canvas_offset_x = 0.0;
    editor->document.view_state.canvas_offset_y = 0.0;
    memset(&editor->pan, 0, sizeof(editor->pan));
    ss_document_touch(&editor->document);
    ss_editor_set_status(editor, "Vista del lienzo centrada.");
}

void ss_editor_canvas_to_world(const SsEditorState *editor, double canvas_x, double canvas_y, double *world_x, double *world_y)
{
    double offset_x = editor != NULL ? editor->document.view_state.canvas_offset_x : 0.0;
    double offset_y = editor != NULL ? editor->document.view_state.canvas_offset_y : 0.0;

    if (world_x != NULL) {
        *world_x = canvas_x - offset_x;
    }
    if (world_y != NULL) {
        *world_y = canvas_y - offset_y;
    }
}

void ss_editor_world_to_canvas(const SsEditorState *editor, double world_x, double world_y, double *canvas_x, double *canvas_y)
{
    double offset_x = editor != NULL ? editor->document.view_state.canvas_offset_x : 0.0;
    double offset_y = editor != NULL ? editor->document.view_state.canvas_offset_y : 0.0;

    if (canvas_x != NULL) {
        *canvas_x = world_x + offset_x;
    }
    if (canvas_y != NULL) {
        *canvas_y = world_y + offset_y;
    }
}

void ss_editor_begin_drag(SsEditorState *editor, const char *node_id, double mouse_x, double mouse_y)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    SsNode *node = structure != NULL ? ss_structure_find_node(structure, node_id) : NULL;

    if (node == NULL) {
        return;
    }

    editor->drag.is_dragging = 1;
    editor->drag.moved = 0;
    ss_str_copy(editor->drag.node_id, sizeof(editor->drag.node_id), node_id);
    editor->drag.offset_x = mouse_x - node->visual.x;
    editor->drag.offset_y = mouse_y - node->visual.y;
}

void ss_editor_drag_to(SsEditorState *editor, double mouse_x, double mouse_y)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    SsNode *node;

    /* Dragging modifies only node visuals in real time. The expensive work
     * such as touching the document is deferred until the gesture ends. */
    if (!editor->drag.is_dragging || structure == NULL) {
        return;
    }

    node = ss_structure_find_node(structure, editor->drag.node_id);
    if (node == NULL) {
        return;
    }

    {
        double next_x = ss_clamp_double(mouse_x - editor->drag.offset_x, 20.0, 3200.0);
        double next_y = ss_clamp_double(mouse_y - editor->drag.offset_y, 20.0, 2400.0);

        if (next_x != node->visual.x || next_y != node->visual.y) {
            node->visual.x = next_x;
            node->visual.y = next_y;
            structure->dirty_layout = 0;
            editor->drag.moved = 1;
        }
    }
}

void ss_editor_end_drag(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    if (editor->drag.is_dragging && editor->drag.moved) {
        ss_document_touch(&editor->document);
    }
    memset(&editor->drag, 0, sizeof(editor->drag));
}

void ss_editor_update_connection_preview(SsEditorState *editor, double mouse_x, double mouse_y)
{
    if (editor == NULL || !editor->connection.is_connecting) {
        return;
    }

    editor->connection.preview_x = mouse_x;
    editor->connection.preview_y = mouse_y;
    editor->connection.has_preview = 1;
}

void ss_editor_cancel_connection(SsEditorState *editor)
{
    if (editor == NULL) {
        return;
    }

    memset(&editor->connection, 0, sizeof(editor->connection));
}

int ss_editor_connect(SsEditorState *editor, const char *target_node_id, const char *relation_kind, double weight, SsError *error)
{
    SsStructure *structure = ss_document_active_structure(&editor->document);
    const char *source_id;
    SsLayoutSnapshot before_snapshot;

    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa.");
        return 0;
    }

    source_id = editor->connection.is_connecting ? editor->connection.source_node_id : editor->selection.node_id;
    if (source_id[0] == '\0') {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "Seleccione un nodo origen.");
        return 0;
    }

    if (!editor->connection.is_connecting) {
        editor->connection.is_connecting = 1;
        ss_str_copy(editor->connection.source_node_id, sizeof(editor->connection.source_node_id), source_id);
        editor->connection.preview_x = 0.0;
        editor->connection.preview_y = 0.0;
        editor->connection.has_preview = 0;
        ss_editor_set_status(editor, "Origen de conexion fijado: %s", source_id);
        return 1;
    }

    ss_editor_capture_layout_before_change(structure, &before_snapshot);
    if (!ss_structure_connect(structure, editor->connection.source_node_id, target_node_id, relation_kind, weight, error)) {
        ss_editor_layout_snapshot_dispose(&before_snapshot);
        return 0;
    }
    ss_editor_commit_structure_change(editor, structure, 0, &before_snapshot, 0);
    ss_editor_layout_snapshot_dispose(&before_snapshot);
    ss_editor_set_status(editor, "Conexion creada entre %s y %s", editor->connection.source_node_id, target_node_id);
    memset(&editor->connection, 0, sizeof(editor->connection));
    return 1;
}
