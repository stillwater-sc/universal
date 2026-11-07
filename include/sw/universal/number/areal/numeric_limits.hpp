#pragma once
// numeric_limits.hpp: definition of numeric_limits for arbitrary real types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
// areal_impl.hpp, NOT the areal.hpp umbrella (#1334). This header is part of the core,
// and the umbrella includes the core -- so including the umbrella back from here made a
// cycle that #pragma once merely masked, dragging manipulators.hpp and iostream.hpp into
// every core-only translation unit and keeping all five I/O-family headers in the graph.
// numeric_limits needs the complete type and nothing else.
#include <universal/number/areal/areal_impl.hpp>
#include <universal/utility/decimal_digits.hpp>   // exact digits10 / max_digits10 (#1597, #1601)
namespace std {

template <unsigned nbits, unsigned es, typename bt> 
class numeric_limits< sw::universal::areal<nbits,es,bt> > {
public:
	using AREAL = sw::universal::areal<nbits, es, bt>;
	// The limits follow the encoding (#1970): a sign, an exponent with bias 2^(es-1) - 1 and
	// gradual underflow, fbits fraction bits, and the ubit, which marks inexact values and is not
	// a digit. min() is the smallest normal value, denorm_min() the smallest subnormal (minpos),
	// epsilon() 2^-fbits, and digits fbits + 1. These were placeholders: min() returned minpos,
	// denorm_min() 1, epsilon() 0 with the ubit set, digits counted the ubit, and has_denorm and
	// is_bounded denied the subnormals and the finite range.
private:
	// the exact encoding of 2^k, for k from MIN_EXP_SUBNORMAL to MAX_EXP, bit by bit so that it
	// is a constant expression for any number of blocks
	static constexpr AREAL pow2(int k) {
		AREAL a;
		if (k >= AREAL::MIN_EXP_NORMAL) {
			const unsigned biased = static_cast<unsigned>(k + AREAL::EXP_BIAS);
			for (unsigned i = 0; i < es; ++i) if ((biased >> i) & 1u) a.set(AREAL::fbits + 1u + i);
		}
		else {
			a.set(1u + static_cast<unsigned>(k - AREAL::MIN_EXP_SUBNORMAL));   // a subnormal: one fraction bit
		}
		return a;
	}
public:
	static constexpr bool is_specialized = true;
	static constexpr AREAL min() { // the smallest positive normal value
		return pow2(AREAL::MIN_EXP_NORMAL);
	}
	static constexpr AREAL max() { // return maximum value
		return AREAL(sw::universal::SpecificValue::maxpos);
	}
	static constexpr AREAL lowest() { // return most negative value
		return AREAL(sw::universal::SpecificValue::maxneg);
	}
	static constexpr AREAL epsilon() { // the distance from 1.0 to the next value, 2^-fbits
		// with es = 1 the encoding cannot hold 2^-fbits, and the smallest positive value is the
		// closest it comes
		if constexpr (-static_cast<int>(AREAL::fbits) >= AREAL::MIN_EXP_SUBNORMAL) return pow2(-static_cast<int>(AREAL::fbits));
		else return denorm_min();
	}
	static constexpr AREAL round_error() { // areal truncates, so the error is up to one ulp
		return AREAL(1.0f);
	}
	static constexpr AREAL denorm_min() {  // the smallest positive subnormal value, minpos
		return AREAL(sw::universal::SpecificValue::minpos);
	}
	static constexpr AREAL infinity() { // return positive infinity
		return AREAL(INFINITY);
	}
	static constexpr AREAL quiet_NaN() { // return non-signaling NaN
		return AREAL(NAN);
	}
	static constexpr AREAL signaling_NaN() { // return signaling NaN
		return AREAL(NAN);
	}

	static constexpr int digits       = static_cast<int>(AREAL::fbits) + 1;   // the significand of an exact value; the ubit is not a digit
	static constexpr int digits10     = sw::universal::decimal_digits10(digits);
	static constexpr int max_digits10 = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	// IEEE convention: 2^(min_exponent - 1) is the smallest normal value, and 2^(max_exponent - 1)
	// the largest power of two below the top of the range
	static constexpr int min_exponent   = AREAL::MIN_EXP_NORMAL + 1;
	static constexpr int min_exponent10 = sw::universal::decimal_min_exponent10(min_exponent);
	static constexpr int max_exponent   = AREAL::MAX_EXP + 1;
	static constexpr int max_exponent10 = sw::universal::decimal_max_exponent10(max_exponent);
	static constexpr bool has_infinity  = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_present;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
