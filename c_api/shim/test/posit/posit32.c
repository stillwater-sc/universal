// posit32.c: example test of the posit API for C programs using 32-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#endif
#include <universal/number/posit1/posit_c_api.h>

int main(int argc, char* argv[])
{
	posit32_t pa, pb, pc;
	char str[posit32_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR32;
	pb = ZERO32;
	pc = posit32_add(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", posit32_bits(pc));

	pa = NAR32;
	pb = ZERO32;
	pc = posit32_sub(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", posit32_bits(pc));

	pa = NAR32;
	pb = ZERO32;
	pc = posit32_mul(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", posit32_bits(pc));

	pa = NAR32;
	pb = ZERO32;
	pc = posit32_div(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", posit32_bits(pc));

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit32_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit32_reinterpret(b);
			pc = posit32_add(pa, pb);
			double da, db, dref;
			da = posit32_tod(pa);
			db = posit32_tod(pb);
			dref = da + db;
			posit32_t pref = posit32_fromd(dref);
			if (posit32_cmp(pref, pc)) {
				printf("FAIL: 32.2x%08xp + 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n",
                    posit32_bits(pa), posit32_bits(pb), posit32_bits(pc), posit32_bits(pref));
				++fails;
				break;
			}
		}
		if (fails) break;
	}
	if (fails) {
		printf("addition        FAIL\n");
		failures = true;
	}
	else {
		printf("addition        PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit32_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit32_reinterpret(b);
			pc = posit32_sub(pa, pb);
			double da, db, dref;
			da = posit32_tod(pa);
			db = posit32_tod(pb);
			dref = da - db;
			posit32_t pref = posit32_fromd(dref);
			if (pref.v != pc.v) {
				printf("FAIL: 32.2x%08xp - 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n",
                    posit32_bits(pa), posit32_bits(pb), posit32_bits(pc), posit32_bits(pref));
				++fails;
			}
		}
	}
	if (fails) {
		printf("subtraction     FAIL\n");
		failures = true;
	}
	else {
		printf("subtraction     PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit32_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit32_reinterpret(b);
			pc = posit32_mul(pa, pb);
			double da, db, dref;
			da = posit32_tod(pa);
			db = posit32_tod(pb);
			dref = da * db;
			posit32_t pref = posit32_fromd(dref);
			if (posit32_cmp(pref, pc) != 0) {
				printf("FAIL: 32.2x%08xp * 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n",
                    posit32_bits(pa), posit32_bits(pb), posit32_bits(pc), posit32_bits(pref));
				++fails;
			}
		}
	}
	if (fails) {
		printf("multiplication  FAIL\n");
		failures = true;
	}
	else {
		printf("multiplication  PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit32_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit32_reinterpret(b);
			pc = posit32_div(pa, pb);
			double da, db, dref;
			da = posit32_tod(pa);
			db = posit32_tod(pb);
			dref = da / db;
			posit32_t pref = posit32_fromd(dref);
			if (posit32_cmp(pref, pc) != 0) {
				printf("FAIL: 32.2x%08xp / 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n",
                    posit32_bits(pa), posit32_bits(pb), posit32_bits(pc), posit32_bits(pref));
				++fails;
			}
		}
	}
	if (fails) {
		printf("division        FAIL\n");
		failures = true;
	}
	else {
		printf("division        PASS\n");
	}

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
