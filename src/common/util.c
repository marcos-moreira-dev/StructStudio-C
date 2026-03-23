#include "common/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

void ss_str_copy(char *dest, size_t capacity, const char *src)
{
    if (dest == NULL || capacity == 0) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    strncpy(dest, src, capacity - 1);
    dest[capacity - 1] = '\0';
}

void ss_timestamp_now(char *dest, size_t capacity)
{
    time_t now;
    struct tm *tm_info;

    if (dest == NULL || capacity == 0) {
        return;
    }

    now = time(NULL);
    tm_info = localtime(&now);
    if (tm_info == NULL) {
        dest[0] = '\0';
        return;
    }

    strftime(dest, capacity, "%Y-%m-%dT%H:%M:%S", tm_info);
}

uint64_t ss_monotonic_millis(void)
{
#ifdef _WIN32
    return (uint64_t) GetTickCount64();
#elif defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t) ts.tv_sec * 1000ULL + (uint64_t) ts.tv_nsec / 1000000ULL;
    }
    return 0;
#else
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == TIME_UTC) {
        return (uint64_t) ts.tv_sec * 1000ULL + (uint64_t) ts.tv_nsec / 1000000ULL;
    }
    return 0;
#endif
}

void ss_generate_id(const char *prefix, char *dest, size_t capacity)
{
    static unsigned long long counter = 1ULL;
    const char *effective_prefix = prefix != NULL ? prefix : "item";

    if (dest == NULL || capacity == 0) {
        return;
    }

    snprintf(dest, capacity, "%s_%llu", effective_prefix, counter++);
}

int ss_parse_int(const char *text, int *value)
{
    char *endptr = NULL;
    long parsed;

    if (text == NULL || text[0] == '\0' || value == NULL) {
        return 0;
    }

    parsed = strtol(text, &endptr, 10);
    if (endptr == text || *endptr != '\0') {
        return 0;
    }

    *value = (int) parsed;
    return 1;
}

double ss_clamp_double(double value, double min_value, double max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

size_t ss_next_capacity(size_t current, size_t required)
{
    size_t next = current == 0 ? SS_ARRAY_GROWTH_STEP : current;

    while (next < required) {
        next += SS_ARRAY_GROWTH_STEP;
    }
    return next;
}

int ss_array_reserve(void **items, size_t element_size, size_t *capacity, size_t required)
{
    void *grown;
    size_t next_capacity;

    if (items == NULL || capacity == NULL || element_size == 0) {
        return 0;
    }

    if (*capacity >= required) {
        return 1;
    }

    next_capacity = ss_next_capacity(*capacity, required);
    grown = realloc(*items, element_size * next_capacity);
    if (grown == NULL) {
        return 0;
    }

    *items = grown;
    *capacity = next_capacity;
    return 1;
}

uint32_t ss_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return ((uint32_t) r << 24) | ((uint32_t) g << 16) | ((uint32_t) b << 8) | ((uint32_t) a);
}
