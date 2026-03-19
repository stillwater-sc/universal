#pragma once
// numeric_limits.hpp: definition of numeric_limits for takum number system types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

template <unsigned nbits, unsigned rbits, typename bt>
class numeric_limits< sw::universal::takum<nbits, rbits, bt> > {
public:
	using TAKUM = sw::universal::takum<nbits, rbits, bt>;
	static constexpr bool is_specialized = true;
	static constexpr TAKUM  min() { // return minimum value
		TAKUM lminpos(sw::universal::SpecificValue::minpos);
		return lminpos;
	}
	static constexpr TAKUM  max() { // return maximum value
		TAKUM lmaxpos(sw::universal::SpecificValue::maxpos);
		return lmaxpos;
	}
	static constexpr TAKUM  lowest() { // return most negative value
		TAKUM lmaxneg(sw::universal::SpecificValue::maxneg);
		return lmaxneg;
	}
	static constexpr TAKUM  epsilon() { // return smallest effective increment from 1.0
		TAKUM one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr TAKUM  round_error() { // return largest rounding error
		return TAKUM(0.5);
	}
	static constexpr TAKUM  denorm_min() {  // return minimum denormalized value
		return min();  // no denormals: denorm_min == min per C++ standard
	}
	static constexpr TAKUM  infinity() { // return positive infinity
		return TAKUM(INFINITY);
	}
	static constexpr TAKUM  quiet_NaN() { // return non-signaling NaN
		return TAKUM(NAN);
	}
	static constexpr TAKUM  signaling_NaN() { // return signaling NaN
		return TAKUM(NAN);
	}

	// Maximum mantissa bits: nbits - overhead (when r=0)
	static constexpr int digits       = TAKUM::maxCharBits + 1; // +1 for implicit leading bit
	static constexpr int digits10     = static_cast<int>(digits * 0.301029995663981); // log10(2)
	static constexpr int max_digits10 = static_cast<int>(digits * 0.301029995663981) + 2;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = TAKUM::min_characteristic();
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.301029995663981);
	static constexpr int max_exponent   = TAKUM::max_characteristic();
	static constexpr int max_exponent10 = static_cast<int>(max_exponent * 0.301029995663981);
	static constexpr bool has_infinity = false;
	static constexpr bool has_quiet_NaN = true;  // NaR serves as NaN
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
