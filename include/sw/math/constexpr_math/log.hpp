#pragma once
// log.hpp: constexpr natural logarithm for IEEE-754 float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Algorithm
// -----------------------------------------------------------------------------
// log(x) = log2(x) * ln(2)
//
// Thin wrapper over the existing log2 machinery (#423 / PR #762). The single
// multiply by detail::LN2 is bit-stable; special-value handling is inherited
// from log2 because log2(NaN), log2(x<0), log2(0), and log2(+inf) propagate
// correctly through the multiplication.
//
// Why not implement directly: the alternative is a parallel artanh series in
// ln-units (avoiding one multiply). The savings is one multiply per call, with
// the cost of duplicating the entire decompose / range-reduce / Horner block.
// At runtime, modern compilers fold the constant multiply away in tight loops;
// at compile time it adds one operation. Not worth the duplication.

#include <limits>

#include <math/constexpr_math/detail.hpp>
#include <math/constexpr_math/log2.hpp>

namespace sw { namespace math { namespace constexpr_math {

// constexpr log(double): natural logarithm of x.
// Special values follow IEEE-754 conventions:
//   log(NaN)  -> NaN
//   log(x<0)  -> NaN
//   log(0)    -> -infinity
//   log(+inf) -> +infinity
//
// The special cases are handled before the multiply because clang's stricter
// constexpr evaluator rejects arithmetic that produces a NaN (i.e.
// log2(NaN) * LN2 would trip the diagnostic even though log2 itself returned
// NaN cleanly via its own early-out).
constexpr double log(double x) {
	if (x != x) return x;                                                // NaN propagation
	if (x < 0.0) return std::numeric_limits<double>::quiet_NaN();
	if (x == 0.0) return -std::numeric_limits<double>::infinity();
	if (x == std::numeric_limits<double>::infinity()) return x;
	return log2(x) * detail::LN2;
}

constexpr float log(float x) {
	if (x != x) return x;
	if (x < 0.0f) return std::numeric_limits<float>::quiet_NaN();
	if (x == 0.0f) return -std::numeric_limits<float>::infinity();
	if (x == std::numeric_limits<float>::infinity()) return x;
	return log2(x) * static_cast<float>(detail::LN2);
}

}}}  // namespace sw::math::constexpr_math
