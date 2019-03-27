// posit8.c: example test of the posit API for C programs using 8-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>

int main(int argc, char* argv[]) 
{
	posit8_t pa, pb, pc;
	char str[POSIT_FORMAT8_SIZE];
	bool failures = false;

	// special case values
	pa = NAR8;
	pb = ZERO8;
	pc = padd8(pa, pb);
	pformat8(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR8;
	pb = ZERO8;
	pc = psub8(pa, pb);
	pformat8(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR8;
	pb = ZERO8;
	pc = pmul8(pa, pb);
	pformat8(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR8;
	pb = ZERO8;
	pc = pdiv8(pa, pb);
	pformat8(pc, str);
	printf("posit value = %s\n", str);

	// full state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = (posit8_t)a;
		for (int b = 0; b < 256; ++b) {
			pb = (posit8_t)b;
			pc = padd8(pa, pb);

			double da, db, dref;
			da = pvalue8(pa);
			db = pvalue8(pb);
			dref = da + db;

			posit8_t pref = passign8f((float)dref);
			if (pref != pc) {
				printf("FAIL: 8.0x%02xp + 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n", pa, pb, pc, pref);
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
		pa = (posit8_t)a;
		for (int b = 0; b < 256; ++b) {
			pb = (posit8_t)b;
			pc = psub8(pa, pb);

			double da, db, dref;
			da = pvalue8(pa);
			db = pvalue8(pb);
			dref = da - db;

			posit8_t pref = passign8f((float)dref);
			if (pref != pc) {
				printf("FAIL: 8.0x%02xp - 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n", pa, pb, pc, pref);
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
		pa = (posit8_t)a;
		for (int b = 0; b < 256; ++b) {
			pb = (posit8_t)b;
			pc = pmul8(pa, pb);

			double da, db, dref;
			da = pvalue8(pa);
			db = pvalue8(pb);
			dref = da * db;

			posit8_t pref = passign8f((float)dref);
			if (pref != pc) {
				printf("FAIL: 8.0x%02xp * 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n", pa, pb, pc, pref);
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
		pa = (posit8_t)a;
		for (int b = 0; b < 256; ++b) {
			pb = (posit8_t)b;
			pc = pdiv8(pa, pb);

			double da, db, dref;
			da = pvalue8(pa);
			db = pvalue8(pb);
			dref = da / db;

			posit8_t pref = passign8f((float)dref);
			if (pref != pc) {
				printf("FAIL: 8.0x%02xp / 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n", pa, pb, pc, pref);
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
