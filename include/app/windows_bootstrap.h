/*
 * StructStudio C
 * --------------
 * Windows-only process bootstrap.
 *
 * This header isolates startup concerns such as DPI awareness from the rest of
 * the application so UI modules can stay toolkit-focused.
 */

#ifndef SS_APP_WINDOWS_BOOTSTRAP_H
#define SS_APP_WINDOWS_BOOTSTRAP_H

void ss_windows_bootstrap(void);

#endif
