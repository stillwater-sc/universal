#pragma once
// cbrt.hpp: the cube root of a faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A compensated cube root (#1957), as sqrt.hpp's square root: scale |x| by 2^(-3k) into
// [1/8, 8), exactly, so the cube below cannot overflow or reach the subnormals; take the
// FloatingPointType's cube root r; apply two Newton corrections r - (r^3 - b) / (3 r^2), with r^3
// formed in faithful arithmetic (r * r is exact through twoProd); scale back by 2^k. Each step
// doubles the bits of r; two are needed because std::cbrt can be an ulp off. Worst relative error
// over 20000 arguments across the range: 1.2 / 1.0 units of epsilon() for float / double. cbrt(-x) = -cbrt(x), and cbrt of +-0, +-inf and NaN is the argument.
// Only a native FloatingPointType.
#include <cmath>         // std::cbrt, std::ilogb, std::ldexp
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/functions/exponent.hpp>   // faithful_detail::scale2

namespace sw { namespace universal {

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> cbrt(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	const F ax = abs(x);
	int k = std::ilogb(Real(ax));
	k = (k >= 0) ? k / 3 : -((-k + 2) / 3);          // floor(ilogb / 3)
	const F b = faithful_detail::scale2(faithful_detail::scale2(ax, -(3 * k) / 2), -(3 * k - (3 * k) / 2));
	// two Newton corrections: std::cbrt is not correctly rounded (glibc is up to an ulp off), and
	// one step from an ulp off leaves a few units of the pair
	F root(std::cbrt(Real(b)));
	for (int i = 0; i < 2; ++i) root = root - (root * root * root - b) / (F(Real(3)) * root * root);
	const F result = faithful_detail::scale2(root, k);
	return x.sign() ? -result : result;
}

}} // namespace sw::universal
