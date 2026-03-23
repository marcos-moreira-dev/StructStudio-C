/*
 * StructStudio C
 * --------------
 * Shared utility declarations and size conventions.
 *
 * Path capacity is intentionally larger than a legacy Windows MAX_PATH-sized
 * buffer so the same document metadata can travel more safely between Windows
 * and Linux builds.
 */

#ifndef SS_COMMON_UTIL_H
#define SS_COMMON_UTIL_H

#include <stddef.h>
#include <stdint.h>

#define SS_ARRAY_GROWTH_STEP 8
#define SS_ID_CAPACITY 64
#define SS_NAME_CAPACITY 96
#define SS_LABEL_CAPACITY 64
#define SS_VALUE_CAPACITY 128
#define SS_DESCRIPTION_CAPACITY 256
#define SS_PATH_CAPACITY 1024
#define SS_RELATION_CAPACITY 32
#define SS_LAYOUT_CAPACITY 32
#define SS_KIND_CAPACITY 32
#define SS_VALUE_TYPE_CAPACITY 16
#define SS_VALIDATION_CAPACITY 32
#define SS_ANALYSIS_REPORT_CAPACITY 4096
#define SS_THEORY_TEXT_CAPACITY 12288

void ss_str_copy(char *dest, size_t capacity, const char *src);
void ss_timestamp_now(char *dest, size_t capacity);
uint64_t ss_monotonic_millis(void);
void ss_generate_id(const char *prefix, char *dest, size_t capacity);
int ss_parse_int(const char *text, int *value);
double ss_clamp_double(double value, double min_value, double max_value);
size_t ss_next_capacity(size_t current, size_t required);
int ss_array_reserve(void **items, size_t element_size, size_t *capacity, size_t required);
uint32_t ss_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

#endif
