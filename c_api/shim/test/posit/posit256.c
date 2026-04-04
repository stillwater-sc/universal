// posit256.c: C API coverage test for 256-bit posits
//
// Tests that all posit256 C API functions are callable and produce
// correct results for representative values. Arithmetic correctness
// is validated by the posit regression suite, not here.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#define POSIT_NO_GENERICS
#include <universal/number/posit1/posit_c_api.h>

int main(int argc, char* argv[])
{
	(void)argc; (void)argv;
	posit256_t pa, pb, pc;
	char str[posit256_str_SIZE];
	int failures = 0;

	// -- Special values: NAR and ZERO --
	pa = NAR256;
	posit256_str(str, pa);
	printf("NAR256  = %s\n", str);

	pa = ZERO256;
	posit256_str(str, pa);
	printf("ZERO256 = %s\n", str);

	// -- Arithmetic with special values --
	pc = posit256_add(NAR256, ZERO256);
	posit256_str(str, pc);
	printf("NAR + 0 = %s\n", str);

	pc = posit256_mul(NAR256, ZERO256);
	posit256_str(str, pc);
	printf("NAR * 0 = %s\n", str);

	// -- Conversion: from long double and back --
	pa = posit256_fromld(1.0L);
	if (posit256_told(pa) != 1.0L) {
		printf("FAIL: fromld(1.0) round-trip\n");
		++failures;
	}

	pa = posit256_fromld(-1.0L);
	if (posit256_told(pa) != -1.0L) {
		printf("FAIL: fromld(-1.0) round-trip\n");
		++failures;
	}

	pa = posit256_fromld(0.0L);
	if (posit256_told(pa) != 0.0L) {
		printf("FAIL: fromld(0.0) round-trip\n");
		++failures;
	}

	// -- Arithmetic: representative values --
	pa = posit256_fromld(1.5L);
	pb = posit256_fromld(2.5L);

	pc = posit256_add(pa, pb);
	if (posit256_told(pc) != 4.0L) {
		printf("FAIL: 1.5 + 2.5 = %Lf (expected 4.0)\n", posit256_told(pc));
		++failures;
	}

	pc = posit256_sub(pb, pa);
	if (posit256_told(pc) != 1.0L) {
		printf("FAIL: 2.5 - 1.5 = %Lf (expected 1.0)\n", posit256_told(pc));
		++failures;
	}

	pc = posit256_mul(pa, pb);
	if (posit256_told(pc) != 3.75L) {
		printf("FAIL: 1.5 * 2.5 = %Lf (expected 3.75)\n", posit256_told(pc));
		++failures;
	}

	pc = posit256_div(pb, pa);
	{
		long double result = posit256_told(pc);
		long double expected = 2.5L / 1.5L;
		long double relerr = (result - expected) / expected;
		if (relerr > 1e-15L || relerr < -1e-15L) {
			printf("FAIL: 2.5 / 1.5 = %Lf (expected ~%Lf)\n", result, expected);
			++failures;
		}
	}

	// -- Comparison --
	pa = posit256_fromld(1.0L);
	pb = posit256_fromld(2.0L);
	if (posit256_cmp(pa, pb) >= 0) {
		printf("FAIL: cmp(1.0, 2.0) should be negative\n");
		++failures;
	}
	if (posit256_cmp(pb, pa) <= 0) {
		printf("FAIL: cmp(2.0, 1.0) should be positive\n");
		++failures;
	}
	if (posit256_cmp(pa, pa) != 0) {
		printf("FAIL: cmp(1.0, 1.0) should be zero\n");
		++failures;
	}

	// -- String conversion --
	pa = posit256_fromld(42.0L);
	posit256_str(str, pa);
	printf("42.0 = %s\n", str);

	// -- Reinterpret (bit pattern) --
	pa = posit256_reinterpret( (uint64_t[]){ 0, 0, 0, 0 } );
	if (posit256_told(pa) != 0.0L) {
		printf("FAIL: reinterpret(0) should be zero\n");
		++failures;
	}

	printf("posit256 C API: %s\n", failures ? "FAIL" : "PASS");
	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
