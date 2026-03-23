/*
 * StructStudio C
 * --------------
 * Native UI entry point.
 *
 * The rest of the application does not know how libui-ng is bootstrapped; it
 * only asks the main window to create the desktop interface and run it.
 */

#ifndef SS_UI_MAIN_WINDOW_H
#define SS_UI_MAIN_WINDOW_H

int ss_main_window_run(void);

#endif
