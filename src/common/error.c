#include "common/error.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void ss_error_clear(SsError *error)
{
    if (error == NULL) {
        return;
    }

    error->code = SS_ERROR_NONE;
    error->message[0] = '\0';
}

void ss_error_set(SsError *error, SsErrorCode code, const char *message)
{
    if (error == NULL) {
        return;
    }

    error->code = code;
    if (message == NULL) {
        error->message[0] = '\0';
        return;
    }

    strncpy(error->message, message, sizeof(error->message) - 1);
    error->message[sizeof(error->message) - 1] = '\0';
}

void ss_error_format(SsError *error, SsErrorCode code, const char *fmt, ...)
{
    va_list args;

    if (error == NULL) {
        return;
    }

    error->code = code;
    if (fmt == NULL) {
        error->message[0] = '\0';
        return;
    }

    va_start(args, fmt);
    vsnprintf(error->message, sizeof(error->message), fmt, args);
    va_end(args);
}

int ss_error_is_ok(const SsError *error)
{
    return error == NULL || error->code == SS_ERROR_NONE;
}
