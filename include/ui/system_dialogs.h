/*
 * StructStudio C
 * --------------
 * Thin wrapper over native message dialogs.
 *
 * The goal is to centralize dialog styling/behavior and allow Windows builds
 * to use stock operating-system icons without leaking Win32 details into the
 * rest of the UI layer.
 */

#ifndef SS_UI_SYSTEM_DIALOGS_H
#define SS_UI_SYSTEM_DIALOGS_H

#include <ui.h>

typedef enum SsDialogIcon {
    SS_DIALOG_ICON_INFO = 0,
    SS_DIALOG_ICON_WARNING,
    SS_DIALOG_ICON_ERROR,
    SS_DIALOG_ICON_QUESTION
} SsDialogIcon;

void ss_dialog_show(uiWindow *parent, const char *title, const char *message, SsDialogIcon icon);
void ss_dialog_show_info(uiWindow *parent, const char *title, const char *message);
void ss_dialog_show_warning(uiWindow *parent, const char *title, const char *message);
void ss_dialog_show_error(uiWindow *parent, const char *title, const char *message);

#endif
