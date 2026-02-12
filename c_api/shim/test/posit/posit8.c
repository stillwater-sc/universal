// posit8.c: example test of the posit API for C programs using 8-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#include <universal/number/posit1/posit_c_api.h>
#include <math.h> // sqrt()

int main(int argc, char* argv[])
{
	posit8_t pa, pb, pc;
	char str[posit8_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR8;
	pb = ZERO8;
	pc = posit8_addp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 + 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit8_subp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 + 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit8_mulp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 + 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit8_divp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 + 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	// full state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_add(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da + db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmpp8(pref, pc)) {
				printf("FAIL: 8.0x%02xp + 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
				++fails;
			}
		}
	}
	if (fails) {
		printf("addition        FAIL\n");
		failures = true;
	}
	else {
		printf("addition        PASS\n");
	}

	// full state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_sub(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da - db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmpp8(pref, pc)) {
				printf("FAIL: 8.0x%02xp - 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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

	// full state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_mul(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da * db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmpp8(pref, pc)) {
				printf("FAIL: 8.0x%02xp * 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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

	// full state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_div(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da / db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmpp8(pref, pc)) {
				printf("FAIL: 8.0x%02xp / 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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

	// full state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {   // includes negative numbers
		pa = posit8_reinterpret(a);
		pc = posit8_sqrt(pa);
		double da, dref;
		da = posit8_tod(pa);
		dref = sqrt(da);
		posit8_t pref = posit8_fromd(dref);
		if (posit8_cmp(pref, pc)) {
			printf("FAIL: sqrt(8.0x%02xp) produced 8.0x%02xp instead of 8.0x%02xp\n",
	    posit8_bits(pa), posit8_bits(pc), posit8_bits(pref));
			++fails;
		}
	}
	if (fails) {
		printf("sqrt            FAIL\n");
		failures = true;
	}
	else {
		printf("sqrt            PASS\n");
	}

	// full state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {   // includes negative numbers
		pa = posit8_reinterpret(a);
		pc = posit8_exp(pa);
		double da, dref;
		da = posit8_tod(pa);
		dref = exp(da);
		posit8_t pref = posit8_fromd(dref);
		if (posit8_cmp(pref, pc)) {
			printf("FAIL: exp(8.0x%02xp) produced 8.0x%02xp instead of 8.0x%02xp\n",
				posit8_bits(pa), posit8_bits(pc), posit8_bits(pref));
			++fails;
		}
	}
	if (fails) {
		printf("exp             FAIL\n");
		failures = true;
	}
	else {
		printf("exp             PASS\n");
	}

	// full state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {   // includes negative numbers
		pa = posit8_reinterpret(a);
		pc = posit8_log(pa);
		double da, dref;
		da = posit8_tod(pa);
		dref = log(da);
		posit8_t pref = posit8_fromd(dref);
		if (posit8_cmp(pref, pc)) {
			printf("FAIL: log(8.0x%02xp) produced 8.0x%02xp instead of 8.0x%02xp\n",
				posit8_bits(pa), posit8_bits(pc), posit8_bits(pref));
			++fails;
		}
	}
	if (fails) {
		printf("log             FAIL\n");
		failures = true;
	}
	else {
		printf("log             PASS\n");
	}

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
