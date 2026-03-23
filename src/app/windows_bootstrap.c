/*
 * StructStudio C
 * --------------
 * Windows bootstrap for process-wide DPI behavior.
 *
 * We keep this isolated from the UI layer because DPI awareness is a process
 * startup concern, not a widget concern. The function uses dynamic lookup so
 * the executable can still start on older Windows releases while preferring
 * Per-Monitor V2 on Windows 10/11.
 */

#include "app/windows_bootstrap.h"

#if defined(_WIN32)

#include <string.h>
#include <windows.h>

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE) -4)
#endif

typedef BOOL (WINAPI *SsSetProcessDpiAwarenessContextFn)(HANDLE value);
typedef HRESULT (WINAPI *SsSetProcessDpiAwarenessFn)(int value);
typedef BOOL (WINAPI *SsSetProcessDPIAwareFn)(void);

/*
 * GetProcAddress() returns FARPROC, which GCC warns about when it is cast
 * directly to a function pointer type under -Wcast-function-type. We keep the
 * dynamic lookup but copy the raw pointer bytes into the typed callback to
 * avoid the warning while preserving the same runtime behavior.
 */
static SsSetProcessDpiAwarenessContextFn ss_lookup_set_process_dpi_awareness_context(HMODULE module_handle)
{
    FARPROC raw_symbol;
    SsSetProcessDpiAwarenessContextFn typed_symbol;

    raw_symbol = GetProcAddress(module_handle, "SetProcessDpiAwarenessContext");
    typed_symbol = NULL;
    if (raw_symbol != NULL) {
        memcpy(&typed_symbol, &raw_symbol, sizeof(typed_symbol));
    }
    return typed_symbol;
}

static SsSetProcessDpiAwarenessFn ss_lookup_set_process_dpi_awareness(HMODULE module_handle)
{
    FARPROC raw_symbol;
    SsSetProcessDpiAwarenessFn typed_symbol;

    raw_symbol = GetProcAddress(module_handle, "SetProcessDpiAwareness");
    typed_symbol = NULL;
    if (raw_symbol != NULL) {
        memcpy(&typed_symbol, &raw_symbol, sizeof(typed_symbol));
    }
    return typed_symbol;
}

static SsSetProcessDPIAwareFn ss_lookup_set_process_dpi_aware(HMODULE module_handle)
{
    FARPROC raw_symbol;
    SsSetProcessDPIAwareFn typed_symbol;

    raw_symbol = GetProcAddress(module_handle, "SetProcessDPIAware");
    typed_symbol = NULL;
    if (raw_symbol != NULL) {
        memcpy(&typed_symbol, &raw_symbol, sizeof(typed_symbol));
    }
    return typed_symbol;
}

static int ss_try_enable_per_monitor_v2(void)
{
    HMODULE user32_module;
    SsSetProcessDpiAwarenessContextFn set_context;

    user32_module = GetModuleHandleW(L"user32.dll");
    if (user32_module == NULL) {
        return 0;
    }

    set_context = ss_lookup_set_process_dpi_awareness_context(user32_module);
    if (set_context != NULL && set_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        return 1;
    }
    return 0;
}

static int ss_try_enable_per_monitor_v1(void)
{
    HMODULE shcore_module;
    SsSetProcessDpiAwarenessFn set_awareness;
    const int process_per_monitor_dpi_aware = 2;

    shcore_module = LoadLibraryW(L"shcore.dll");
    if (shcore_module == NULL) {
        return 0;
    }

    set_awareness = ss_lookup_set_process_dpi_awareness(shcore_module);
    if (set_awareness != NULL) {
        HRESULT result = set_awareness(process_per_monitor_dpi_aware);
        FreeLibrary(shcore_module);
        return result >= 0;
    }

    FreeLibrary(shcore_module);
    return 0;
}

static int ss_try_enable_system_dpi_aware(void)
{
    HMODULE user32_module;
    SsSetProcessDPIAwareFn set_system_aware;

    user32_module = GetModuleHandleW(L"user32.dll");
    if (user32_module == NULL) {
        return 0;
    }

    set_system_aware = ss_lookup_set_process_dpi_aware(user32_module);
    if (set_system_aware != NULL) {
        return set_system_aware() ? 1 : 0;
    }
    return 0;
}

void ss_windows_bootstrap(void)
{
    if (ss_try_enable_per_monitor_v2()) {
        return;
    }
    if (ss_try_enable_per_monitor_v1()) {
        return;
    }
    ss_try_enable_system_dpi_aware();
}

#else

void ss_windows_bootstrap(void)
{
}

#endif
