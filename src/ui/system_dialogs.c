/*
 * StructStudio C
 * --------------
 * Native system dialog wrappers.
 *
 * On Windows we call MessageBoxW directly so dialogs can use project-owned
 * icons without leaking Win32 details into the rest of the UI layer. Other
 * platforms keep a libui-ng fallback to preserve portability.
 */

#include "ui/system_dialogs.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui/windows_resources.h"
#endif

static const char *ss_dialog_safe_text(const char *text)
{
    return text != NULL ? text : "";
}

#ifdef _WIN32
static wchar_t *ss_dialog_to_wide(const char *text)
{
    int size;
    wchar_t *wide;
    const char *safe = ss_dialog_safe_text(text);

    size = MultiByteToWideChar(CP_UTF8, 0, safe, -1, NULL, 0);
    if (size <= 0) {
        size = MultiByteToWideChar(CP_ACP, 0, safe, -1, NULL, 0);
        if (size <= 0) {
            return NULL;
        }
        wide = (wchar_t *) calloc((size_t) size, sizeof(*wide));
        if (wide == NULL) {
            return NULL;
        }
        if (MultiByteToWideChar(CP_ACP, 0, safe, -1, wide, size) <= 0) {
            free(wide);
            return NULL;
        }
        return wide;
    }

    wide = (wchar_t *) calloc((size_t) size, sizeof(*wide));
    if (wide == NULL) {
        return NULL;
    }
    if (MultiByteToWideChar(CP_UTF8, 0, safe, -1, wide, size) <= 0) {
        free(wide);
        return NULL;
    }
    return wide;
}

static UINT ss_dialog_icon_flags(SsDialogIcon icon)
{
    switch (icon) {
        case SS_DIALOG_ICON_WARNING:
            return MB_ICONWARNING;
        case SS_DIALOG_ICON_ERROR:
            return MB_ICONERROR;
        case SS_DIALOG_ICON_QUESTION:
            return MB_ICONQUESTION;
        case SS_DIALOG_ICON_INFO:
        default:
            return MB_ICONINFORMATION;
    }
}

static LPCWSTR ss_dialog_resource_icon(SsDialogIcon icon)
{
    switch (icon) {
        case SS_DIALOG_ICON_WARNING:
            return MAKEINTRESOURCEW(IDI_DIALOG_WARNING);
        case SS_DIALOG_ICON_ERROR:
            return MAKEINTRESOURCEW(IDI_DIALOG_ERROR);
        case SS_DIALOG_ICON_QUESTION:
            return MAKEINTRESOURCEW(IDI_DIALOG_QUESTION);
        case SS_DIALOG_ICON_INFO:
        default:
            return MAKEINTRESOURCEW(IDI_DIALOG_INFO);
    }
}
#endif

void ss_dialog_show(uiWindow *parent, const char *title, const char *message, SsDialogIcon icon)
{
#ifdef _WIN32
    HWND hwnd = parent != NULL ? (HWND) uiControlHandle(uiControl(parent)) : NULL;
    wchar_t *title_wide = ss_dialog_to_wide(title);
    wchar_t *message_wide = ss_dialog_to_wide(message);
    MSGBOXPARAMSW params;

    if (title_wide != NULL && message_wide != NULL) {
        ZeroMemory(&params, sizeof(params));
        params.cbSize = sizeof(params);
        params.hwndOwner = hwnd;
        params.hInstance = GetModuleHandleW(NULL);
        params.lpszText = message_wide;
        params.lpszCaption = title_wide;
        params.dwStyle = MB_OK | MB_SETFOREGROUND | MB_TASKMODAL | MB_USERICON;
        params.lpszIcon = ss_dialog_resource_icon(icon);
        if (MessageBoxIndirectW(&params) != 0) {
            free(title_wide);
            free(message_wide);
            return;
        }

        MessageBoxW(
            hwnd,
            message_wide,
            title_wide,
            MB_OK | MB_SETFOREGROUND | MB_TASKMODAL | ss_dialog_icon_flags(icon));
        free(title_wide);
        free(message_wide);
        return;
    }

    free(title_wide);
    free(message_wide);
#endif

    if (icon == SS_DIALOG_ICON_ERROR) {
        uiMsgBoxError(parent, ss_dialog_safe_text(title), ss_dialog_safe_text(message));
    } else {
        uiMsgBox(parent, ss_dialog_safe_text(title), ss_dialog_safe_text(message));
    }
}

void ss_dialog_show_info(uiWindow *parent, const char *title, const char *message)
{
    ss_dialog_show(parent, title, message, SS_DIALOG_ICON_INFO);
}

void ss_dialog_show_warning(uiWindow *parent, const char *title, const char *message)
{
    ss_dialog_show(parent, title, message, SS_DIALOG_ICON_WARNING);
}

void ss_dialog_show_error(uiWindow *parent, const char *title, const char *message)
{
    ss_dialog_show(parent, title, message, SS_DIALOG_ICON_ERROR);
}
