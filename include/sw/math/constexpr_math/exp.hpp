#pragma once
// exp.hpp: constexpr natural exponential for IEEE-754 float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Algorithm
// -----------------------------------------------------------------------------
// exp(x) = exp2(x * log2(e))
//
// Thin wrapper over the existing exp2 machinery (#764 / PR #769). The single
// multiply by detail::LOG2E (= log2(e) = 1/ln(2)) is bit-stable; special-value
// handling is short-circuited before the multiply because clang's stricter
// constexpr evaluator rejects "arithmetic produces a NaN" (i.e.
// exp2(NaN * LOG2E) would trip the diagnostic), mirroring the pattern in
// log.hpp.

#include <limits>

#include <math/constexpr_math/detail.hpp>
#include <math/constexpr_math/exp2.hpp>

namespace sw { namespace math { namespace constexpr_math {

// constexpr exp(double): natural exponential e^x.
// Special values follow IEEE-754 conventions:
//   exp(NaN)  -> NaN
//   exp(+inf) -> +inf
//   exp(-inf) -> 0
//   exp(0)    -> 1  (handled by exp2(0) == 1 in the wrapper path)
//   overflow  -> +inf  (when x * LOG2E >= 1024, propagated by exp2)
//   underflow -> 0     (when x * LOG2E < -1075, propagated by exp2)
constexpr double exp(double x) {
	if (x != x) return x;                                                // NaN propagation
	if (x == std::numeric_limits<double>::infinity()) return x;
	if (x == -std::numeric_limits<double>::infinity()) return 0.0;
	return exp2(x * detail::LOG2E);
}

constexpr float exp(float x) {
	if (x != x) return x;
	if (x == std::numeric_limits<float>::infinity()) return x;
	if (x == -std::numeric_limits<float>::infinity()) return 0.0f;
	return exp2(x * static_cast<float>(detail::LOG2E));
}

}}}  // namespace sw::math::constexpr_math
