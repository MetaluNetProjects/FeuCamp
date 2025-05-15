// Pongar config

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/time.h>
typedef struct timeval absolute_time_t;

#define MAX(x,m) ((m) > (x) ? (m) : (x))
#define MIN(x,m) ((m) < (x) ? (m) : (x))

/* -------------------------- pico sdk compat --------------------------*/
inline absolute_time_t make_timeout_time_ms(int ms) {
    struct timeval tv;
    struct timeval tv_offset {
        .tv_sec = ms / 1000, .tv_usec = (ms % 1000) * 1000
    };
    gettimeofday(&tv, 0);
    timeradd(&tv, &tv_offset, &tv);
    return tv;
}

inline bool time_reached(absolute_time_t t) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return timercmp(&t, &tv, < );
}

inline absolute_time_t at_the_end_of_time { .tv_sec = (long)1e12, .tv_usec = 0};

inline absolute_time_t get_absolute_time() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}

inline absolute_time_t boot_time = get_absolute_time();

inline uint32_t to_ms_since_boot (absolute_time_t t) {
    struct timeval tv;
    timersub(&t, &boot_time, &tv);
    return tv.tv_sec * 1000U + tv.tv_usec / 1000U;
}

/* -------------------------- Fraise compat --------------------------*/
inline uint8_t fraise_get_uint8() {
    return 0;
}

