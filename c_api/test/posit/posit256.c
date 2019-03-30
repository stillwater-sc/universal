// posit256.c: example test of the posit API for C programs using 256-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>

int main(int argc, char* argv[]) 
{
	const int maxNr = 196;
	posit256_t pa, pb, pc;
	char str[POSIT_FORMAT256_SIZE];
	bool failures = false;
	bool bReportIndividualTestCases = false;

	// special case values
	pa = NAR256;
	pb = ZERO256;
	pc = posit_add256(pa, pb);
	posit_format256(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR256;
	pb = ZERO256;
	pc = posit_sub256(pa, pb);
	posit_format256(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR256;
	pb = ZERO256;
	pc = posit_mul256(pa, pb);
	posit_format256(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR256;
	pb = ZERO256;
	pc = posit_div256(pa, pb);
	posit_format256(pc, str);
	printf("posit value = %s\n", str);

	bool noReference = true;
	printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<<256,4>>\n", sizeof(long double));

	// partial state space
	int fails = 0;
	for (int a = 0; a < maxNr; ++a) {
		pa = posit_assign256(a, 0, 0, 0);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit_assign256(b, 0, 0, 0);
			pc = posit_add256(pa, pb);

			long double da, db, dref;
			da = posit_value256(pa);
			db = posit_value256(pb);
			dref = da + db;

			posit256_t pref = posit_assign256f(dref);
			if (posit_equal256(pref, pc)) {
				char sa[POSIT_FORMAT256_SIZE], sb[POSIT_FORMAT256_SIZE], sc[POSIT_FORMAT256_SIZE], sref[POSIT_FORMAT256_SIZE];
				posit_format256(pa, sa);
				posit_format256(pb, sb);
				posit_format256(pc, sc);
				posit_format256(pref, sref);
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit_assign256(a, 0, 0, 0);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit_assign256(b, 0, 0, 0);
			pc = posit_sub256(pa, pb);

			long double da, db, dref;
			da = posit_value256(pa);
			db = posit_value256(pb);
			dref = da - db;

			posit256_t pref = posit_assign256f(dref);
			if (posit_equal256(pref, pc)) {
				char sa[POSIT_FORMAT256_SIZE], sb[POSIT_FORMAT256_SIZE], sc[POSIT_FORMAT256_SIZE], sref[POSIT_FORMAT256_SIZE];
				posit_format256(pa, sa);
				posit_format256(pb, sb);
				posit_format256(pc, sc);
				posit_format256(pref, sref);
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit_assign256(a, 0, 0, 0);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit_assign256(b, 0, 0, 0);
			pc = posit_mul256(pa, pb);

			long double da, db, dref;
			da = posit_value256(pa);
			db = posit_value256(pb);
			dref = da * db;

			posit256_t pref = posit_assign256f(dref);
			if (posit_equal256(pref, pc)) {
				char sa[POSIT_FORMAT256_SIZE], sb[POSIT_FORMAT256_SIZE], sc[POSIT_FORMAT256_SIZE], sref[POSIT_FORMAT256_SIZE];
				posit_format256(pa, sa);
				posit_format256(pb, sb);
				posit_format256(pc, sc);
				posit_format256(pref, sref);
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
	for (int a = 0; a < maxNr; ++a) {
		pa = posit_assign256(a, 0, 0, 0);
		for (int b = 0; b < maxNr; ++b) {
			pb = posit_assign256(b, 0, 0, 0);
			pc = posit_div256(pa, pb);

			long double da, db, dref;
			da = posit_value256(pa);
			db = posit_value256(pb);
			dref = da / db;

			posit256_t pref = posit_assign256f(dref);
			if (posit_equal256(pref, pc)) {
				char sa[POSIT_FORMAT256_SIZE], sb[POSIT_FORMAT256_SIZE], sc[POSIT_FORMAT256_SIZE], sref[POSIT_FORMAT256_SIZE];
				posit_format256(pa, sa);
				posit_format256(pb, sb);
				posit_format256(pc, sc);
				posit_format256(pref, sref);
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
