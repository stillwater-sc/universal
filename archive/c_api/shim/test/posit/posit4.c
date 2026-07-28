// posit4.c: example test of the posit API for C programs using 4-bit posits
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
	const int maxNr = 16;
	posit4_t pa, pb, pc;
	char str[posit4_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR4;
	pb = ZERO4;
	pc = posit4_add(pa, pb);
	posit4_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 4.0x%1xp\n", posit4_bits(pc));

	pa = NAR4;
	pb = ZERO4;
	pc = posit4_sub(pa, pb);
	posit4_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 4.0x%1xp\n", posit4_bits(pc));

	pa = NAR4;
	pb = ZERO4;
	pc = posit4_mul(pa, pb);
	posit4_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 4.0x%1xp\n", posit4_bits(pc));

	pa = NAR4;
	pb = ZERO4;
	pc = posit4_div(pa, pb);
	posit4_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 4.0x%1xp\n", posit4_bits(pc));


	// full state space
	int fails = 0;
	for (int a = 0; a < maxNr; ++a) {
		pa = posit4_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit4_reinterpret(b);
			pc = posit4_add(pa, pb);
			float da, db, dref;
			da = posit4_tof(pa);
			db = posit4_tof(pb);
			dref = da + db;
			posit4_t pref = posit4_fromf(dref);
			if (posit4_cmp(pref, pc)) {
				printf("FAIL: 4.0x%1xp + 4.0x%1xp produced 4.0x%1xp instead of 4.0x%1xp\n",
                    posit4_bits(pa), posit4_bits(pb), posit4_bits(pc), posit4_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit4_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit4_reinterpret(b);
			pc = posit4_sub(pa, pb);
			float da, db, dref;
			da = posit4_tof(pa);
			db = posit4_tof(pb);
			dref = da - db;
			posit4_t pref = posit4_fromf(dref);
			if (posit4_cmp(pref, pc)) {
				printf("FAIL: 4.0x%1xp - 4.0x%1xp produced 4.0x%1xp instead of 4.0x%1xp\n",
                    posit4_bits(pa), posit4_bits(pb), posit4_bits(pc), posit4_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit4_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit4_reinterpret(b);
			pc = posit4_mul(pa, pb);
			float da, db, dref;
			da = posit4_tof(pa);
			db = posit4_tof(pb);
			dref = da * db;
			posit4_t pref = posit4_fromf(dref);
			if (posit4_cmp(pref, pc)) {
				printf("FAIL: 4.0x%1xp * 4.0x%1xp produced 4.0x%1xp instead of 4.0x%1xp\n",
                    posit4_bits(pa), posit4_bits(pb), posit4_bits(pc), posit4_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit4_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit4_reinterpret(b);
			pc = posit4_div(pa, pb);
			float da, db, dref;
			da = posit4_tof(pa);
			db = posit4_tof(pb);
			dref = da / db;
			posit4_t pref = posit4_fromf(dref);
			if (posit4_cmp(pref, pc)) {
				printf("FAIL: 4.0x%1xp / 4.0x%1xp produced 4.0x%1xp instead of 4.0x%1xp\n",
                    posit4_bits(pa), posit4_bits(pb), posit4_bits(pc), posit4_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit4_reinterpret(a);
		pc = posit4_sqrt(pa);
		double da, dref;
		da = posit4_tod(pa);
		dref = sqrt(da);
		posit4_t pref = posit4_fromd(dref);
		if (posit4_cmp(pref, pc)) {
			printf("FAIL: sqrt(4.0x%1xp) produced 4.0x%1xp instead of 4.0x%1xp\n",
	    posit4_bits(pa), posit4_bits(pc), posit4_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {   // includes negative numbers
		pa = posit4_reinterpret(a);
		pc = posit4_exp(pa);
		double da, dref;
		da = posit4_tod(pa);
		dref = exp(da);
		posit4_t pref = posit4_fromd(dref);
		if (posit4_cmp(pref, pc)) {
			printf("FAIL: exp(4.0x%1xp) produced 4.0x%1xp instead of 4.0x%1xpp\n",
				posit4_bits(pa), posit4_bits(pc), posit4_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {   // includes negative numbers
		pa = posit4_reinterpret(a);
		pc = posit4_log(pa);
		double da, dref;
		da = posit4_tod(pa);
		dref = log(da);
		posit4_t pref = posit4_fromd(dref);
		if (posit4_cmp(pref, pc)) {
			printf("FAIL: log(4.0x%1xp) produced 4.0x%1xp instead of 4.0x%1xp\n",
				posit4_bits(pa), posit4_bits(pc), posit4_bits(pref));
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
