// posit128.c: example test of the posit API for C programs using 128-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#include <universal/number/posit/posit_c_api.h>

int main(int argc, char* argv[])
{
	const int maxNr = 128;
	posit128_t pa, pb, pc;
	char str[posit128_str_SIZE];
	bool failures = false;
	bool bReportIndividualTestCases = false;

	// special case values
	pa = NAR128;
	pb = ZERO128;
	pc = posit128_add(pa, pb);
	posit128_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit128_sub(pa, pb);
	posit128_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit128_mul(pa, pb);
	posit128_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit128_div(pa, pb);
	posit128_str(str, pc);
	printf("posit value = %s\n", str);

	bool noReference = true;
	printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<<128,4>>\n", sizeof(long double));

	// partial state space
	int fails = 0;
	for (int a = 0; a < maxNr; ++a) {
		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < maxNr; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit128_add(pa, pb);

			long double da, db, dref;
			da = posit128_told(pa);
			db = posit128_told(pb);
			dref = da + db;

			posit128_t pref = posit128_fromld(dref);
			if (posit128_cmp(pref, pc)) {
				char sa[posit128_str_SIZE], sb[posit128_str_SIZE], sc[posit128_str_SIZE], sref[posit128_str_SIZE];
				posit128_str(sa, pa);
				posit128_str(sb, pb);
				posit128_str(sc, pc);
				posit128_str(sref, pref);

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
		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < maxNr; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit128_sub(pa, pb);

			long double da, db, dref;
			da = posit128_told(pa);
			db = posit128_told(pb);
			dref = da - db;

			posit128_t pref = posit128_fromld(dref);
			if (posit128_cmp(pref, pc)) {
				char sa[posit128_str_SIZE], sb[posit128_str_SIZE], sc[posit128_str_SIZE], sref[posit128_str_SIZE];
				posit128_str(sa, pa);
				posit128_str(sb, pb);
				posit128_str(sc, pc);
				posit128_str(sref, pref);

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
		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < maxNr; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit128_mul(pa, pb);

			long double da, db, dref;
			da = posit128_told(pa);
			db = posit128_told(pb);
			dref = da * db;

			posit128_t pref = posit128_fromld(dref);
			if (posit128_cmp(pref, pc)) {
				char sa[posit128_str_SIZE], sb[posit128_str_SIZE], sc[posit128_str_SIZE], sref[posit128_str_SIZE];
				posit128_str(sa, pa);
				posit128_str(sb, pb);
				posit128_str(sc, pc);
				posit128_str(sref, pref);

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
		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < maxNr; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit128_div(pa, pb);

			long double da, db, dref;
			da = posit128_told(pa);
			db = posit128_told(pb);
			dref = da / db;

			posit128_t pref = posit128_fromld(dref);
			if (posit128_cmp(pref, pc)) {
				char sa[posit128_str_SIZE], sb[posit128_str_SIZE], sc[posit128_str_SIZE], sref[posit128_str_SIZE];
				posit128_str(sa, pa);
				posit128_str(sb, pb);
				posit128_str(sc, pc);
				posit128_str(sref, pref);

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
