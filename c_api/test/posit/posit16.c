// posit16.c: example test of the posit API for C programs using 16-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#include <posit_c_api.h>

int main(int argc, char* argv[])
{
	posit16_t pa, pb, pc;
	char str[posit16_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR16;
	pb = ZERO16;
	pc = posit16_add(pa, pb);
	posit16_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 16.1x%04xp\n", posit16_bits(pc));

	pa = NAR16;
	pb = ZERO16;
	pc = posit16_sub(pa, pb);
	posit16_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 16.1x%04xp\n", posit16_bits(pc));

	pa = NAR16;
	pb = ZERO16;
	pc = posit16_mul(pa, pb);
	posit16_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 16.1x%04xp\n", posit16_bits(pc));

	pa = NAR16;
	pb = ZERO16;
	pc = posit16_div(pa, pb);
	posit16_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 16.1x%04xp\n", posit16_bits(pc));


	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
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
	for (int a = 0; a < 256; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
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
	for (int a = 0; a < 256; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
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
	for (int a = 0; a < 256; ++a) {
		pa = posit16_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
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

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
