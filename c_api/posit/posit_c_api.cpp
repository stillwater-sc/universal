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

// marshal128 takes a posit128_t and marshals it into a raw bitblock
static void marshal128(posit128_t a, sw::unum::bitblock<128>& raw) {
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

// unmarshal128 takes a raw bitblock and unmarshals it into a posit128_t
static void unmarshal128(sw::unum::bitblock<128>& raw, posit128_t& a) {
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

template<size_t nbits, size_t es, class positN_t> class convert {
	static sw::unum::posit<nbits, es> decode(positN_t bits);
	static positN_t encode(sw::unum::posit<nbits, es> p);
};
template<size_t nbits, size_t es, class positN_t> class convert_small : convert<nbits,es,positN_t> {
	public:
	static sw::unum::posit<nbits, es> decode(positN_t bits) {
		sw::unum::posit<nbits, es> pa;
		pa.set_raw_bits((uint64_t) bits.v);
		return pa;
	}
	static positN_t encode(sw::unum::posit<nbits, es> p) {
		return { p.encoding() };
	}
};
template<size_t nbits, size_t es, class positN_t> class convert_big : convert<nbits,es,positN_t> {
	public:
	static sw::unum::posit<nbits, es> decode(positN_t bits) {
		sw::unum::posit<nbits, es> pa;
		sw::unum::bitblock<nbits> raw;
		marshal128(bits, raw);
		pa.set(raw);
		return pa;
	}
	static positN_t encode(sw::unum::posit<nbits, es> p) {
		posit128_t out;
		sw::unum::bitblock<nbits> raw = p.get();
		unmarshal128(raw, out);
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

typedef capi<8,0,posit8_t,convert_small<8,0,posit8_t>> capi8;
typedef capi<16,1,posit16_t,convert_small<16,1,posit16_t>> capi16;
typedef capi<32,2,posit32_t,convert_small<32,2,posit32_t>> capi32;
typedef capi<64,3,posit64_t,convert_small<64,3,posit64_t>> capi64;
typedef capi<128,4,posit128_t,convert_big<128,4,posit128_t>> capi128;

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

}
