// posit8.c: example test of the posit API for C programs using 8-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>

int main(int argc, char* argv[])
{
	posit8_t pa, pb, pc;
	char str[posit8_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR8;
	pb = ZERO8;
	pc = posit_add(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 8.0x%02xp\n", posit_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit_sub(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 8.0x%02xp\n", posit_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit_mul(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 8.0x%02xp\n", posit_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit_div(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 8.0x%02xp\n", posit_bits(pc));


	// full state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit_add(pa, pb);
			float da, db, dref;
			da = posit_tod(pa);
			db = posit_tod(pb);
			dref = da + db;
			posit8_t pref = posit8_reinterpret(dref);
			if (posit_cmp(pref, pc)) {
				printf("FAIL: 8.0x%02xp + 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit_bits(pa), posit_bits(pb), posit_bits(pc), posit_bits(pref));
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
			pc = posit_sub(pa, pb);
			float da, db, dref;
			da = posit_tod(pa);
			db = posit_tod(pb);
			dref = da - db;
			posit8_t pref = posit8_reinterpret(dref);
			if (posit_cmp(pref, pc)) {
				printf("FAIL: 8.0x%02xp - 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit_bits(pa), posit_bits(pb), posit_bits(pc), posit_bits(pref));
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
			pc = posit_mul(pa, pb);
			float da, db, dref;
			da = posit_tod(pa);
			db = posit_tod(pb);
			dref = da * db;
			posit8_t pref = posit8_reinterpret(dref);
			if (posit_cmp(pref, pc)) {
				printf("FAIL: 8.0x%02xp * 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit_bits(pa), posit_bits(pb), posit_bits(pc), posit_bits(pref));
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
			pc = posit_div(pa, pb);
			float da, db, dref;
			da = posit_tod(pa);
			db = posit_tod(pb);
			dref = da / db;
			posit8_t pref = posit8_reinterpret(dref);
			if (posit_cmp(pref, pc)) {
				printf("FAIL: 8.0x%02xp / 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit_bits(pa), posit_bits(pb), posit_bits(pc), posit_bits(pref));
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
