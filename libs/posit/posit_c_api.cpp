// posit_api.cpp: implementation of the posit API for C programs

#include "posit_c_api.h"
#define POSIT_FAST_POSIT_8_0  1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#define POSIT_FAST_POSIT_64_3 0
#include <posit>

///////////////////////////////////////////////////////////////
/////////        ADDITION

// posit<8,0> addition
posit8_t posit_add8(posit8_t a, posit8_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits,es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a + b;
	return (posit8_t)presult.encoding();
}

// posit<16,1> addition
posit16_t posit_add16(posit16_t a, posit16_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a + b;
	return (posit16_t)presult.encoding();
}

// posit<32,2> addition
posit32_t posit_add32(posit32_t a, posit32_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a + b;
	return (posit32_t)presult.encoding();
}

// posit<64,3> addition
posit64_t posit_add64(posit64_t a, posit64_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a + b;
	return (posit64_t)presult.encoding();
}

///////////////////////////////////////////////////////////////
/////////        SUBTRACTION

// posit<8,0> subtraction
posit8_t posit_sub8(posit8_t a, posit8_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a - b;
	return (posit8_t)presult.encoding();
}

// posit<16,1> subtraction
posit16_t posit_sub16(posit16_t a, posit16_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a - b;
	return (posit16_t)presult.encoding();
}

// posit<32,2> subtraction
posit32_t posit_sub32(posit32_t a, posit32_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a - b;
	return (posit32_t)presult.encoding();
}

// posit<64,3> subtraction
posit64_t posit_sub64(posit64_t a, posit64_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a - b;
	return (posit64_t)presult.encoding();
}

///////////////////////////////////////////////////////////////
/////////        MULTIPLICATION

// posit<8,0> multiplication
posit8_t posit_mul8(posit8_t a, posit8_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a * b;
	return (posit8_t)presult.encoding();
}

// posit<16,1> multiplication
posit16_t posit_mul16(posit16_t a, posit16_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a * b;
	return (posit16_t)presult.encoding();
}

// posit<32,2> multiplication
posit32_t posit_mul32(posit32_t a, posit32_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a * b;
	return (posit32_t)presult.encoding();
}

// posit<64,3> multiplication
posit64_t posit_mul64(posit64_t a, posit64_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a * b;
	return (posit64_t)presult.encoding();
}

///////////////////////////////////////////////////////////////
/////////        DIVISION

// posit<8,0> division
posit8_t posit_div8(posit8_t a, posit8_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a / b;
	return (posit8_t)presult.encoding();
}

// posit<16,1> division
posit16_t posit_div16(posit16_t a, posit16_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a / b;
	return (posit16_t)presult.encoding();
}

// posit<32,2> division
posit32_t posit_div32(posit32_t a, posit32_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a / b;
	return (posit32_t)presult.encoding();
}

// posit<64,3> division
posit64_t posit_div64(posit64_t a, posit64_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	pb.set_raw_bits(b);
	presult = a / b;
	return (posit64_t)presult.encoding();
}

///////////////////////////////////////////////////////////////
/////////        SQUARE ROOT

// posit<8,0> sqrt
posit8_t posit_sqrt8(posit8_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	presult = sqrt(a);
	return (posit8_t)presult.encoding();
}

// posit<16,1> sqrt
posit16_t posit_add16(posit16_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	presult = sqrt(a);
	return (posit16_t)presult.encoding();
}

// posit<32,2> sqrt
posit32_t posit_add32(posit32_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	presult = sqrt(a);
	return (posit32_t)presult.encoding();
}

// posit<64,3> sqrt
posit64_t posit_sqrt64(posit64_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 4;
	posit<nbits, es> pa, presult;
	pa.set_raw_bits(a);
	presult = sqrt(a);
	return (posit64_t)presult.encoding();
}

