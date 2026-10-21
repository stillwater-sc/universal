// hypot.hpp: hypotenuse function hypot(x, y) = sqrt(x^2 + y^2) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <cstdint>
#include <utility>
#include <universal/number/efloat/math/exponent.hpp>  // exact_product, exact_sum, round_result
#include <universal/number/efloat/math/sqrt.hpp>      // sqrt_to

namespace sw {
namespace universal {

// hypot(x, y) = sqrt(x^2 + y^2), correctly rounded in the current mode at the wider of the
// precisions x and y carry (#1783). IEEE-754 special cases:
//   - if either argument is +/-inf, the result is +inf (even if the other is NaN)
//   - otherwise if either argument is NaN, the result is NaN
//   - hypot(x, y) == hypot(y, x) == hypot(x, -y); hypot(0, y) == |y|
template<unsigned nlimbs>
inline efloat<nlimbs> hypot(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	// IEEE-754: Infinity suppresses NaN inside hypot! Infinity check must run first.
	if (x.isinf() || y.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(false);
		return inf;
	}
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan;
		nan.setnan();
		return nan;
	}

	// the result's precision is the wider one x and y carry (#1757)
	const unsigned precision = efloat_detail::result_precision(x, y);
	const bool carries = x.carries_precision() || y.carries_precision();
	efloat<nlimbs> big(x), small(y);
	big.setsign(false);
	small.setsign(false);
	if (big < small) std::swap(big, small);
	if (small.iszero()) {   // hypot(x, 0) = |x|, exact; hypot(0, 0) = +0
		efloat_detail::round_result(big, precision, carries);
		return big;
	}
	const int64_t k = big.scale();
	const int64_t width = std::max<int64_t>(efloat_detail::significant_bits(big), precision + 2);
	if (k - small.scale() > width + 8) {
		// hypot = big + c with 0 < c < small^2 / (2 big) < 2^(k - 2 width - 14): c lies wholly below
		// big's last bit at `width` bits, so big plus any such c rounds as the result does
		efloat<nlimbs> c(1);
		c.setexponent(k - width - 2);
		efloat<nlimbs> result = efloat_detail::exact_sum(big, c, precision + 2);
		efloat_detail::round_result(result, precision, carries);
		return result;
	}
	// hypot(x, y) = 2^k hypot(x 2^-k, y 2^-k): the squares stay well inside the exponent's range.
	// They are exact, and so is their sum, or it rounds as the exact one does for a root of up to
	// `precision` bits (pad 2 precision + 4); the root is exact before it rounds once (#1783).
	// Before, max sqrt(1 + (min / max)^2) rounded four times and was off by up to an ulp.
	big.setexponent(0);
	small.setexponent(small.scale() - k);
	const efloat<nlimbs> sum = efloat_detail::exact_sum(efloat_detail::exact_product(big, big),
	                                                    efloat_detail::exact_product(small, small), 2 * precision + 4);
	efloat<nlimbs> result = efloat_detail::sqrt_to(sum, precision);
	result.setexponent(result.scale() + k);
	efloat_detail::round_result(result, precision, carries);
	return result;
}

}  // namespace universal
}  // namespace sw
