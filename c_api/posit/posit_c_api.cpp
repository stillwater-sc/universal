// posit_c_api.cpp: implementation of the posit API for C programs
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>
#define POSIT_FAST_POSIT_8_0  1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#define POSIT_FAST_POSIT_64_3 0
#include <posit>

///////////////////////////////////////////////////////////////
/// conversions

// marshal128 takes a posit128_t and marshals it into a raw bitblock
void marshal128(posit128_t a, sw::unum::bitblock<128>& raw) {
	// 16 bytes
	uint32_t bit_cntr = 0;
	for (int c = 0; c < 16; ++c) {
		unsigned char byte = a.x[c];
		unsigned char mask = (unsigned char)(1);
		for (int b = 0; b < 8; ++b) {
			raw[bit_cntr++] = mask & byte;
			mask <<= 1;
		}
	}
}

// marshal256 takes a posit256_t and marshals it into a raw bitblock
void marshal256(posit256_t a, sw::unum::bitblock<256>& raw) {
	// 32 bytes
	uint32_t bit_cntr = 0;
	for (int c = 0; c < 32; ++c) {
		unsigned char byte = a.x[c];
		unsigned char mask = (unsigned char)(1);
		for (int b = 0; b < 8; ++b) {
			raw[bit_cntr++] = mask & byte;
			mask <<= 1;
		}
	}
}

// unmarshal128 takes a raw bitblock and unmarshals it into a posit128_t
void unmarshal128(sw::unum::bitblock<128>& raw, posit128_t& a) {
	// 16 bytes
	uint32_t bit_cntr = 0;
	for (int c = 0; c < 16; ++c) {
		unsigned char byte = 0;
		unsigned char mask = (unsigned char)(1);
		for (int b = 0; b < 8; ++b) {
			if (raw[bit_cntr++]) {
				byte |= mask;
			}
			mask <<= 1;
		}
		a.x[c] = byte;
	}
}

// unmarshal256 takes a raw bitblock and unmarshals it into a posit256_t
void unmarshal256(sw::unum::bitblock<256>& raw, posit256_t& a) {
	// 32 bytes
	uint32_t bit_cntr = 0;
	for (int c = 0; c < 32; ++c) {
		unsigned char byte = 0;
		unsigned char mask = (unsigned char)(1);
		for (int b = 0; b < 8; ++b) {
			if (raw[bit_cntr++]) {
				byte |= mask;
			}
			mask <<= 1;
		}
		a.x[c] = byte;
	}
}

///////////////////////////////////////////////////////////////
/////////        output

// report posit format for posit8_t. str must be at least 8 characters in size
void posit_format8(posit8_t a, char* str) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	std::string s = posit_format(pa);
	sprintf(str, "%s", s.c_str());
}

// report posit format for posit16_t. str must be at least 10 characters in size
void posit_format16(posit16_t a, char* str) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	std::string s = posit_format(pa);
	sprintf(str, "%s", s.c_str());
}

// report posit format for posit32_t. str must be at least 14 characters in size
void posit_format32(posit32_t a, char* str) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	std::string s = posit_format(pa);
	sprintf(str, "%s", s.c_str());
}

// report posit format for posit64_t. str must be at least 22 characters in size
void posit_format64(posit64_t a, char* str) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	std::string s = posit_format(pa);
	sprintf(str, "%s", s.c_str());
}

// report posit format for posit128_t. str must be at least 40 characters in size
void posit_format128(posit128_t a, char* str) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	std::string s = posit_format(pa);
	sprintf(str, "%s", s.c_str());
}

// report posit format for posit256_t. str must be at least 72 characters in size
void posit_format256(posit256_t a, char* str) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	std::string s = posit_format(pa);
	sprintf(str, "%s", s.c_str());
}

///////////////////////////////////////////////////////////////
/////////        casts to double

double posit_value8(posit8_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	return (double)pa;
}

double posit_value16(posit16_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	return (double)pa;
}

double posit_value32(posit32_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	return (double)pa;
}

long double posit_value64(posit64_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa;
	pa.set_raw_bits(a);
	return (long double)pa;
}

long double posit_value128(posit128_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	return (long double)pa;
}

long double posit_value256(posit256_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	return (long double)pa;
}

///////////////////////////////////////////////////////////////
/////////        bit assignment

// helper to make it easier to create 128bit numbers
posit128_t posit_assign128(unsigned long long lower, unsigned long long upper = 0) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa;
	struct {
		unsigned long long lower;
		unsigned long long upper;
	} p128_mem;
	p128_mem.upper = upper;
	p128_mem.lower = lower;
	bitblock<nbits> raw;
	marshal128((posit128_t&)p128_mem, raw);
	pa.set(raw);
	posit128_t result;
	bitblock<nbits> x = pa.get();
	unmarshal128(x, result);
	return result;
}

// helper to make it easier to create 256bit numbers
posit256_t posit_assign256(unsigned long long lower0, unsigned long long lower1 = 0, unsigned long long lower2 = 0, unsigned long long lower3 = 0) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa;
	struct {
		unsigned long long lower0;
		unsigned long long lower1;
		unsigned long long lower2;
		unsigned long long lower3;
	} p256_mem;
	p256_mem.lower3 = lower3;
	p256_mem.lower2 = lower2;
	p256_mem.lower1 = lower1;
	p256_mem.lower0 = lower0;
	bitblock<nbits> raw;
	marshal256((posit256_t&)p256_mem, raw);
	pa.set(raw);
	posit256_t result;
	bitblock<nbits> x = pa.get();
	unmarshal256(x, result);
	return result;
}

///////////////////////////////////////////////////////////////
/////////        integer assignment

posit8_t posit_assign8i(int a) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa(a);
	return (posit8_t)pa.encoding();
}

posit16_t posit_assign16i(int a) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa(a);
	return (posit16_t)pa.encoding();
}

posit32_t posit_assign32i(long a) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa(a);
	return (posit32_t)pa.encoding();
}

posit64_t posit_assign64i(long long a) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa(a);
	return (posit64_t)pa.encoding();
}

posit128_t posit_assign128i(long long a) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa(a);
	posit128_t result;
	bitblock<nbits> x = pa.get();
	unmarshal128(x, result);
	return result;
}

posit256_t posit_assign256i(long long a) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa(a);
	posit256_t result;
	bitblock<nbits> x = pa.get();
	unmarshal256(x, result);
	return result;
}

///////////////////////////////////////////////////////////////
/////////        IEEE floating point assignment

posit8_t posit_assign8f(float a) {
	using namespace sw::unum;
	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	posit<nbits, es> pa(a);
	return (posit8_t)pa.encoding();
}

posit16_t posit_assign16f(float a) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa(a);
	return (posit16_t)pa.encoding();
}

posit32_t posit_assign32f(double a) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa(a);
	return (posit32_t)pa.encoding();
}

posit64_t posit_assign64f(long double a) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa(a);
	return (posit64_t)pa.encoding();
}

posit128_t posit_assign128f(long double a) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa(a);
	posit128_t result;
	bitblock<nbits> x = pa.get();
	unmarshal128(x, result);
	return result;
}

posit256_t posit_assign256f(long double a) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa(a);
	posit256_t result;
	bitblock<nbits> x = pa.get();
	unmarshal256(x, result);
	return result;
}

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
	presult = pa + pb;
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
	presult = pa + pb;
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
	presult = pa + pb;
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
	presult = pa + pb;
	return (posit64_t)presult.encoding();
}

// posit<128,4> addition
posit128_t posit_add128(posit128_t a, posit128_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	marshal128(b, raw);
	pb.set(raw);
	presult = pa + pb;
	// marshall it back to a posit128_t
	posit128_t result;
	bitblock<nbits> x = presult.get();
	unmarshal128(x, result);
	return result;
}

// posit<256,5> addition
posit256_t posit_add256(posit256_t a, posit256_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	marshal256(b, raw);
	pb.set(raw);
	presult = pa + pb;
	// marshall it back to a posit256_t
	posit256_t result;
	bitblock<nbits> x = presult.get();
	unmarshal256(x, result);
	return result;
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
	presult = pa - pb;
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
	presult = pa - pb;
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
	presult = pa - pb;
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
	presult = pa - pb;
	return (posit64_t)presult.encoding();
}

// posit<128,4> subtraction
posit128_t posit_sub128(posit128_t a, posit128_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	marshal128(b, raw);
	pb.set(raw);
	presult = pa - pb;
	// marshall it back to a posit128_t
	posit128_t result;
	bitblock<nbits> x = presult.get();
	unmarshal128(x, result);
	return result;
}

// posit<256,5> subtraction
posit256_t posit_sub256(posit256_t a, posit256_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	marshal256(b, raw);
	pb.set(raw);
	presult = pa - pb;
	// marshall it back to a posit256_t
	posit256_t result;
	bitblock<nbits> x = presult.get();
	unmarshal256(x, result);
	return result;
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
	presult = pa * pb;
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
	presult = pa * pb;
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
	presult = pa * pb;
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
	presult = pa * pb;
	return (posit64_t)presult.encoding();
}

// posit<128,4> multiplication
posit128_t posit_mul128(posit128_t a, posit128_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	marshal128(b, raw);
	pb.set(raw);
	presult = pa * pb;
	// marshall it back to a posit128_t
	posit128_t result;
	bitblock<nbits> x = presult.get();
	unmarshal128(x, result);
	return result;
}

// posit<256,5> multiplication
posit256_t posit_mul256(posit256_t a, posit256_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	marshal256(b, raw);
	pb.set(raw);
	presult = pa * pb;
	// marshall it back to a posit256_t
	posit256_t result;
	bitblock<nbits> x = presult.get();
	unmarshal256(x, result);
	return result;
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
	presult = pa / pb;
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
	presult = pa / pb;
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
	presult = pa / pb;
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
	presult = pa / pb;
	return (posit64_t)presult.encoding();
}

// posit<128,4> division
posit128_t posit_div128(posit128_t a, posit128_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	marshal128(b, raw);
	pb.set(raw);
	presult = pa / pb;
	// marshall it back to a posit128_t
	posit128_t result;
	bitblock<nbits> x = presult.get();
	unmarshal128(x, result);
	return result;
}

// posit<256,5> division
posit256_t posit_div256(posit256_t a, posit256_t b) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa, pb, presult;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	marshal256(b, raw);
	pb.set(raw);
	presult = pa / pb;
	// marshall it back to a posit256_t
	posit256_t result;
	bitblock<nbits> x = presult.get();
	unmarshal256(x, result);
	return result;
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
	presult = sqrt(pa);
	return (posit8_t)presult.encoding();
}

// posit<16,1> sqrt
posit16_t posit_sqrt16(posit16_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	presult = sqrt(pa);
	return (posit16_t)presult.encoding();
}

// posit<32,2> sqrt
posit32_t posit_sqrt32(posit32_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pa, pb, presult;
	pa.set_raw_bits(a);
	presult = sqrt(pa);
	return (posit32_t)presult.encoding();
}

// posit<64,3> sqrt
posit64_t posit_sqrt64(posit64_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 64;
	constexpr size_t es = 3;
	posit<nbits, es> pa, presult;
	pa.set_raw_bits(a);
	presult = sqrt(pa);
	return (posit64_t)presult.encoding();
}

// posit<128,4> sqrt
posit128_t posit_sqrt128(posit128_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 128;
	constexpr size_t es = 4;
	posit<nbits, es> pa, presult;
	bitblock<nbits> raw;
	marshal128(a, raw);
	pa.set(raw);
	presult = sqrt(pa);
	raw = presult.get();
	unmarshal128(raw, a); // reusing the stack var
	return a;
}

// posit<256,5> sqrt
posit256_t posit_sqrt256(posit256_t a) {
	using namespace sw::unum;
	constexpr size_t nbits = 256;
	constexpr size_t es = 5;
	posit<nbits, es> pa, presult;
	bitblock<nbits> raw;
	marshal256(a, raw);
	pa.set(raw);
	presult = sqrt(pa);
	raw = presult.get();
	unmarshal256(raw, a); // reusing the stack var
	return a;
}

///////////////////////////////////////////////////////////////
/////////        logic operators

bool posit_equal8(posit8_t a, posit8_t b) {
	return a == b;
}

bool posit_equal16(posit16_t a, posit16_t b) {
	return a == b;
}

bool posit_equal32(posit32_t a, posit32_t b) {
	return a == b;
}
bool posit_equal64(posit64_t a, posit64_t b) {
	return a == b;
}

bool posit_equal128(posit128_t a, posit128_t b) {
	bool bEqual = true;
	for (int i = 0; i < 16; ++i) {
		if (a.x[i] != b.x[i]) {
			bEqual = false;
			break;
		}
	}
	return bEqual;
}

bool posit_equal256(posit256_t a, posit256_t b) {
	bool bEqual = true;
	for (int i = 0; i < 32; ++i) {
		if (a.x[i] != b.x[i]) {
			bEqual = false;
			break;
		}
	}
	return bEqual;
}

int posit_cmp8(posit8_t a, posit8_t b) {
	return a - b;
}

int posit_cmp16(posit16_t a, posit16_t b) {
	return a - b;
}

int posit_cmp32(posit32_t a, posit32_t b) {
	return a - b;
}

int posit_cmp64(posit64_t a, posit64_t b) {
	return int(a - b);
}

// TODO
int posit_cmp128(posit128_t a, posit128_t b) {
	int cmp = 0;

	return cmp;
}
// TODO
int posit_cmp256(posit256_t a, posit256_t b) {
	int cmp = 0;

	return cmp;
}
