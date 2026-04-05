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

	// -- Arithmetic with special values: NAR propagates through all ops --
	pc = posit64_add(NAR64, ZERO64);
	if (posit64_cmp(pc, NAR64) != 0) {
		printf("FAIL: NAR + 0 should be NAR\n");
		++failures;
	}

	pc = posit64_sub(NAR64, ZERO64);
	if (posit64_cmp(pc, NAR64) != 0) {
		printf("FAIL: NAR - 0 should be NAR\n");
		++failures;
	}

	pc = posit64_mul(NAR64, ZERO64);
	if (posit64_cmp(pc, NAR64) != 0) {
		printf("FAIL: NAR * 0 should be NAR\n");
		++failures;
	}

	pc = posit64_div(NAR64, ZERO64);
	if (posit64_cmp(pc, NAR64) != 0) {
		printf("FAIL: NAR / 0 should be NAR\n");
		++failures;
	}

	// -- Conversion: long double round-trip (posit64 API uses long double) --
	pa = posit64_fromd((long double)1.0);
	{
		long double val = posit64_told(pa);
		if (val != 1.0L) {
			printf("FAIL: fromd/told(1.0) round-trip\n");
			++failures;
		}
	}

	pa = posit64_fromd((long double)-1.0);
	{
		long double val = posit64_told(pa);
		if (val != -1.0L) {
			printf("FAIL: fromd/told(-1.0) round-trip\n");
			++failures;
		}
	}

	pa = posit64_fromd((long double)0.0);
	{
		long double val = posit64_told(pa);
		if (val != 0.0L) {
			printf("FAIL: fromd/told(0.0) round-trip\n");
			++failures;
		}
	}

	// -- Arithmetic: representative values --
	pa = posit64_fromd((long double)1.5);
	pb = posit64_fromd((long double)2.5);

	pc = posit64_add(pa, pb);
	{
		long double val = posit64_told(pc);
		if (val != 4.0L) {
			printf("FAIL: 1.5 + 2.5 = %Lf (expected 4.0)\n", val);
			++failures;
		}
	}

	pc = posit64_sub(pb, pa);
	{
		long double val = posit64_told(pc);
		if (val != 1.0L) {
			printf("FAIL: 2.5 - 1.5 = %Lf (expected 1.0)\n", val);
			++failures;
		}
	}

	pc = posit64_mul(pa, pb);
	{
		long double val = posit64_told(pc);
		if (val != 3.75L) {
			printf("FAIL: 1.5 * 2.5 = %Lf (expected 3.75)\n", val);
			++failures;
		}
	}

	pc = posit64_div(pb, pa);
	{
		long double val = posit64_told(pc);
		long double expected = 2.5L / 1.5L;
		long double relerr = (val - expected) / expected;
		if (relerr > 1e-10L || relerr < -1e-10L) {
			printf("FAIL: 2.5 / 1.5 = %Lf (expected ~%Lf)\n", val, expected);
			++failures;
		}
	}

	// -- Comparison --
	pa = posit64_fromd((long double)1.0);
	pb = posit64_fromd((long double)2.0);
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
	pa = posit64_fromd((long double)42.0);
	posit64_str(str, pa);
	printf("42.0 = %s\n", str);

	// -- Reinterpret (bit pattern) --
	pa = posit64_reinterpret(0);
	{
		long double val = posit64_told(pa);
		if (val != 0.0L) {
			printf("FAIL: reinterpret(0) should be zero\n");
			++failures;
		}
	}

	printf("posit64 C API: %s\n", failures ? "FAIL" : "PASS");
	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
