#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../c_api/posit/posit_c_api.h"

int main(int argc, char* argv[]) 
{
	posit32_t pa, pb, pc;
	char str[14];
	bool failures = false;

	// special case values
	pa = NAR32;
	pb = 0;
	pc = posit_add32(pa, pb);
	posit_format32(pc, str);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", pc);

	pa = NAR32;
	pb = 0;
	pc = posit_sub32(pa, pb);
	posit_format32(pc, str);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", pc);

	pa = NAR32;
	pb = 0;
	pc = posit_mul32(pa, pb);
	posit_format32(pc, str);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", pc);

	pa = NAR32;
	pb = 0;
	pc = posit_div32(pa, pb);
	posit_format32(pc, str);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", pc);

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_bit_assign32(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign32(b);
			pc = posit_add32(pa, pb);

			double da, db, dref;
			da = posit_value32(pa);
			db = posit_value32(pb);
			dref = da + db;

			posit32_t pref = posit_float_assign32(dref);
			if (pref != pc) {
				printf("FAIL: 32.2x%08xp + 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n", pa, pb, pc, pref);
				++fails;
				break;
			}
		}
		if (fails) break;
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
		pa = posit_bit_assign32(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign32(b);
			pc = posit_sub32(pa, pb);

			double da, db, dref;
			da = posit_value32(pa);
			db = posit_value32(pb);
			dref = da - db;

			posit32_t pref = posit_float_assign32(dref);
			if (pref != pc) {
				printf("FAIL: 32.2x%08xp + 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n", pa, pb, pc, pref);
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
		pa = posit_bit_assign32(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign32(b);
			pc = posit_mul32(pa, pb);

			double da, db, dref;
			da = posit_value32(pa);
			db = posit_value32(pb);
			dref = da * db;

			posit32_t pref = posit_float_assign32(dref);
			if (pref != pc) {
				printf("FAIL: 32.2x%08xp + 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n", pa, pb, pc, pref);
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

	// partial state space
	fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_bit_assign32(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign32(b);
			pc = posit_div32(pa, pb);

			double da, db, dref;
			da = posit_value32(pa);
			db = posit_value32(pb);
			dref = da / db;

			posit32_t pref = posit_float_assign32(dref);
			if (pref != pc) {
				printf("FAIL: 32.2x%08xp + 32.2x%08xp produced 32.2x%08xp instead of 32.2x%08xp\n", pa, pb, pc, pref);
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
