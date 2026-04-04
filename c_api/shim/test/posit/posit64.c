// posit64.c: C API coverage test for 64-bit posits
//
// Tests that all posit64 C API functions are callable and produce
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
	posit64_t pa, pb, pc;
	char str[posit64_str_SIZE];
	int failures = 0;

	// -- Special values: NAR and ZERO --
	pa = NAR64;
	posit64_str(str, pa);
	printf("NAR64  = %s\n", str);

	pa = ZERO64;
	posit64_str(str, pa);
	printf("ZERO64 = %s\n", str);

	// -- Arithmetic with special values --
	// NAR + 0 = NAR
	pc = posit64_add(NAR64, ZERO64);
	posit64_str(str, pc);
	printf("NAR + 0 = %s\n", str);

	// NAR * 0 = NAR
	pc = posit64_mul(NAR64, ZERO64);
	posit64_str(str, pc);
	printf("NAR * 0 = %s\n", str);

	// -- Conversion: from double and back --
	pa = posit64_fromd(1.0);
	if (posit64_tod(pa) != 1.0) {
		printf("FAIL: fromd(1.0) round-trip\n");
		++failures;
	}

	pa = posit64_fromd(-1.0);
	if (posit64_tod(pa) != -1.0) {
		printf("FAIL: fromd(-1.0) round-trip\n");
		++failures;
	}

	pa = posit64_fromd(0.0);
	if (posit64_tod(pa) != 0.0) {
		printf("FAIL: fromd(0.0) round-trip\n");
		++failures;
	}

	// -- Conversion: from long double --
	pa = posit64_fromld(3.14159265358979323846L);
	printf("pi = %s\n", (posit64_str(str, pa), str));

	// -- Arithmetic: representative values --
	pa = posit64_fromd(1.5);
	pb = posit64_fromd(2.5);

	pc = posit64_add(pa, pb);
	if (posit64_tod(pc) != 4.0) {
		printf("FAIL: 1.5 + 2.5 = %f (expected 4.0)\n", posit64_tod(pc));
		++failures;
	}

	pc = posit64_sub(pb, pa);
	if (posit64_tod(pc) != 1.0) {
		printf("FAIL: 2.5 - 1.5 = %f (expected 1.0)\n", posit64_tod(pc));
		++failures;
	}

	pc = posit64_mul(pa, pb);
	if (posit64_tod(pc) != 3.75) {
		printf("FAIL: 1.5 * 2.5 = %f (expected 3.75)\n", posit64_tod(pc));
		++failures;
	}

	pc = posit64_div(pb, pa);
	// 2.5 / 1.5 is not exactly representable; just check it's reasonable
	{
		double result = posit64_tod(pc);
		double expected = 2.5 / 1.5;
		double relerr = (result - expected) / expected;
		if (relerr > 1e-10 || relerr < -1e-10) {
			printf("FAIL: 2.5 / 1.5 = %f (expected ~%f)\n", result, expected);
			++failures;
		}
	}

	// -- Comparison --
	pa = posit64_fromd(1.0);
	pb = posit64_fromd(2.0);
	if (posit64_cmp(pa, pb) >= 0) {
		printf("FAIL: cmp(1.0, 2.0) should be negative\n");
		++failures;
	}
	if (posit64_cmp(pb, pa) <= 0) {
		printf("FAIL: cmp(2.0, 1.0) should be positive\n");
		++failures;
	}
	if (posit64_cmp(pa, pa) != 0) {
		printf("FAIL: cmp(1.0, 1.0) should be zero\n");
		++failures;
	}

	// -- String conversion --
	pa = posit64_fromd(42.0);
	posit64_str(str, pa);
	printf("42.0 = %s\n", str);

	// -- Reinterpret (bit pattern) --
	pa = posit64_reinterpret(0);
	if (posit64_tod(pa) != 0.0) {
		printf("FAIL: reinterpret(0) should be zero\n");
		++failures;
	}

	printf("posit64 C API: %s\n", failures ? "FAIL" : "PASS");
	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
