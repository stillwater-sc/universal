// classify.hpp: classification functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>

namespace sw { namespace universal {

// STD LIB function: Categorizes efloat value into: zero, normal, infinite, NaN
template<unsigned nlimbs>
constexpr int fpclassify(const efloat<nlimbs>& x) noexcept {
	if (x.iszero()) return FP_ZERO;
	if (x.isnan()) return FP_NAN;
	if (x.isinf()) return FP_INFINITE;
	return FP_NORMAL;
}

// STD LIB function: Determines if the efloat has a finite value (normal or zero)
template<unsigned nlimbs>
constexpr bool isfinite(const efloat<nlimbs>& x) noexcept {
	return !x.isinf() && !x.isnan();
}

// STD LIB function: Determines if the efloat is positive or negative infinity
template<unsigned nlimbs>
constexpr bool isinf(const efloat<nlimbs>& x) noexcept {
	return x.isinf();
}

// STD LIB function: Determines if the efloat is a NaN
template<unsigned nlimbs>
constexpr bool isnan(const efloat<nlimbs>& x) noexcept {
	return x.isnan();
}

// STD LIB function: Determines if the efloat is normal (neither zero, infinite, nor NaN)
template<unsigned nlimbs>
constexpr bool isnormal(const efloat<nlimbs>& x) noexcept {
	return !x.iszero() && !x.isinf() && !x.isnan();
}

// Determines if the efloat is denormalized (always false for efloat as it is always normalized)
template<unsigned nlimbs>
constexpr bool isdenorm(const efloat<nlimbs>& x) noexcept {
	return false;
}

// STD LIB function: Determines if the sign of the efloat is negative
template<unsigned nlimbs>
constexpr bool signbit(const efloat<nlimbs>& x) noexcept {
	return x.sign() == -1;
}

}} // namespace sw::universal
