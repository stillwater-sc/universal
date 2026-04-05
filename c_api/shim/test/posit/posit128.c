// posit128.c: C API coverage test for 128-bit posits
//
// Tests that all posit128 C API functions are callable and produce
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
	posit128_t pa, pb, pc;
	char str[posit128_str_SIZE];
	int failures = 0;

	// -- Special values: NAR and ZERO --
	pa = NAR128;
	posit128_str(str, pa);
	printf("NAR128  = %s\n", str);

	pa = ZERO128;
	posit128_str(str, pa);
	printf("ZERO128 = %s\n", str);

	// -- Arithmetic with special values --
	pc = posit128_add(NAR128, ZERO128);
	posit128_str(str, pc);
	printf("NAR + 0 = %s\n", str);

	pc = posit128_mul(NAR128, ZERO128);
	posit128_str(str, pc);
	printf("NAR * 0 = %s\n", str);

	// -- Conversion: use double (not long double) because long double
	// is only 64-bit on RISC-V, POWER, and ARM, which loses precision
	// for wide posit types. The values used here are exactly
	// representable in double.
	pa = posit128_fromd(1.0);
	if (posit128_tod(pa) != 1.0) {
		printf("FAIL: fromd(1.0) round-trip\n");
		++failures;
	}

	pa = posit128_fromd(-1.0);
	if (posit128_tod(pa) != -1.0) {
		printf("FAIL: fromd(-1.0) round-trip\n");
		++failures;
	}

	pa = posit128_fromd(0.0);
	if (posit128_tod(pa) != 0.0) {
		printf("FAIL: fromd(0.0) round-trip\n");
		++failures;
	}

	// -- Long double API: just verify it's callable --
	pa = posit128_fromld(3.14L);
	posit128_str(str, pa);
	printf("fromld(3.14) = %s\n", str);

	// -- Arithmetic: representative values --
	pa = posit128_fromd(1.5);
	pb = posit128_fromd(2.5);

	pc = posit128_add(pa, pb);
	if (posit128_tod(pc) != 4.0) {
		printf("FAIL: 1.5 + 2.5 = %f (expected 4.0)\n", posit128_tod(pc));
		++failures;
	}

	pc = posit128_sub(pb, pa);
	if (posit128_tod(pc) != 1.0) {
		printf("FAIL: 2.5 - 1.5 = %f (expected 1.0)\n", posit128_tod(pc));
		++failures;
	}

	pc = posit128_mul(pa, pb);
	if (posit128_tod(pc) != 3.75) {
		printf("FAIL: 1.5 * 2.5 = %f (expected 3.75)\n", posit128_tod(pc));
		++failures;
	}

	pc = posit128_div(pb, pa);
	{
		double result = posit128_tod(pc);
		double expected = 2.5 / 1.5;
		double relerr = (result - expected) / expected;
		if (relerr > 1e-10 || relerr < -1e-10) {
			printf("FAIL: 2.5 / 1.5 = %f (expected ~%f)\n", result, expected);
			++failures;
		}
	}

	// -- Comparison --
	pa = posit128_fromd(1.0);
	pb = posit128_fromd(2.0);
	if (posit128_cmp(pa, pb) >= 0) {
		printf("FAIL: cmp(1.0, 2.0) should be negative\n");
		++failures;
	}
	if (posit128_cmp(pb, pa) <= 0) {
		printf("FAIL: cmp(2.0, 1.0) should be positive\n");
		++failures;
	}
	if (posit128_cmp(pa, pa) != 0) {
		printf("FAIL: cmp(1.0, 1.0) should be zero\n");
		++failures;
	}

	// -- String conversion --
	pa = posit128_fromd(42.0);
	posit128_str(str, pa);
	printf("42.0 = %s\n", str);

	// -- Reinterpret (bit pattern) --
	pa = posit128_reinterpret( (uint64_t[]){ 0, 0 } );
	if (posit128_tod(pa) != 0.0) {
		printf("FAIL: reinterpret(0) should be zero\n");
		++failures;
	}

	printf("posit128 C API: %s\n", failures ? "FAIL" : "PASS");
	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
