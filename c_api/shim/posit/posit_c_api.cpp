// posit_c_api.cpp: implementation of the posit API for C programs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <tuple>
#include <universal/number/posit/posit_c_api.h>

// configure the C++ library
// default behavior that is kept
// POSIT_ROUNDING_ERROR_FREE_IO_FORMAT
// enable literals in expressions
// POSIT_ENABLE_LITERALS
// Disable exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// Disable standard posit specializations for the shim
#define POSIT_FAST_POSIT_4_0   0
#define POSIT_FAST_POSIT_8_0   0
#define POSIT_FAST_POSIT_16_1  1
#define POSIT_FAST_POSIT_32_2  1
#define POSIT_FAST_POSIT_64_3  0
#define POSIT_FAST_POSIT_128_4 0
#define POSIT_FAST_POSIT_256_5 0
// Now include the C++ library
#include <universal/number/posit/posit.hpp>


// marshal takes a positN_t and marshals it into a raw bitblock
template<size_t nbits, size_t es, typename positN_t>
void marshal(positN_t a, sw::universal::bitblock<nbits>& raw) {
	int nrBytes = 0;
	int maxBitsInByte = 8; // default is multi-byte data structures
	switch (nbits) {
	case 4:
		maxBitsInByte = 4; // except for nbits < 8
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
		for (int b = 0; b < maxBitsInByte; ++b) {
			raw[bit_cntr++] = mask & byte;
			mask <<= 1;
		}
	}
}

// unmarshal takes a raw bitblock and unmarshals it into a positN_t
template<size_t nbits, size_t es, typename positN_t>
void unmarshal(sw::universal::bitblock<nbits>& raw, positN_t& a) {
	int nrBytes = 0;
	int maxBitsInByte = 8; // default is multi-byte data structures
	switch (nbits) {
	case 4:
		maxBitsInByte = 4; // except for nbits < 8
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
		for (int b = 0; b < maxBitsInByte; ++b) {
			if (raw[bit_cntr++]) {
				byte |= mask;
			}
			mask <<= 1;
		}
		a.x[c] = byte;
	}
}

template<size_t nbits, size_t es, class positN_t> class convert {
	static sw::universal::posit<nbits, es> decode(positN_t bits);
	static positN_t encode(sw::universal::posit<nbits, es> p);
};

template<size_t nbits, size_t es, class positN_t> class convert_bytes : convert<nbits,es,positN_t> {
	public:
	static sw::universal::posit<nbits, es> decode(positN_t bits) {
		sw::universal::posit<nbits, es> pa;
		sw::universal::bitblock<nbits> raw;
		marshal<nbits,es>(bits, raw);
		pa.setBitblock(raw);
		return pa;
	}
	static positN_t encode(sw::universal::posit<nbits, es> p) {
		positN_t out;
		sw::universal::bitblock<nbits> raw = p.get();
		unmarshal<nbits,es>(raw, out);
		return out;
	}
};

// operation<2,1> = 2 args, 1 result
template<size_t nbits, size_t es> class operation21 {
	public:
	static sw::universal::posit<nbits, es> op(
		sw::universal::posit<nbits, es> a,
		sw::universal::posit<nbits, es> b
	);
};
template<size_t nbits, size_t es> class operation22 {
	public:
	static std::tuple<sw::universal::posit<nbits, es>,sw::universal::posit<nbits, es>> op(
		sw::universal::posit<nbits, es> a,
		sw::universal::posit<nbits, es> b
	);
};
template<size_t nbits, size_t es> class operation11 {
	public:
	static sw::universal::posit<nbits, es> op(
		sw::universal::posit<nbits, es> a
	);
};
#define OPERATION21(name, ...) \
	template<size_t nbits, size_t es> class name: operation21<nbits,es> { \
		public: static sw::universal::posit<nbits, es> \
			op(sw::universal::posit<nbits, es> a, sw::universal::posit<nbits, es> b) __VA_ARGS__ \
	}
#define OPERATION22(name, ...) \
	template<size_t nbits, size_t es> class name: operation22<nbits,es> { \
		public: static std::tuple<sw::universal::posit<nbits, es>,sw::universal::posit<nbits, es>> \
			op(sw::universal::posit<nbits, es> a, sw::universal::posit<nbits, es> b) __VA_ARGS__ \
	}
#define OPERATION11(name, ...) \
	template<size_t nbits, size_t es> class name: operation11<nbits,es> { \
		public: static sw::universal::posit<nbits, es> \
			op(sw::universal::posit<nbits, es> a) __VA_ARGS__ \
	}
OPERATION21(op_add, { return a + b; });
OPERATION21(op_sub, { return a - b; });
OPERATION21(op_mul, { return a * b; });
OPERATION21(op_div, { return a / b; });
OPERATION11(op_sqrt, { return sw::universal::sqrt<nbits, es>(a); });
OPERATION11(op_exp, { return sw::universal::exp<nbits, es>(a); });
OPERATION11(op_log, { return sw::universal::log<nbits, es>(a); });
OPERATION22(op_add_exact, {
    // TODO
    //return a.add_exact(b);
    return std::make_tuple(sw::universal::posit<nbits, es>(0), sw::universal::posit<nbits, es>(0));
});
OPERATION22(op_sub_exact, {
    // TODO
    //return a.add_exact(b);
    return std::make_tuple(sw::universal::posit<nbits, es>(0), sw::universal::posit<nbits, es>(0));
});

template<size_t _nbits, size_t _es, class positN_t, class positNx2_t, class convert> class capi {
	public:
	static constexpr size_t nbits = _nbits;
	static constexpr size_t es = _es;
	static constexpr positN_t positN = positN_t();
	static constexpr convert conv = convert();

	static void format(positN_t p, char* str) {
		using namespace sw::universal;
		posit<nbits, es> pa = convert::decode(p);
		std::string s = hex_format(pa);
		sprintf(str, "%s", s.c_str());
	}

	template<class out>
	static out to(positN_t bits) {
		using namespace sw::universal;
		posit<nbits, es> pa = convert::decode(bits);
		return static_cast<out>(pa);
	}

	template<class in>
	static positN_t from(in a) {
		using namespace sw::universal;
		posit<nbits, es> pa(a);
		return convert::encode(pa);
	}

    template<class operation22>
	static positNx2_t op22(positN_t a, positN_t b) {
		using namespace sw::universal;
		posit<nbits, es> pa = convert::decode(a);
		posit<nbits, es> pb = convert::decode(b);
        posit<nbits, es> x;
        posit<nbits, es> y;
        std::tie(x, y) = operation22::op(pa, pb);
        positNx2_t out;
        out.x = convert::encode(x);
        out.y = convert::encode(y);
		return out;
	}

	template<class operation21>
	static positN_t op21(positN_t a, positN_t b) {
		using namespace sw::universal;
		posit<nbits, es> pa = convert::decode(a);
		posit<nbits, es> pb = convert::decode(b);
		posit<nbits, es> res = operation21::op(pa, pb);
		return convert::encode(res);
	}

	template<class operation11>
	static positN_t op11(positN_t a) {
		using namespace sw::universal;
		posit<nbits, es> pa = convert::decode(a);
		posit<nbits, es> res = operation11::op(pa);
		return convert::encode(res);
	}

	template<class ocapi>
	static positN_t fromp(decltype(ocapi::positN) p) {
		using namespace sw::universal;
		posit<ocapi::nbits, ocapi::es> inp = decltype(ocapi::conv)::decode(p);
		// TODO: There must be a better way to do this
		double d = (double) inp;
		posit<nbits, es> outp(d);
		return convert::encode(outp);
	}

	static int cmp(positN_t a, positN_t b) {
		using namespace sw::universal;
		posit<nbits, es> pa = convert::decode(a);
		posit<nbits, es> pb = convert::decode(b);
		return (pa > pb) ? 1 : (pa < pb) ? -1 : 0;
	}
};

typedef capi<4,0,posit4_t,posit4x2_t,convert_bytes<4,0,posit4_t>> capi4;
typedef capi<8,0,posit8_t,posit8x2_t,convert_bytes<8,0,posit8_t>> capi8;
typedef capi<16,1,posit16_t,posit16x2_t,convert_bytes<16,1,posit16_t>> capi16;
typedef capi<32,2,posit32_t,posit32x2_t,convert_bytes<32,2,posit32_t>> capi32;
typedef capi<64,3,posit64_t,posit64x2_t,convert_bytes<64,3,posit64_t>> capi64;
typedef capi<128,4,posit128_t,posit128x2_t,convert_bytes<128,4,posit128_t>> capi128;
typedef capi<256,5,posit256_t,posit256x2_t,convert_bytes<256,5,posit256_t>> capi256;

// prevent any symbol mangling
extern "C" {

#define POSIT_IMPLS

#define POSIT_NBITS 4
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 8
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 16
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 32
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 64
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 128
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

#define POSIT_NBITS 256
#include "universal/number/posit/posit_c_macros.h"
#undef POSIT_NBITS

}
