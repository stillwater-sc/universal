#pragma once
//  performance_runner.h : pure C functions to aid in performance testing and reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdio.h>
#include <stddef.h>

// convert a floating point value to a power-of-ten string
char* toPowerOfTen(double value) {
    const char* scales[] = { " ", "K", "M", "G", "T", "P", "E", "Z" };
    double lower_bound = 1.0;
    double scale_factor = 1.0;
    int integer_value = 0;
    size_t scale = 0;
    for (size_t i = 0; i < sizeof(scales); ++i) {
        if (value > lower_bound && value < 1000 * lower_bound) {
            integer_value = (int)(value / scale_factor);
            scale = i;
            break;
        }
        lower_bound *= 1000;
        scale_factor *= 1000.0;
    }
//    std::stringstream ss;
//    ss << std::setw(3) << std::right << integer_value << ' ' << scales[scale];
    static char result[20]; // static to ensure it persists after the function returns
    sprintf(result, "%3d %s", integer_value, scales[scale]);
    return result;
}

#ifdef _WIN32
#include <windows.h>

void PerformanceRunner(const char* tag, void (*f)(size_t), size_t nr_ops) {
    LARGE_INTEGER frequency, begin, end;
    double elapsed_time;
    double ops_per_sec;

    /* Get the frequency of the performance counter */
    QueryPerformanceFrequency(&frequency);

    /* Get start time */
    QueryPerformanceCounter(&begin);

    /* Execute the function */
    f(nr_ops);

    /* Get end time */
    QueryPerformanceCounter(&end);

    /* Calculate elapsed time in seconds */
    elapsed_time = (double)(end.QuadPart - begin.QuadPart) / frequency.QuadPart;

    /* Calculate operations per second */
    ops_per_sec = (double)nr_ops / elapsed_time;

    /* Print results - assuming to_power_of_ten returns a string */
    printf("%s %10zu per %15.9f sec -> %s ops/sec\n",
        tag, nr_ops, elapsed_time, toPowerOfTen(ops_per_sec));
}

#else
#include <time.h>

void PerformanceRunner(const char* tag, void (*f)(size_t), size_t nr_ops) {
    struct timespec begin, end;
    double elapsed_time;
    double ops_per_sec;

    /* Get start time */
    clock_gettime(CLOCK_MONOTONIC, &begin);

    /* Execute the function */
    f(nr_ops);

    /* Get end time */
    clock_gettime(CLOCK_MONOTONIC, &end);

    /* Calculate elapsed time in seconds */
    elapsed_time = (end.tv_sec - begin.tv_sec) +
        (end.tv_nsec - begin.tv_nsec) / 1000000000.0;

    /* Calculate operations per second */
    ops_per_sec = (double)nr_ops / elapsed_time;

    /* Print results */
    printf("%s %10zu per %15.9f sec -> %.2e ops/sec\n",
        tag, nr_ops, elapsed_time, ops_per_sec);
}
#endif

/* Portable fallback using clock() - add this if you need it */
#ifdef NEED_CLOCK_FALLBACK
#include <time.h>

void performance_runner_portable(const char* tag, void (*f)(size_t), size_t nr_ops) {
    clock_t begin, end;
    double elapsed_time;
    double ops_per_sec;

    begin = clock();
    f(nr_ops);
    end = clock();

    elapsed_time = ((double)(end - begin)) / CLOCKS_PER_SEC;
    ops_per_sec = (double)nr_ops / elapsed_time;

    printf("%s %10zu per %15.9f sec -> %.2e ops/sec\n",
        tag, nr_ops, elapsed_time, ops_per_sec);
}
#endif
