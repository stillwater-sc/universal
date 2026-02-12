// posit8.c: example program showing the use of the posit8_t type of the C API of the posit library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// pull in the posit C API definitions
#include <universal/number/posit1/posit_c_api.h>

// pull in the source code to be compiled as a C library
#include <universal/number/posit1/specialized/posit_8_0.h>

// elementary functions
posit8_t posit8_sqrt(posit8_t a) {
	posit8_t p;
	float f = posit8_tof(a);
	float root = sqrtf(f);
	p = posit8_fromf(root);
	return p;
}
posit8_t posit8_log(posit8_t a) {
	posit8_t p;
	float f = posit8_tof(a);
	float logarithm = logf(f);
	p = posit8_fromf(logarithm);
	return p;
}
posit8_t posit8_exp(posit8_t a) {
	posit8_t p;
	float f = posit8_tof(a);
	float exponent = expf(f);
	p = posit8_fromf(exponent);
	return p;
}
// logic functions
// cmp returns -1 if a < b, 0 if a == b, and 1 if a > b
int posit8_cmpp8(posit8_t a, posit8_t b) {
	// posits are ordered as signed integers
	return a.v - b.v;
}

#if defined(__cplusplus) || defined(_MSC_VER)
// string conversion functions
void posit8_str(char str[16], posit8_t a) {
	float f = posit8_tof(a);
	sprintf(str, "%f", f);
}
#else
// string conversion functions
void posit8_str(char str[static 16], posit8_t a) {
	float f = posit8_tof(a);
	sprintf(str, "%f", f);
}
#endif
