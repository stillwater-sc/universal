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

	// -- Arithmetic with special values: NAR propagates through all ops --
	pc = posit256_add(NAR256, ZERO256);
	if (posit256_cmp(pc, NAR256) != 0) {
		printf("FAIL: NAR + 0 should be NAR\n");
		++failures;
	}

	pc = posit256_sub(NAR256, ZERO256);
	if (posit256_cmp(pc, NAR256) != 0) {
		printf("FAIL: NAR - 0 should be NAR\n");
		++failures;
	}

	pc = posit256_mul(NAR256, ZERO256);
	if (posit256_cmp(pc, NAR256) != 0) {
		printf("FAIL: NAR * 0 should be NAR\n");
		++failures;
	}

	pc = posit256_div(NAR256, ZERO256);
	if (posit256_cmp(pc, NAR256) != 0) {
		printf("FAIL: NAR / 0 should be NAR\n");
		++failures;
	}

	// -- Conversion: use fromd/told (long double API).
	// On RISC-V, POWER, ARM: long double == double (64-bit).
	// On x86: long double is 80-bit extended precision.
	// The test values are exactly representable in 64-bit double.
	pa = posit256_fromd((long double)1.0);
	{
		long double val = posit256_told(pa);
		if (val != 1.0L) {
			printf("FAIL: fromd/told(1.0) round-trip\n");
			++failures;
		}
	}

	pa = posit256_fromd((long double)-1.0);
	{
		long double val = posit256_told(pa);
		if (val != -1.0L) {
			printf("FAIL: fromd/told(-1.0) round-trip\n");
			++failures;
		}
	}

	pa = posit256_fromd((long double)0.0);
	{
		long double val = posit256_told(pa);
		if (val != 0.0L) {
			printf("FAIL: fromd/told(0.0) round-trip\n");
			++failures;
		}
	}

	// -- Arithmetic: representative values --
	pa = posit256_fromd((long double)1.5);
	pb = posit256_fromd((long double)2.5);

	pc = posit256_add(pa, pb);
	{
		long double val = posit256_told(pc);
		if (val != 4.0L) {
			printf("FAIL: 1.5 + 2.5 = %Lf (expected 4.0)\n", val);
			++failures;
		}
	}

	pc = posit256_sub(pb, pa);
	{
		long double val = posit256_told(pc);
		if (val != 1.0L) {
			printf("FAIL: 2.5 - 1.5 = %Lf (expected 1.0)\n", val);
			++failures;
		}
	}

	pc = posit256_mul(pa, pb);
	{
		long double val = posit256_told(pc);
		if (val != 3.75L) {
			printf("FAIL: 1.5 * 2.5 = %Lf (expected 3.75)\n", val);
			++failures;
		}
	}

	pc = posit256_div(pb, pa);
	{
		long double val = posit256_told(pc);
		long double expected = 2.5L / 1.5L;
		long double relerr = (val - expected) / expected;
		if (relerr > 1e-10L || relerr < -1e-10L) {
			printf("FAIL: 2.5 / 1.5 = %Lf (expected ~%Lf)\n", val, expected);
			++failures;
		}
	}

	// -- Comparison --
	pa = posit256_fromd((long double)1.0);
	pb = posit256_fromd((long double)2.0);
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
	pa = posit256_fromd((long double)42.0);
	posit256_str(str, pa);
	printf("42.0 = %s\n", str);

	// -- Reinterpret (bit pattern) --
	pa = posit256_reinterpret( (uint64_t[]){ 0, 0, 0, 0 } );
	{
		long double val = posit256_told(pa);
		if (val != 0.0L) {
			printf("FAIL: reinterpret(0) should be zero\n");
			++failures;
		}
	}
	// Non-zero reinterpret: NAR encoding is MSB set in highest word
	pa = posit256_reinterpret( (uint64_t[]){ 0, 0, 0, 0x8000000000000000ULL } );
	if (posit256_cmp(pa, NAR256) != 0) {
		printf("FAIL: reinterpret(NAR pattern) should be NAR\n");
		++failures;
	}

	printf("posit256 C API: %s\n", failures ? "FAIL" : "PASS");
	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
