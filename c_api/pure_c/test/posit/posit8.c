// posit8.c: example test of the posit API for C programs using 8-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <math.h>	// required to provide explicit sqrt/exp/log declarations

#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so we'll leave it out from these tests
#include <universal/number/posit/posit_c_api.h>

void SpecialCases(void) {
	char str[posit8_str_SIZE];
	posit8_t pa, pb, pc;

	// special case values
	pa = NAR8;
	pb = ZERO8;
	pc = posit8_addp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 + 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit8_subp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 - 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit8_mulp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 * 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = NAR8;
	pb = ZERO8;
	pc = posit8_divp8(pa, pb);
	posit8_str(str, pc);
	printf("NAR8 / 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));

	pa = posit8_fromsi(1);
	pb = ZERO8;
	pc = posit8_divp8(pa, pb);
	posit8_str(str, pc);
	printf("1.0  / 0 = %s (8.0x%02xp)\n", str, posit8_bits(pc));
}

int main(int argc, char* argv[])
{
	posit8_t pa, pb, pc;
	int NR_POSITS = 256;
	bool failures = false;
	bool bReportIndividualTestFailure = true;

	printf("Special cases\n");
	SpecialCases();

	// conversion tests
	printf("\nConversion tests\n");
	for (int a = 0; a < NR_POSITS; ++a) {
		pa = posit8_reinterpret(a);
		float fa = posit8_tof(pa);
		pb = posit8_fromf(fa);
		if (posit8_cmpp8(pa, pb)) {
			printf("FAIL: 8.0x%02x != 8.0x%02x\n", pa.v, pb.v);
		}
	}

	// full state space
	int fails = 0;
	for (int a = 0; a < NR_POSITS; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < NR_POSITS; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_addp8(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da + db;
			posit8_t pref = posit8_fromf(dref);
			//printf("dref = %f  pref = 0x%2x\n", dref, pref.v);
			if (posit8_cmpp8(pref, pc)) {
				if (bReportIndividualTestFailure)
				printf("FAIL: 8.0x%02xp + 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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
	for (int a = 0; a < NR_POSITS; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < NR_POSITS; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_subp8(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da - db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmpp8(pref, pc)) {
				if (bReportIndividualTestFailure)
					printf("FAIL: 8.0x%02xp - 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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
	for (int a = 0; a < NR_POSITS; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < NR_POSITS; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_mulp8(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da * db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmp(pref, pc)) {
				if (bReportIndividualTestFailure)
					printf("FAIL: 8.0x%02xp * 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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
	for (int a = 0; a < NR_POSITS; ++a) {
		pa = posit8_reinterpret(a);
		for (int b = 0; b < NR_POSITS; ++b) {
			pb = posit8_reinterpret(b);
			pc = posit8_divp8(pa, pb);
			float da, db, dref;
			da = posit8_tof(pa);
			db = posit8_tof(pb);
			dref = da / db;
			posit8_t pref = posit8_fromf(dref);
			if (posit8_cmp(pref, pc)) {
				if (bReportIndividualTestFailure)
					printf("FAIL: 8.0x%02xp / 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n",
                    posit8_bits(pa), posit8_bits(pb), posit8_bits(pc), posit8_bits(pref));
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
	for (int a = 0; a < NR_POSITS; ++a) {   // includes negative numbers
		pa = posit8_reinterpret(a);
		pc = posit8_sqrt(pa);
		double da, dref;
		da = posit8_tod(pa);
		dref = sqrt(da);
		posit8_t pref = posit8_fromd(dref);
		if (posit8_cmp(pref, pc)) {
			if (bReportIndividualTestFailure)
				printf("FAIL: sqrt(8.0x%02xp) produced 8.0x%02xp instead of 8.0x%02xp\n",
						 posit8_bits(pa), posit8_bits(pc), posit8_bits(pref));
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
	for (int a = 0; a < 256; ++a) {   // includes negative numbers
		pa = posit8_reinterpret(a);
		pc = posit8_exp(pa);
		double da, dref;
		da = posit8_tod(pa);
		dref = exp(da);
		posit8_t pref = posit8_fromd(dref);
		if (posit8_cmp(pref, pc)) {
			if (bReportIndividualTestFailure)
				printf("FAIL: exp(8.0x%02xp) produced 8.0x%02xp instead of 8.0x%02xp\n",
				posit8_bits(pa), posit8_bits(pc), posit8_bits(pref));
			++fails;
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
	for (int a = 0; a < 128; ++a) {   // excludes negative numbers
		pa = posit8_reinterpret(a);
		pc = posit8_log(pa);
		double da, dref;
		da = posit8_tod(pa);
		dref = log(da);
		posit8_t pref = posit8_fromd(dref);
		if (posit8_cmp(pref, pc)) {
			if (bReportIndividualTestFailure)
				printf("FAIL: log(8.0x%02xp) produced 8.0x%02xp instead of 8.0x%02xp\n",
				posit8_bits(pa), posit8_bits(pc), posit8_bits(pref));
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

	return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
