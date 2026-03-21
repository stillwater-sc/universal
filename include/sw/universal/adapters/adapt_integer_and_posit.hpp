#pragma once
// adapt_integer_and_posit.hpp: adapter functions to convert integer<size> type and posit<nbits,es> types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit_fwd.hpp>
#include <universal/number/integer/integer_fwd.hpp>

// The design assumes you pick your posit and integer, configure their environments
// and as part of that configuration, you include this file before the posit and integer 
// so that you enable the adapter code in the posit and integer types with the define below.
// This allows us to offer operator=() to assign posits to integers and vice versa.

// Why are the convertion functions not part of the default Integer or Posit types?
// It would tightly couple the types, which we want to avoid.
// If we want to productize these convertions we would need a new
// layer in the module design that sits above the Universal types. TODO

// if included, set the compilation flag that will enable the operator=(const TargetType&) in the SourceType.
#ifndef ADAPTER_POSIT_AND_INTEGER
#define ADAPTER_POSIT_AND_INTEGER 1
#else
#define ADAPTER_POSIT_AND_INTEGER 0
#endif // ADAPTER_POSIT_AND_INTEGER

namespace sw { namespace universal {

	namespace internal {
		template<unsigned fbits> class bitblock;
	}

	// convert a Posit to an Integer
	template<unsigned nbits, unsigned es, unsigned ibits, typename BlockType, IntegerNumberType NumberType>
	inline void convert_p2i(const posit<nbits, es>& p, integer<ibits, BlockType, NumberType>& v) {
		// get the scale of the posit value
		int _scale = scale(p);
		if (_scale < 0) {
			v = 0;
			return;
		}
		if (_scale == 0) {
			v = 1;
			if (p.isneg()) {
				v.flip();
				v += 1;
			}
		}
		else {
			// gather all the fraction bits
			constexpr unsigned fbits = posit<nbits, es>::fbits;
			using BitBlock = internal::bitblock<fbits+1u>;
			BitBlock significant = extract_significant<nbits, es, fbits>(p);
			// the radix point is at fbits, to make an integer out of this
			// we shift that radix point fbits to the right.
			// that is equivalent to a scale of 2^fbits
			v.clear();
			int msb = (v.nbits < p.fbits + 1) ? v.nbits : p.fbits + 1;
			for (int i = msb-1; i >= 0; --i) {
				v.setbit(i, significant[i]);
			}
			int shift = _scale - p.fbits;  // if scale > fbits we need to shift left
			v <<= shift;
			if (p.isneg()) {
				v.flip();
				v += 1;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// convert an Integer to a Posit
	template<unsigned ibits, typename BlockType, IntegerNumberType NumberType, unsigned nbits, unsigned es>
	inline void convert_i2p(const integer<ibits, BlockType, NumberType>& w, posit<nbits, es>& p) {
		using Integer = integer<ibits, BlockType, NumberType>;

		bool sign = w < 0;
		bool isZero = w == 0;
		bool isInf = false;
		bool isNan = false;
		long _scale = scale(w);
		Integer w2 = sign ? twosComplement(w) : w;
		int msb = findMsb(w2);
		internal::bitblock<nbits> fraction_without_hidden_bit;
		int fbit = nbits - 1;
		for (int i = msb - 1; i >= 0; --i) {
			fraction_without_hidden_bit.set(fbit, w2.at(i));
			--fbit;
		}
		internal::value<nbits> v;
		v.set(sign, _scale, fraction_without_hidden_bit, isZero, isInf, isNan);
		p = v;
	}

}} // namespace sw::universal
