// posit64.c: example test of the posit API for C programs using 64-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#include <universal/number/posit/posit_c_api.h>

int main(int argc, char* argv[])
{
	posit64_t pa, pb, pc;
	char str[posit64_str_SIZE];
	bool failures = false;
	bool bReportIndividualTestCases = false;

	// special case tolds
	pa = NAR64;
	pb = ZERO64;
	pc = posit64_add(pa, pb);
	posit64_str(str, pc);
	printf("posit told = %s\n", str);

	pa = NAR64;
	pb = ZERO64;
	pc = posit64_sub(pa, pb);
	posit64_str(str, pc);
	printf("posit told = %s\n", str);

	pa = NAR64;
	pb = ZERO64;
	pc = posit64_mul(pa, pb);
	posit64_str(str, pc);
	printf("posit told = %s\n", str);

	pa = NAR64;
	pb = ZERO64;
	pc = posit64_div(pa, pb);
	posit64_str(str, pc);
	printf("posit told = %s\n", str);

	bool noReference = true;
	printf("sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<64,3>\n", sizeof(long double));

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit64_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit64_reinterpret(b);
			pc = posit64_add(pa, pb);
			long double da, db, dref;
			da = posit64_told(pa);
			db = posit64_told(pb);
			dref = da + db;
			posit64_t pref = posit64_fromd(dref);
			if (posit64_cmp(pref, pc)) {
				char sa[posit64_str_SIZE], sb[posit64_str_SIZE], sc[posit64_str_SIZE], sref[posit64_str_SIZE];
				posit64_str(sa, pa);
				posit64_str(sb, pb);
				posit64_str(sc, pc);
				posit64_str(sref, pref);
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
		pa = posit64_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit64_reinterpret(b);
			pc = posit64_sub(pa, pb);
			long double da, db, dref;
			da = posit64_told(pa);
			db = posit64_told(pb);
			dref = da - db;
			posit64_t pref = posit64_fromd(dref);
			if (posit64_cmp(pref, pc)) {
				char sa[posit64_str_SIZE], sb[posit64_str_SIZE], sc[posit64_str_SIZE], sref[posit64_str_SIZE];
				posit64_str(sa, pa);
				posit64_str(sb, pb);
				posit64_str(sc, pc);
				posit64_str(sref, pref);
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
		pa = posit64_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit64_reinterpret(b);
			pc = posit64_mul(pa, pb);
			long double da, db, dref;
			da = posit64_told(pa);
			db = posit64_told(pb);
			dref = da * db;
			posit64_t pref = posit64_fromd(dref);
			if (posit64_cmp(pref, pc)) {
				char sa[posit64_str_SIZE], sb[posit64_str_SIZE], sc[posit64_str_SIZE], sref[posit64_str_SIZE];
				posit64_str(sa, pa);
				posit64_str(sb, pb);
				posit64_str(sc, pc);
				posit64_str(sref, pref);
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

	if (sizeof(long double) != 16) {
		printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<64,3>\n", sizeof(long double));
	}
	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit64_reinterpret(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit64_reinterpret(b);
			pc = posit64_div(pa, pb);
			long double da, db, dref;
			da = posit64_told(pa);
			db = posit64_told(pb);
			dref = da / db;
			posit64_t pref = posit64_fromd(dref);
			if (posit64_cmp(pref, pc)) {
				char sa[posit64_str_SIZE], sb[posit64_str_SIZE], sc[posit64_str_SIZE], sref[posit64_str_SIZE];
				posit64_str(sa, pa);
				posit64_str(sb, pb);
				posit64_str(sc, pc);
				posit64_str(sref, pref);
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
