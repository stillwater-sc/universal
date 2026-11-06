#pragma once
// numeric_limits.hpp: definition of numeric_limits for faithful types
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A faithful<Real> is a value of Real plus the error of that value, a two-part expansion like dd.
// Its limits follow the convention of the multi-double types (#1882), with Real in place of double
// and two parts (#1949):
//   min()      the smallest value at full precision, 2^(min_exponent - 1). Below it the error part
//              goes subnormal and the pair holds fewer than digits bits, so epsilon() no longer
//              bounds the relative rounding error. 2^-969 for double, as for dd.
//   epsilon()  eps(Real)^2 / 2: 2^-105 for double, 2^-47 for float
//   max()      Real's maximum plus the largest error that does not round the sum up to infinity
//   digits     twice Real's
// Every one of these used to be a stub: min(), max() and lowest() were zero, epsilon() and
// denorm_min() one, and digits 3333333.
#include <limits>

namespace std {

template <typename FloatingPointType>
class numeric_limits< sw::universal::faithful<FloatingPointType> > {
	using Real = FloatingPointType;
	using RealLimits = std::numeric_limits<Real>;
public:
	using FAITHFUL = sw::universal::faithful<FloatingPointType>;
	static constexpr bool is_specialized = true;
	static constexpr FAITHFUL min() { // return minimum value at full precision
		return FAITHFUL(sw::universal::faithful_detail::pow2<Real>(min_exponent - 1), Real(0));
	}
	static constexpr FAITHFUL max() { // return maximum value
		FAITHFUL lmaxpos;
		return sw::universal::maxpos<FloatingPointType>(lmaxpos);
	}
	static constexpr FAITHFUL lowest() { // return most negative value
		FAITHFUL lmaxneg;
		return sw::universal::maxneg<FloatingPointType>(lmaxneg);
	}
	static constexpr FAITHFUL epsilon() { // return smallest effective increment from 1.0
		return FAITHFUL(sw::universal::faithful_detail::pow2<Real>(-2 * (RealLimits::digits - 1) - 1), Real(0));
	}
	static constexpr FAITHFUL round_error() { // return largest rounding error
		return FAITHFUL(Real(0.5), Real(0));
	}
	static constexpr FAITHFUL denorm_min() {  // return minimum denormalized value
		FAITHFUL lminpos;
		return sw::universal::minpos<FloatingPointType>(lminpos);
	}
	static constexpr FAITHFUL infinity() { // return positive infinity
		return FAITHFUL(RealLimits::infinity(), Real(0));
	}
	static constexpr FAITHFUL quiet_NaN() { // return non-signaling NaN
		return FAITHFUL(RealLimits::quiet_NaN(), Real(0));
	}
	static constexpr FAITHFUL signaling_NaN() { // return signaling NaN
		return FAITHFUL(RealLimits::signaling_NaN(), Real(0));
	}

	static constexpr int digits       = 2 * RealLimits::digits;
	static constexpr int digits10     = static_cast<int>(digits * 0.30103);
	// enough decimal digits to read the pair back: ceil(1 + digits * log10(2))
	static constexpr int max_digits10 = static_cast<int>(digits * 0.30103) + 2;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	// C++ specification: min() == 2^(min_exponent - 1); the floor rises by Real's digits, as dd's
	// does by double's
	static constexpr int min_exponent   = RealLimits::min_exponent + RealLimits::digits;
	static constexpr int min_exponent10 = static_cast<int>(min_exponent * 0.30103);
	static constexpr int max_exponent   = RealLimits::max_exponent;
	static constexpr int max_exponent10 = RealLimits::max_exponent10;
	static constexpr bool has_infinity      = RealLimits::has_infinity;
	static constexpr bool has_quiet_NaN     = RealLimits::has_quiet_NaN;
	static constexpr bool has_signaling_NaN = RealLimits::has_signaling_NaN;
	// below min() the error part goes subnormal and then underflows, so precision falls off
	// gradually down to denorm_min(), Real's subnormal minimum, as in dd (#1888)
	static constexpr float_denorm_style has_denorm = RealLimits::has_denorm;
	static constexpr bool has_denorm_loss = RealLimits::has_denorm_loss;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

}
