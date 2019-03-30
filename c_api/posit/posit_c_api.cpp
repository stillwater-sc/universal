// posit_c_api.cpp: implementation of the posit API for C programs
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <posit_c_api.h>
#define POSIT_FAST_POSIT_8_0   1
#define POSIT_FAST_POSIT_16_1  1 
#define POSIT_FAST_POSIT_32_2  1
// thefollowing posit configurations do not have a fast implementation yet
#define POSIT_FAST_POSIT_64_3  0
#define POSIT_FAST_POSIT_128_4 0
#define POSIT_FAST_POSIT_256_5 0
#include <posit>

// marshal takes a positN_t and marshals it into a raw bitblock
template<size_t nbits, size_t es, typename positN_t>
void marshal(positN_t a, sw::unum::bitblock<nbits>& raw) {
	int nrBytes = 0;
	switch (nbits) {
	case 8:
		nrBytes = 1;
		break;
	case 16:
		nrBytes = 2;
		break;
	case 32:
		nrBytes = 4;
		break;
	case 64:
		nrBytes = 8;
		break;
	case 128:
		nrBytes = 16;
		break;
	case 256:
		nrBytes = 32;
		break;
	default:
		nrBytes = 0;
	}
	uint32_t bit_cntr = 0;
	for (int c = 0; c < nrBytes; ++c) {
		unsigned char byte = a.x[c];
		unsigned char mask = (unsigned char)(1);
		for (int b = 0; b < 8; ++b) {
			raw[bit_cntr++] = mask & byte;
			mask <<= 1;
		}
	}
}

// unmarshal takes a raw bitblock and unmarshals it into a positN_t
template<size_t nbits, size_t es, typename positN_t>
void unmarshal(sw::unum::bitblock<nbits>& raw, positN_t& a) {
	int nrBytes = 0;
	switch (nbits) {
	case 8:
		nrBytes = 1;
		break;
	case 16:
		nrBytes = 2;
		break;
	case 32:
		nrBytes = 4;
		break;
	case 64:
		nrBytes = 8;
		break;
	case 128:
		nrBytes = 16;
		break;
	case 256:
		nrBytes = 32;
		break;
	default:
		nrBytes = 0;
	}
	uint32_t bit_cntr = 0;
	for (int c = 0; c < nrBytes; ++c) {
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

template<size_t nbits, size_t es, class positN_t> class convert {
	static sw::unum::posit<nbits, es> decode(positN_t bits);
	static positN_t encode(sw::unum::posit<nbits, es> p);
};
/*
template<size_t nbits, size_t es, class positN_t> class convert_small : convert<nbits,es,positN_t> {
	public:
	static sw::unum::posit<nbits, es> decode(positN_t bits) {
		sw::unum::posit<nbits, es> pa;
		pa.set_raw_bits((uint64_t) bits.v);
		return pa;
	}
	//error C2397: conversion from 'unsigned __int64' to 'uint32_t' requires a narrowing conversion
	//note: while compiling class template member function 'positN_t convert_small<32,2,positN_t>::encode(sw::unum::posit<32,2>)'
	static positN_t encode(sw::unum::posit<nbits, es> p) {
		return { p.encoding() };
	}
};
*/
template<size_t nbits, size_t es, class positN_t> class convert_bytes : convert<nbits,es,positN_t> {
	public:
	static sw::unum::posit<nbits, es> decode(positN_t bits) {
		sw::unum::posit<nbits, es> pa;
		sw::unum::bitblock<nbits> raw;
		marshal<nbits,es>(bits, raw);
		pa.set(raw);
		return pa;
	}
	static positN_t encode(sw::unum::posit<nbits, es> p) {
		positN_t out;
		sw::unum::bitblock<nbits> raw = p.get();
		unmarshal<nbits,es>(raw, out);
		return out;
	}
};

template<size_t nbits, size_t es> class operation {
	public:
	static sw::unum::posit<nbits, es> op(
		sw::unum::posit<nbits, es> a,
		sw::unum::posit<nbits, es> b
	);
};
#define OPERATION(name, impl) \
	template<size_t nbits, size_t es> class name: operation<nbits,es> { \
		public: static sw::unum::posit<nbits, es> \
			op(sw::unum::posit<nbits, es> a, sw::unum::posit<nbits, es> b) impl \
	}
OPERATION(op_add, { return a + b; });
OPERATION(op_sub, { return a - b; });
OPERATION(op_mul, { return a * b; });
OPERATION(op_div, { return a / b; });

template<size_t _nbits, size_t _es, class positN_t, class convert> class capi {
	public:
	static constexpr size_t nbits = _nbits;
	static constexpr size_t es = _es;
	static constexpr positN_t positN = positN_t();
	static constexpr convert conv = convert();

	static void format(positN_t p, char* str) {
		using namespace sw::unum;
		posit<nbits, es> pa = convert::decode(p);
		std::string s = posit_format(pa);
		sprintf(str, "%s", s.c_str());
	}

	template<class out>
	static out to(positN_t bits) {
		using namespace sw::unum;
		posit<nbits, es> pa = convert::decode(bits);
		return static_cast<out>(pa);
	}

	template<class in>
	static positN_t from(in a) {
		using namespace sw::unum;
		posit<nbits, es> pa(a);
		return convert::encode(pa);
	}

	template<class operation>
	static positN_t op(positN_t a, positN_t b) {
		using namespace sw::unum;
		posit<nbits, es> pa = convert::decode(a);
		posit<nbits, es> pb = convert::decode(b);
		posit<nbits, es> res = operation::op(pa, pb);
		return convert::encode(res);
	}

	template<class ocapi>
	static positN_t fromp(decltype(ocapi::positN) p) {
		using namespace sw::unum;
		posit<ocapi::nbits, ocapi::es> inp = decltype(ocapi::conv)::decode(p);
		// TODO: There must be a better way to do this
		double d = (double) inp;
		posit<nbits, es> outp(d);
		return convert::encode(outp);
	}

	static int cmp(positN_t a, positN_t b) {
		using namespace sw::unum;
		posit<nbits, es> pa = convert::decode(a);
		posit<nbits, es> pb = convert::decode(b);
		return (pa > pb) ? 1 : (pa < pb) ? -1 : 0;
	}
};

typedef capi<8,0,posit8_t,convert_bytes<8,0,posit8_t>> capi8;
typedef capi<16,1,posit16_t,convert_bytes<16,1,posit16_t>> capi16;
typedef capi<32,2,posit32_t,convert_bytes<32,2,posit32_t>> capi32;
typedef capi<64,3,posit64_t,convert_bytes<64,3,posit64_t>> capi64;
typedef capi<128,4,posit128_t,convert_bytes<128,4,posit128_t>> capi128;
typedef capi<256,5,posit256_t,convert_bytes<256,5,posit256_t>> capi256;

// prevent any symbol mangling
extern "C" {

#define POSIT_IMPLS

#define POSIT_NBITS 8
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 16
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 32
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 64
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 128
#include "posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 256
#include "posit_c_macros.h"
#undef POSIT_NBITS

}
