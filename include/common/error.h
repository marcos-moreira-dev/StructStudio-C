#ifndef SS_COMMON_ERROR_H
#define SS_COMMON_ERROR_H

#include <stddef.h>

#define SS_MESSAGE_CAPACITY 256

typedef enum SsErrorCode {
    SS_ERROR_NONE = 0,
    SS_ERROR_ARGUMENT,
    SS_ERROR_NOT_FOUND,
    SS_ERROR_DUPLICATE,
    SS_ERROR_INVALID_STATE,
    SS_ERROR_VALIDATION,
    SS_ERROR_IO,
    SS_ERROR_PARSE,
    SS_ERROR_MEMORY
} SsErrorCode;

typedef struct SsError {
    SsErrorCode code;
    char message[SS_MESSAGE_CAPACITY];
} SsError;

void ss_error_clear(SsError *error);
void ss_error_set(SsError *error, SsErrorCode code, const char *message);
void ss_error_format(SsError *error, SsErrorCode code, const char *fmt, ...);
int ss_error_is_ok(const SsError *error);

#endif
