// exact_test.cpp: functional tests for C API
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit_c_api.h>
#if !(defined(_MSC_VER))
#include <sys/time.h>
#endif

#include <random>

static posit8_t posit8_abs(posit8_t x) {
    if (x.v & 0x80) { return posit8_sub(ZERO8, x); }
    return x;
}

static posit8x2_t posit8x2(posit8_t x, posit8_t y) {
    posit8x2_t out;
    out.x = x;
    out.y = y;
    return out;
}

static posit8x2_t posit8_add_exact_bruteforce(posit8_t a, posit8_t b, bool loud) {
    // searching for [x,y] where x+y = a+b and y is the smallest
    // obvious first step is swap a,b so that b is always <= a
    if (posit8_cmp(posit8_abs(a), posit8_abs(b)) < 0) {
        //printf("abs a < b  %f %f\n", posit8_tod(a), posit8_tod(b));
        return posit8_add_exact_bruteforce(b, a, loud);
    }
    double realsum = posit8_tod(a) + posit8_tod(b);
    // nan special case
    if (std::isnan(realsum)) {  // explicit call to std, to work-around a bug in gcc 5 and 6
        // return a NAR and a ZERO so that someone running add_exact
        // as a mutative sorting comparitor and discarding zeros
        // will end with 1 NAR only.
        return posit8x2(NAR8, ZERO8);
    }
    // search for a smaller value of y which satisfies x+y == a+b
    // start scanning at 0 and work up switching between positive and negative
    // to find the value of smallest magnitude which can satisfy x+y == a+b
    uint8_t end = posit8_bits(b) & 0x7f;
    for (int i = 0; i < end; i++) {
        posit8_t py = posit8_reinterpret(i);
        posit8_t ny = posit8_sub(ZERO8, py);
        double dpy = posit8_tod(py);
        double dny = posit8_tod(ny);

        for (int j = 0; j < (1<<8); j++) {
            posit8_t x = posit8_reinterpret(j);
            double dx = posit8_tod(x);
            if ((dx + dpy) == realsum) {
                if (posit8_cmp(ny, b) == 0) { continue; }
                return posit8x2(x, py);
            }
            if ((dx + dny) == realsum) {
                return posit8x2(x, ny);
            }
        }
    }
    if (loud) {
        posit8_t sum = posit8_add(a, b);
        printf("No better solution was found for %f %f sum=%f\n",
            posit8_tod(a), posit8_tod(b), posit8_tod(sum));
    }
    return posit8x2(a, b);
}

#define BUF_LEN 1024
typedef union workbuf_u {
    posit8_t posits[1024];
    uint8_t bytes[1024];
    uint32_t ints[1024/4];
} workbuf_t;

int cmp(const void* a, const void* b) {
    return posit8_cmp( posit8_abs( ((posit8_t*)b)[0] ),  posit8_abs( ((posit8_t*)a)[0] ) );
}

int add_compress(workbuf_t* buf) {
    int cycles = 0;
    int last_non_zero = BUF_LEN - 1;
    for (;;) {
        bool done = true;
        qsort(buf, last_non_zero+1, sizeof buf->posits[0], cmp);
        int next_last_non_zero = last_non_zero;
        // do this in n threads if you have them
        for (int i = 0; i <= last_non_zero; i += 2) {
            if (!buf->bytes[i]) { continue; }
            posit8x2_t x = posit8_add_exact_bruteforce(buf->posits[i], buf->posits[i+1], false);
            if (posit8_cmp(buf->posits[i], x.x)) { done = false; }
            buf->posits[i] = x.x;
            buf->posits[i+1] = x.y;
            next_last_non_zero = i+1;
        }
        last_non_zero = next_last_non_zero;
        cycles++;
        if (done) { break; }
    }
    return cycles;
}

int testrun(std::default_random_engine& random, uint64_t* sum) {
    workbuf_t buf;
    for (unsigned int i = 0; i < sizeof buf / 4; i++) { buf.ints[i] = random(); }
    // kill off the NaRs because they're boring
    for (unsigned int i = 0; i < sizeof buf; i++) { buf.bytes[i] += (buf.bytes[i] == 0x80); }

    double ref = 0;
    for (unsigned int i = 0; i < sizeof buf; i++) { ref += posit8_tod(buf.posits[i]); }

	int cycles = 0;
	uint64_t micros = 0;
#if !defined(_MSC_VER)
    struct timeval tv, tv2;
    gettimeofday(&tv, NULL);
    cycles = add_compress(&buf);
    gettimeofday(&tv2, NULL);
#endif

    int nonzero = 0;
    double result = 0;
    for (unsigned int i = 0; i < sizeof buf; i++) {
        if (!buf.bytes[i]) { continue; }
        nonzero++;
        double x = posit8_tod(buf.posits[i]);
        //printf("%f ", x);
        result += x;
    }

#if !defined(_MSC_VER)
    micros = (tv2.tv_usec - tv.tv_usec);
    micros += ((tv2.tv_sec - tv.tv_sec) * 1000000);
    *sum += micros;
#endif

    printf("%f == %f, 1024 posits compressed to %d in %d cycles \t%lld micros\n",
        ref, result, nonzero, cycles, (long long int)micros);
    return (ref == result) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main() {
    std::default_random_engine random;
    uint64_t sum = 0;
    int cycles = 32;
    for (int i = 0; i < cycles; i++) {
        if (testrun(random, &sum)) { return EXIT_FAILURE; }
    }
    printf("Average time = %lld\n", (long long int) sum / cycles);
}
