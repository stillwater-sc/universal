// posit128.c: example test of the posit API for C programs using 128-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>

int main(int argc, char* argv[]) 
{
	posit128_t pa, pb, pc;
	char str[POSIT_FORMAT128_SIZE];
	bool failures = false;
	bool bReportIndividualTestCases = false;

	// special case values
	pa = NAR128;
	pb = ZERO128;
	pc = posit_add128(pa, pb);
	posit_format128(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit_sub128(pa, pb);
	posit_format128(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit_mul128(pa, pb);
	posit_format128(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit_div128(pa, pb);
	posit_format128(pc, str);
	printf("posit value = %s\n", str);

	bool noReference = true;
	printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<<128,4>>\n", sizeof(long double));

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_assign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = posit_assign128(b, 0);
			pc = posit_add128(pa, pb);

			long double da, db, dref;
			da = posit_value128(pa);
			db = posit_value128(pb);
			dref = da + db;

			posit128_t pref = posit_assign128f(dref);
			if (posit_equal128(pref, pc)) {
				char sa[POSIT_FORMAT128_SIZE], sb[POSIT_FORMAT128_SIZE], sc[POSIT_FORMAT128_SIZE], sref[POSIT_FORMAT128_SIZE];
				posit_format128(pa, sa);
				posit_format128(pb, sb);
				posit_format128(pc, sc);
				posit_format128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s + %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (noReference) {
			printf("addition        uncertain\n");
		}
		else {
			printf("addition        FAIL\n");
			failures = true;
		}
	}
	else {
		printf("addition        PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_assign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = posit_assign128(b, 0);
			pc = posit_sub128(pa, pb);

			long double da, db, dref;
			da = posit_value128(pa);
			db = posit_value128(pb);
			dref = da - db;

			posit128_t pref = posit_assign128f(dref);
			if (posit_equal128(pref, pc)) {
				char sa[POSIT_FORMAT128_SIZE], sb[POSIT_FORMAT128_SIZE], sc[POSIT_FORMAT128_SIZE], sref[POSIT_FORMAT128_SIZE];
				posit_format128(pa, sa);
				posit_format128(pb, sb);
				posit_format128(pc, sc);
				posit_format128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s - %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (noReference) {
			printf("subtraction     uncertain\n");
		}
		else {
			printf("subtraction     FAIL\n");
			failures = true;
		}
	}
	else {
		printf("subtraction     PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_assign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = posit_assign128(b, 0);
			pc = posit_mul128(pa, pb);

			long double da, db, dref;
			da = posit_value128(pa);
			db = posit_value128(pb);
			dref = da * db;

			posit128_t pref = posit_assign128f(dref);
			if (posit_equal128(pref, pc)) {
				char sa[POSIT_FORMAT128_SIZE], sb[POSIT_FORMAT128_SIZE], sc[POSIT_FORMAT128_SIZE], sref[POSIT_FORMAT128_SIZE];
				posit_format128(pa, sa);
				posit_format128(pb, sb);
				posit_format128(pc, sc);
				posit_format128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s * %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (noReference) {
			printf("multiplication  uncertain\n");
		}
		else {
			printf("multiplication  FAIL\n");
			failures = true;
		}
	}
	else {
		printf("multiplication  PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_assign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = posit_assign128(b, 0);
			pc = posit_div128(pa, pb);

			long double da, db, dref;
			da = posit_value128(pa);
			db = posit_value128(pb);
			dref = da / db;

			posit128_t pref = posit_assign128f(dref);
			if (posit_equal128(pref, pc)) {
				char sa[POSIT_FORMAT128_SIZE], sb[POSIT_FORMAT128_SIZE], sc[POSIT_FORMAT128_SIZE], sref[POSIT_FORMAT128_SIZE];
				posit_format128(pa, sa);
				posit_format128(pb, sb);
				posit_format128(pc, sc);
				posit_format128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s / %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (noReference) {
			printf("division        uncertain\n");
		}
		else {
			printf("division        FAIL\n");
			failures = true;
		}
	}
	else {
		printf("division        PASS\n");
	}

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
