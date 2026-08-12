#pragma once
// numeric_limits.hpp: definition of numeric_limits for takum number system types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <limits>

namespace sw { namespace universal {

// numeric_limits<>::min_exponent and max_exponent are int by standard mandate, but
// a takum characteristic is int64_t.  The specification fixes rbits = 3, where the
// range is [-255, 254] and the two agree; Universal also instantiates wider regime
// fields, and at rbits = 5 the range is [1 - 2^32, 2^32 - 2], which does not fit.
// Narrowing that silently wraps: takum<16,5> reported min_exponent == 1 and
// max_exponent == -2, an inverted range, and gcc flagged it with -Woverflow.
//
// Saturate instead.  The reported bounds are then a conservative subset of the real
// ones -- generic code that sizes a scratch exponent or picks a scaling strategy
// from them stays correct, just pessimistic, rather than being handed a range with
// the wrong sign.  Callers that need the exact bounds read min_characteristic() and
// max_characteristic() off the type, which keep full int64_t width.
constexpr int takum_exponent_cast(std::int64_t c) noexcept {
	constexpr std::int64_t lo = static_cast<std::int64_t>((std::numeric_limits<int>::min)());
	constexpr std::int64_t hi = static_cast<std::int64_t>((std::numeric_limits<int>::max)());
	return static_cast<int>((c < lo) ? lo : ((c > hi) ? hi : c));
}

// floor() as a constant expression.  The arguments here are characteristics scaled
// by log2(sqrt(e)), at most ~3.1e9 in magnitude, so int64_t is ample.
constexpr std::int64_t takum_exponent_floor(double x) noexcept {
	std::int64_t t = static_cast<std::int64_t>(x);
	return (x < 0.0 && static_cast<double>(t) != x) ? (t - 1) : t;
}

// log2(sqrt(e)): converts a takum_log characteristic, which counts powers of the
// value base sqrt(e), into the power-of-two exponent numeric_limits must report.
inline constexpr double takum_log2_of_sqrt_e = 0.7213475204444817;

// floor(log2|value|) for a decoded takum_log magnitude, whose value is sqrt(e)^(c+m).
template<typename Decoded>
constexpr std::int64_t takum_log_exponent_of(const Decoded& d) noexcept {
	return takum_exponent_floor(
		(static_cast<double>(d.c) + d.template fraction<double>()) * takum_log2_of_sqrt_e);
}

}} // namespace sw::universal

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

	// C++ specification: min_exponent is one more than the smallest negative power of
	// the radix that is a normalized value, max_exponent one more than the largest
	// integer power that is representable -- the +1 convention cfloat's specialization
	// also documents.  A linear takum is (1 + f) * 2^c, so the exponent of an encoding
	// is just its characteristic.
	//
	// Taken from the extreme ENCODINGS, magnitude 1 and magnitude_mask(), rather than
	// from min/max_characteristic(): those are the range the format advertises, which
	// the extremes need not attain.  takum<12,3> is the case in point -- its narrowest
	// DR has no trailing field, so minpos sits at c == -254, not at the advertised
	// -255.  Both agree with std::ilogb(minpos) + 1 and std::ilogb(maxpos) + 1.
	// Saturating on the way to int: see takum_exponent_cast above.
	static constexpr int min_exponent   = sw::universal::takum_exponent_cast(
		TAKUM::Codec::decode(1ull).c + 1);
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.301029995663981);
	static constexpr int max_exponent   = sw::universal::takum_exponent_cast(
		TAKUM::Codec::decode(TAKUM::Codec::magnitude_mask()).c + 1);
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
	// takum_codec::encode_rounded() rounds the trailing field -- and, when the layout
	// leaves none, the characteristic -- to nearest with ties to even.  Conversion
	// never truncates toward zero.
	static constexpr float_round_style round_style = round_to_nearest;
};

// numeric_limits for the LOGARITHMIC takum.
//
// Note on radix: this type's values are powers of sqrt(e), but radix is a
// static constexpr int and cannot express that.  Per the standard it describes
// the radix of the integer representation of the significand, which here is the
// base-2 fixed-point logarithmic value l, so it stays 2.  The value base is
// exposed separately as takum_log<>::value_base.  Because radix is 2, the
// exponent fields below are radix-2 exponents rather than characteristics --
// the conversion is done where they are defined.  See docs/takum-design.md.
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

	// These are radix-2 exponents, as radix == 2 above requires, NOT characteristics.
	// A takum_log characteristic counts powers of sqrt(e), so reporting it raw would
	// overstate the range by 1/log2(sqrt(e)) ~ 1.39x.  Taken from the extreme encodings
	// and converted, with the same +1 convention the linear specialization documents.
	// Both agree with std::ilogb(minpos) + 1 and std::ilogb(maxpos) + 1.
	static constexpr int min_exponent   = sw::universal::takum_exponent_cast(
		sw::universal::takum_log_exponent_of(TAKUMLOG::Codec::decode(1ull)) + 1);
	static constexpr int max_exponent   = sw::universal::takum_exponent_cast(
		sw::universal::takum_log_exponent_of(
			TAKUMLOG::Codec::decode(TAKUMLOG::Codec::magnitude_mask())) + 1);
	// base 10 follows from the base-2 bounds above, so log10(2) -- not log10(sqrt(e)),
	// which would apply the base conversion a second time
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.301029995663981);
	static constexpr int max_exponent10 = static_cast<int>(max_exponent * 0.301029995663981);
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
	static constexpr float_round_style round_style = round_to_nearest;  // as takum<>, same codec
};

}
