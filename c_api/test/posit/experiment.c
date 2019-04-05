// experiment.c: playground for manual experimenting with behavior of the C API
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(_MSC_VER)
#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so disable the generic C11 macros
#endif
#include <posit_c_api.h>

int main(int argc, char* argv[])
{
	posit32_t pa, pb, pc;
	char str[posit32_str_SIZE];
	bool failures = false;

	// special case values
	pa = NAR32;
	pb = ZERO32;
	pc = posit32_add(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);
	printf("posit value = 32.2x%08xp\n", posit32_bits(pc));

	pa = NAR32;
	pb = ZERO32;
	pc = posit32_sub(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR32;
	pb = ZERO32;
	pc = posit32_mul(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);

	pa = NAR32;
	pb = ZERO32;
	pc = posit32_div(pa, pb);
	posit32_str(str, pc);
	printf("posit value = %s\n", str);

	// manual testing of special values
	printf(">>>>>>>>>>>>>>> Special values\n");
	{
		int refsi = -1;
		pa = posit32_fromsi(refsi);
		posit32_str(str, pa);
		printf("posit value = %s: real value = %15.9f\n", str, posit32_tod(pa));
	}

	{
		unsigned int refui = 1;
		pa = posit32_fromui(refui);
		posit32_str(str, pa);
		printf("posit value = %s: real value = %15.9f\n", str, posit32_tod(pa));
	}

	{
		float reff = -1.5;
		pa = posit32_fromf(reff);
		posit32_str(str, pa);
		printf("posit value = %s: real value = %15.9f\n", str, posit32_tod(pa));
	}

	{
		double refd = 1.5;
		pa = posit32_fromd(refd);
		posit32_str(str, pa);
		printf("posit value = %s: real value = %15.9f\n", str, posit32_tod(pa));
	}

	// partial state space
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa = posit32_reinterpret(a);
		double da, dref;
			da = posit32_tod(pa);
			posit32_t pref = posit32_fromd(da);
			if (posit32_cmp(pref, pa)) {
				printf("FAIL: 32.2x%08xp produced %8.5f which returned into 32.2x%08xp\n",
					posit32_bits(pa), da, posit32_bits(pref));
				++fails;
				break;
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

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
