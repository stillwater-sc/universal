// posit16.c: example test of the posit API for C programs using 16-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#include <universal/number/posit1/posit_c_api.h>
#include <math.h>

int main(int argc, char* argv[])
{
	const int maxNr = 512;
	const int fullState = 256*256;
	posit16_t pa, pb, pc;
	char str[posit16_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR16;
	pb = ZERO16;
	pc = posit16_add(pa, pb);
	posit16_str(str, pc);
	printf("NAR16 + 0 = %s (16.1x%04xp)\n", str, posit16_bits(pc));

	pa = NAR16;
	pb = ZERO16;
	pc = posit16_sub(pa, pb);
	posit16_str(str, pc);
	printf("NAR16 + 0 = %s (16.1x%04xp)\n", str, posit16_bits(pc));

	pa = NAR16;
	pb = ZERO16;
	pc = posit16_mul(pa, pb);
	posit16_str(str, pc);
	printf("NAR16 + 0 = %s (16.1x%04xp)\n", str, posit16_bits(pc));

	pa = NAR16;
	pb = ZERO16;
	pc = posit16_div(pa, pb);
	posit16_str(str, pc);
	printf("NAR16 + 0 = %s (16.1x%04xp)\n", str, posit16_bits(pc));

	// partial state space
	int fails = 0;
	for (int a = 0; a < maxNr; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit16_reinterpret(b);
			pc = posit16_add(pa, pb);
			float da, db, dref;
			da = posit16_tof(pa);
			db = posit16_tof(pb);
			dref = da + db;
			posit16_t pref = posit16_fromf(dref);
			if (posit16_cmp(pref, pc)) {
				printf("FAIL: 16.1x%04xp + 16.1x%04xp produced 16.1x%04xp instead of 16.1x%04xp\n",
                    posit16_bits(pa), posit16_bits(pb), posit16_bits(pc), posit16_bits(pref));
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

	// partial state space
	fails = 0;
	for (int a = 0; a < maxNr; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit16_reinterpret(b);
			pc = posit16_sub(pa, pb);
			float da, db, dref;
			da = posit16_tof(pa);
			db = posit16_tof(pb);
			dref = da - db;
			posit16_t pref = posit16_fromf(dref);
			if (posit16_cmp(pref, pc)) {
				printf("FAIL: 16.1x%04xp - 16.1x%04xp produced 16.1x%04xp instead of 16.1x%04xp\n",
                    posit16_bits(pa), posit16_bits(pb), posit16_bits(pc), posit16_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit16_reinterpret(b);
			pc = posit16_mul(pa, pb);
			float da, db, dref;
			da = posit16_tof(pa);
			db = posit16_tof(pb);
			dref = da * db;
			posit16_t pref = posit16_fromf(dref);
			if (posit16_cmp(pref, pc)) {
				printf("FAIL: 16.1x%04xp * 16.1x%04xp produced 16.1x%04xp instead of 16.1x%04xp\n",
                    posit16_bits(pa), posit16_bits(pb), posit16_bits(pc), posit16_bits(pref));
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit16_reinterpret(b);
			pc = posit16_div(pa, pb);
			float da, db, dref;
			da = posit16_tof(pa);
			db = posit16_tof(pb);
			dref = da / db;
			posit16_t pref = posit16_fromf(dref);
			if (posit16_cmp(pref, pc)) {
				printf("FAIL: 16.1x%04xp / 16.1x%04xp produced 16.1x%04xp instead of 16.1x%04xp\n",
                    posit16_bits(pa), posit16_bits(pb), posit16_bits(pc), posit16_bits(pref));
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
	for (int a = 0; a < fullState; ++a) {   // includes negative numbers
		pa = posit16_reinterpret(a);
		pc = posit16_sqrt(pa);
		double da, dref;
		da = posit16_tod(pa);
		dref = sqrt(da);
		posit16_t pref = posit16_fromd(dref);
		if (posit16_cmp(pref, pc)) {
			printf("FAIL: sqrt(16.1x%04xp) produced 16.1x%04xp instead of 16.1x%04xp\n",
				posit16_bits(pa), posit16_bits(pc), posit16_bits(pref));
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
	for (int a = 0; a < fullState; ++a) {   // includes negative numbers
		pa = posit16_reinterpret(a);
		pc = posit16_exp(pa);
		double da, dref;
		da = posit16_tod(pa);
		dref = exp(da);
		posit16_t pref = posit16_fromd(dref);
		if (posit16_cmp(pref, pc)) {
			if (dref > 0.0) {
			    printf("FAIL: exp(16.1x%04xp) produced 16.1x%04xp instead of 16.1x%04xp\n",
			    	posit16_bits(pa), posit16_bits(pc), posit16_bits(pref));
			    ++fails;
			} // special case of posit rounding to minpos
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
	for (int a = 0; a < fullState; ++a) {   // includes negative numbers
		pa = posit16_reinterpret(a);
		pc = posit16_log(pa);
		double da, dref;
		da = posit16_tod(pa);
		dref = log(da);
		posit16_t pref = posit16_fromd(dref);
		if (posit16_cmp(pref, pc)) {
			printf("FAIL: log(16.1x%04xp) produced 16.1x%04xp instead of 16.1x%04xp\n",
				posit16_bits(pa), posit16_bits(pc), posit16_bits(pref));
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
