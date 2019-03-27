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
	pc = padd128(pa, pb);
	pformat128(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = psub128(pa, pb);
	pformat128(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = pmul128(pa, pb);
	pformat128(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR128;
	pb = ZERO128;
	pc = pdiv128(pa, pb);
	pformat128(pc, str);
	printf("posit value = %s\n", str);

	if (sizeof(long double) != 16) {
		printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<<128,4>>\n", sizeof(long double));
	}
	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = passign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = passign128(b, 0);
			pc = padd128(pa, pb);

			long double da, db, dref;
			da = pvalue128(pa);
			db = pvalue128(pb);
			dref = da + db;

			posit128_t pref = passign128f(dref);
			if (pequal128(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				pformat128(pa, sa);
				pformat128(pb, sb);
				pformat128(pc, sc);
				pformat128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s + %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (sizeof(long double) != 16) {
			printf("addition        uncertain\n");
		}
		else {
			printf("addition        FAIL\n");
		}
		failures = true;
	}
	else {
		printf("addition        PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = passign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = passign128(b, 0);
			pc = psub128(pa, pb);

			long double da, db, dref;
			da = pvalue128(pa);
			db = pvalue128(pb);
			dref = da - db;

			posit128_t pref = passign128f(dref);
			if (pequal128(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				pformat128(pa, sa);
				pformat128(pb, sb);
				pformat128(pc, sc);
				pformat128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s - %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (sizeof(long double) != 16) {
			printf("subtraction     uncertain\n");
		}
		else {
			printf("subtraction     FAIL\n");
		}
		failures = true;
	}
	else {
		printf("subtraction     PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = passign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = passign128(b, 0);
			pc = pmul128(pa, pb);

			long double da, db, dref;
			da = pvalue128(pa);
			db = pvalue128(pb);
			dref = da * db;

			posit128_t pref = passign128f(dref);
			if (pequal128(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				pformat128(pa, sa);
				pformat128(pb, sb);
				pformat128(pc, sc);
				pformat128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s * %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (sizeof(long double) != 16) {
			printf("multiplication  uncertain\n");
		}
		else {
			printf("multiplication  FAIL\n");
		}
		failures = true;
	}
	else {
		printf("multiplication  PASS\n");
	}

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = passign128(a, 0);
		for (int b = 0; b < 256; ++b) {
			pb = passign128(b, 0);
			pc = pdiv128(pa, pb);

			long double da, db, dref;
			da = pvalue128(pa);
			db = pvalue128(pb);
			dref = da / db;

			posit128_t pref = passign128f(dref);
			if (pequal128(pref, pc)) {
				char sa[40], sb[40], sc[40], sref[40];
				pformat128(pa, sa);
				pformat128(pb, sb);
				pformat128(pc, sc);
				pformat128(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s / %s produced %s instead of %s\n", sa, sb, sc, sref);
				++fails;
			}
		}
	}
	if (fails) {
		if (sizeof(long double) != 16) {
			printf("division        uncertain\n");
		}
		else {
			printf("division        FAIL\n");
		}
		failures = true;
	}
	else {
		printf("division        PASS\n");
	}

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
