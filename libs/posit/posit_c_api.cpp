// posit_api.cpp: implementation of the posit API for C programs

#include "posit_c_api.h"
#define POSIT_FAST_SPECIALIZATION
#include <posit>

posit8_t posit_add8(posit8_t a, posit8_t b) {
	using namespace sw::unum;

	posit<8,0> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a + b;
	return (posit8_t)presult.encoding();
}
