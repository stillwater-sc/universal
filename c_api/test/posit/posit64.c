#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "../c_api/posit/posit_c_api.h"

int main(int argc, char* argv[]) 
{
	posit64_t pa, pb, pc;
	char str[14];
	bool failures = false;
	bool bReportIndividualTestCases = false;

	// special case values
	pa = NAR64;
	pb = 0;
	pc = posit_add64(pa, pb);
	posit_format64(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR64;
	pb = 0;
	pc = posit_sub64(pa, pb);
	posit_format64(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR64;
	pb = 0;
	pc = posit_mul64(pa, pb);
	posit_format64(pc, str);
	printf("posit value = %s\n", str);

	pa = NAR64;
	pb = 0;
	pc = posit_div64(pa, pb);
	posit_format64(pc, str);
	printf("posit value = %s\n", str);

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_bit_assign64(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign64(b);
			pc = posit_add64(pa, pb);

			long double da, db, dref;
			da = posit_value64(pa);
			db = posit_value64(pb);
			dref = da + db;

			posit64_t pref = posit_float_assign64(dref);
			if (pref != pc) {
				char sa[32], sb[32], sc[32], sref[32];
				posit_format64(pa, sa);
				posit_format64(pb, sb);
				posit_format64(pc, sc);
				posit_format64(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s + %s produced %s instead of %s\n", sa, sb, sc, sref);
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
		pa = posit_bit_assign64(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign64(b);
			pc = posit_sub64(pa, pb);

			long double da, db, dref;
			da = posit_value64(pa);
			db = posit_value64(pb);
			dref = da - db;

			posit64_t pref = posit_float_assign64(dref);
			if (pref != pc) {
				char sa[32], sb[32], sc[32], sref[32];
				posit_format64(pa, sa);
				posit_format64(pb, sb);
				posit_format64(pc, sc);
				posit_format64(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s - %s produced %s instead of %s\n", sa, sb, sc, sref);
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
		pa = posit_bit_assign64(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign64(b);
			pc = posit_mul64(pa, pb);

			long double da, db, dref;
			da = posit_value64(pa);
			db = posit_value64(pb);
			dref = da * db;

			posit64_t pref = posit_float_assign64(dref);
			if (pref != pc) {
				char sa[32], sb[32], sc[32], sref[32];
				posit_format64(pa, sa);
				posit_format64(pb, sb);
				posit_format64(pc, sc);
				posit_format64(pref, sref);
				if (bReportIndividualTestCases) printf("FAIL: %s * %s produced %s instead of %s\n", sa, sb, sc, sref);
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

	if (sizeof(long double) != 16) {
		printf("Sizeof (long double) is %zu, which isn't sufficiently precise to validate posit<64,3>\n", sizeof(long double));
	}
	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_bit_assign64(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign64(b);
			pc = posit_div64(pa, pb);

			long double da, db, dref;
			da = posit_value64(pa);
			db = posit_value64(pb);
			dref = da / db;

			posit64_t pref = posit_float_assign64(dref);
			if (pref != pc) {
				char sa[32], sb[32], sc[32], sref[32];
				posit_format64(pa, sa);
				posit_format64(pb, sb);
				posit_format64(pc, sc);
				posit_format64(pref, sref);
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
