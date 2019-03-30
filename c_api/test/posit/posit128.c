// posit128.c: example test of the posit API for C programs using 128-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>

int main(int argc, char* argv[])
{
	posit128_t pa, pb, pc;
	char str[posit128_str_SIZE];
	bool failures = false;
	bool bReportIndividualTestCases = false;

	// special case values
	pa = NAR128;
	pb = ZERO128;
	pc = posit_add(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit_sub(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit_mul(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = posit_div(pa, pb);
	posit_str(str, pc);
	printf("posit value = %s\n", str);

	bool noReference = true;
	printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<<128,4>>\n", sizeof(long double));

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < 256; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit_add(pa, pb);


			long double da, db, dref;
			da = posit128_told(pa);
			db = posit128_told(pb);
			dref = da + db;

			posit128_t pref = posit128(dref);
			if (posit_cmp(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				posit_str(sa, pa);
				posit_str(sb, pb);
				posit_str(sc, pc);
				posit_str(sref, pref);

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

		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < 256; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit_sub(pa, pb);


			long double da, db, dref;
			da = posit_told(pa);
			db = posit_told(pb);
			dref = da - db;


			posit128_t pref = posit128(dref);
			if (posit_cmp(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				posit_str(sa, pa);
				posit_str(sb, pb);
				posit_str(sc, pc);
				posit_str(sref, pref);

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

		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < 256; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit_mul(pa, pb);


			long double da, db, dref;
			da = posit_told(pa);
			db = posit_told(pb);
			dref = da * db;


			posit128_t pref = posit128(dref);
			if (posit_cmp(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				posit_str(sa, pa);
				posit_str(sb, pb);
				posit_str(sc, pc);
				posit_str(sref, pref);

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

		pa = posit128_reinterpret( (uint64_t[]){ a, 0 } );
		for (int b = 0; b < 256; ++b) {
			pb = posit128_reinterpret( (uint64_t[]){ b, 0 } );
			pc = posit_div(pa, pb);

			long double da, db, dref;
			da = posit_told(pa);
			db = posit_told(pb);
			dref = da / db;

			posit128_t pref = posit128(dref);
			if (posit_cmp(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				posit_str(sa, pa);
				posit_str(sb, pb);
				posit_str(sc, pc);
				posit_str(sref, pref);

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
