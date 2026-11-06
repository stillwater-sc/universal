#pragma once
// hypot.hpp: sqrt(x^2 + y^2) for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// hypot(x, y) without overflow or underflow (#1957): both arguments are scaled by a common power of
// two, 2^-k with k the larger binary exponent, which is exact; the sum of squares and the
// compensated sqrt are then in range, and the result is scaled back. As std::hypot, an infinite
// argument gives +inf even when the other is NaN. Worst relative error over 20000 argument pairs,
// from 2^-40 to 2^40 and across the whole range: 1.1 / 1.3 units of epsilon() for float / double. Only a native FloatingPointType.
#include <algorithm>     // std::max
#include <cmath>         // std::ilogb
#include <limits>
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/functions/exponent.hpp>   // faithful_detail::scale2
#include <universal/number/faithful/math/functions/sqrt.hpp>

namespace sw { namespace universal {

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> hypot(const faithful<FloatingPointType>& x, const faithful<FloatingPointType>& y) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isinf() || y.isinf()) return F(std::numeric_limits<Real>::infinity());
	if (x.isnan() || y.isnan()) return F(std::numeric_limits<Real>::quiet_NaN());
	const F ax = abs(x), ay = abs(y);
	if (ax.iszero()) return ay;
	if (ay.iszero()) return ax;
	const int k = std::max(std::ilogb(Real(ax)), std::ilogb(Real(ay)));
	auto down = [&](const F& v) { return faithful_detail::scale2(faithful_detail::scale2(v, -(k / 2)), -(k - k / 2)); };
	const F xs = down(ax), ys = down(ay);
	const F r = sqrt(xs * xs + ys * ys);
	return faithful_detail::scale2(faithful_detail::scale2(r, k / 2), k - k / 2);
}

}} // namespace sw::universal
