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

// numeric_limits for the LOGARITHMIC takum.
//
// Note on radix: this type's values are powers of sqrt(e), but radix is a
// static constexpr int and cannot express that.  Per the standard it describes
// the radix of the integer representation of the significand, which here is the
// base-2 fixed-point logarithmic value l, so it stays 2.  The value base is
// exposed separately as takum_log<>::value_base.  Likewise min_exponent /
// max_exponent below bound the characteristic c, which is in units of sqrt(e),
// not a power of two.  See docs/takum-design.md.
template <unsigned nbits, unsigned rbits, typename bt>
class numeric_limits< sw::universal::takum_log<nbits, rbits, bt> > {
public:
	using TAKUMLOG = sw::universal::takum_log<nbits, rbits, bt>;
	static constexpr bool is_specialized = true;
	static constexpr TAKUMLOG min() {
		TAKUMLOG lminpos(sw::universal::SpecificValue::minpos);
		return lminpos;
	}
	static constexpr TAKUMLOG max() {
		TAKUMLOG lmaxpos(sw::universal::SpecificValue::maxpos);
		return lmaxpos;
	}
	static constexpr TAKUMLOG lowest() {
		TAKUMLOG lmaxneg(sw::universal::SpecificValue::maxneg);
		return lmaxneg;
	}
	static constexpr TAKUMLOG epsilon() {
		TAKUMLOG one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr TAKUMLOG round_error() { return TAKUMLOG(0.5); }
	static constexpr TAKUMLOG denorm_min()  { return min(); }  // no denormals
	static constexpr TAKUMLOG infinity()    { return TAKUMLOG(INFINITY); }
	static constexpr TAKUMLOG quiet_NaN()   { return TAKUMLOG(NAN); }
	static constexpr TAKUMLOG signaling_NaN() { return TAKUMLOG(NAN); }

	static constexpr int digits       = TAKUMLOG::maxCharBits + 1;
	static constexpr int digits10     = static_cast<int>(digits * 0.301029995663981);
	static constexpr int max_digits10 = static_cast<int>(digits * 0.301029995663981) + 2;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;  // representation radix; value base is sqrt(e)

	// bounds on the characteristic c (units of sqrt(e), not powers of two)
	static constexpr int min_exponent   = static_cast<int>(TAKUMLOG::min_characteristic());
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.217147240951626); // log10(sqrt(e))
	static constexpr int max_exponent   = static_cast<int>(TAKUMLOG::max_characteristic());
	static constexpr int max_exponent10 = static_cast<int>(max_exponent * 0.217147240951626);
	static constexpr bool has_infinity = false;
	static constexpr bool has_quiet_NaN = true;   // NaR serves as NaN
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
