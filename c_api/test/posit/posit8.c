#include <stdlib.h>
#include <stdio.h>
#include "../c_api/posit/posit_c_api.h"

int main(int argc, char* argv[]) 
{
	posit8_t pa, pb, pc;
	char str[8];

	pa = NAR8;
	pb = 0;
	pc = posit_add8(pa, pb);
	posit_format8(pc, str);
	printf("posit value = %s\n", str);
	printf("posit value = 8.0x%02xp\n", pc);

	pa = 0;
	pb = 0x01;
	pc = posit_add8(pa, pb);
	posit_format8(pc, str);
	printf("posit value = %s\n", str);
	printf("posit value = 8.0x%02xp\n", pc);

	// full state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit_bit_assign8(a);
		for (int b = 0; b < 256; ++b) {
			pb = posit_bit_assign8(b);
			pc = posit_add8(pa, pb);

			double da, db, dref;
			da = posit_value8(pa);
			db = posit_value8(pb);
			dref = da + db;

			posit8_t pref = posit_float_assign8((float)dref);
			if (pref != pc) {
				printf("FAIL: 8.0x%02xp + 8.0x%02xp produced 8.0x%02xp instead of 8.0x%02xp\n", pa, pb, pc, pref);
				++fails;
			}
		}
	}


	return fails > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
