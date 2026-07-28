// experiment.c: playground for manual experimenting with behavior of the C API
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(_MSC_VER)
#define POSIT_NO_GENERICS // MSVC doesn't support _Generic so disable the generic C11 macros
#endif
#include <universal/number/posit1/posit_c_api.h>

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

/*
posit[2] posit_add_exact(posit a, posit b):
the arguments are 2 posits a and b of the same parameters
the return value is a pair of posits, the first one is the nearest value to the actual sum and the second result is the difference between the first value and the exact result.
if the exponents of a and b are such that there is no bit-overlap in the mantissas, this function returns max(a,b), min(a,b)
posit[2] posit_sub_exact(posit a, posit b): same as add_exact with b negated
posit<nbits*2,es+1> posit_mul_promote(posit a, posit b):
the arguments are 2 posits a and b of the same parameters
the result is a posit with nbits twice that of the arguments and an es one more than that of the arguments
this function is equivalent to converting a and b to the larger size and then multiplying.
this function should never round (if it does then I've made a mistake)
posit<nbits*2,es+1> posit_div_promote(posit a, posit b):
Result should be the same as posit_div( posit<nbits2,es+1>(a), posit<nbits2,es+1>(b) )
posit posit_frexp(posit a, int* exp_out):
Defined in https://en.cppreference.com/w/cpp/numeric/math/frexp
posit posit_ldexp(posit a, int exp):
Defined here https://en.cppreference.com/w/cpp/numeric/math/ldexp
*/

	return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
