#pragma once
// classify.hpp: classification and sign functions for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <cmath> classification functions, for generic code and for std::complex<faithful> (#1950).
// They classify the value a faithful represents, value + error rounded to the FloatingPointType,
// so a pair built by hand that is not canonical is classified by its sum. They read faithful's
// members rather than <cmath>, so they also serve a faithful over a Universal type such as cfloat.
#include <cmath>         // FP_NAN, FP_INFINITE, FP_ZERO, FP_SUBNORMAL, FP_NORMAL, std::signbit
#include <limits>        // std::numeric_limits<>::min
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>

namespace sw { namespace universal {

template<typename FloatingPointType>
inline bool isnan(const faithful<FloatingPointType>& a) { return a.isnan(); }

template<typename FloatingPointType>
inline bool isinf(const faithful<FloatingPointType>& a) { return a.isinf(); }

template<typename FloatingPointType>
inline bool isfinite(const faithful<FloatingPointType>& a) { return !a.isinf() && !a.isnan(); }

// FP_NAN, FP_INFINITE, FP_ZERO, FP_SUBNORMAL or FP_NORMAL, of the represented value
template<typename FloatingPointType>
inline int fpclassify(const faithful<FloatingPointType>& a) {
	if (a.isnan()) return FP_NAN;
	if (a.isinf()) return FP_INFINITE;
	if (a.iszero()) return FP_ZERO;
	const FloatingPointType v = FloatingPointType(a);   // value + error, rounded
	const FloatingPointType m = v < FloatingPointType(0) ? -v : v;
	return (m < std::numeric_limits<FloatingPointType>::min()) ? FP_SUBNORMAL : FP_NORMAL;
}

template<typename FloatingPointType>
inline bool isnormal(const faithful<FloatingPointType>& a) { return fpclassify(a) == FP_NORMAL; }

template<typename FloatingPointType>
inline bool isdenorm(const faithful<FloatingPointType>& a) { return fpclassify(a) == FP_SUBNORMAL; }

template<typename FloatingPointType>
inline bool iszero(const faithful<FloatingPointType>& a) { return a.iszero(); }

// the sign bit of the represented value. A native FloatingPointType reports it for a zero and a
// NaN too, as std::signbit does: -0 is negative.
template<typename FloatingPointType>
inline bool signbit(const faithful<FloatingPointType>& a) {
	if constexpr (std::is_floating_point_v<FloatingPointType>) {
		const FloatingPointType v = FloatingPointType(a);
		// a zero sum keeps the sign of its value part: (-0, 0) is -0, while (1, -1) sums to +0
		return std::signbit(v == FloatingPointType(0) ? a.high() : v);
	}
	else {
		return a.sign();
	}
}

// a with the sign of b: both parts negated when the sign bits differ
template<typename FloatingPointType>
inline faithful<FloatingPointType> copysign(const faithful<FloatingPointType>& a, const faithful<FloatingPointType>& b) {
	return (signbit(a) != signbit(b)) ? -a : a;
}

// the magnitude; abs() is defined with the type
template<typename FloatingPointType>
inline faithful<FloatingPointType> fabs(const faithful<FloatingPointType>& a) { return abs(a); }

}} // namespace sw::universal
