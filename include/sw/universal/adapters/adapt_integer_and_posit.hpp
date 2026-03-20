#pragma once
// adapt_integer_and_posit.hpp: adapter functions to convert integer<size> type and posit<nbits,es> types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit_fwd.hpp>
#include <universal/number/integer/integer_fwd.hpp>

// The design assumes you pick your posit and integer, configure their environments,
// and include this header before the concrete posit/integer headers. Doing so defines
// the opt-in macro those types test when deciding whether to expose cross-assignments.
// This keeps the base number-system headers decoupled unless a translation unit explicitly
// asks for the bridge layer.

// Why are the conversion functions not part of the default Integer or Posit types?
// It would tightly couple the types, which we want to avoid.
// If we want to productize these conversions, we would need a new
// layer in the module design that sits above the Universal types. TODO

// If this adapter header is seen first, enable the cross-assignment hooks in the participating types.
// This is intentionally translation-unit scoped rather than a permanent dependency between the libraries.
#ifndef ADAPTER_POSIT_AND_INTEGER
#define ADAPTER_POSIT_AND_INTEGER 1
#else
#define ADAPTER_POSIT_AND_INTEGER 0
#endif // ADAPTER_POSIT_AND_INTEGER

namespace sw { namespace universal {

	namespace internal {
		template<unsigned fbits> class bitblock;
	}

	/**
	 * @brief Convert a posit to an integer by dropping any fractional part.
	 *
	 * @details The posit is decoded into its explicit significant and shifted onto the integer radix.
	 * Negative values are recoded into the integer's two's-complement form after the unsigned magnitude
	 * has been assembled. Values with negative posit scale are mapped to zero, so this adapter behaves
	 * like a truncating conversion rather than a rounded one.
	 */
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

	/**
	 * @brief Convert an integer into the posit conversion bridge used by the posit assignment operators.
	 *
	 * @details The integer is first decomposed into sign, scale, and fraction bits and then marshalled
	 * through `internal::value`. That extra hop avoids duplicating posit rounding/packing logic here:
	 * once the integer has been normalized into the library's generic scientific-notation form, the
	 * existing posit conversion path can perform the final encoding.
	 */
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
