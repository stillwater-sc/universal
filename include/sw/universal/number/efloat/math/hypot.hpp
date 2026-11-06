// hypot.hpp: hypotenuse function hypot(x, y) = sqrt(x^2 + y^2) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <universal/number/efloat/math/exponent.hpp>  // exact_sum, limb_buffer, round_result

namespace sw {
namespace universal {

namespace efloat_detail {

	// sqrt(x^2 + y^2) for finite x and y != 0 with scale(x) >= scale(y), rounded once at
	// `precision` bits (#1802), as sqrt_exact rounds a root. With x = X 2^(k - 64 mx + 1) and
	// F = 64 n - 1, n >= mx, the frame N = floor((x^2 + y^2) 2^(2F - 2k - 2c)), c = 1 when the sum
	// is 4^(k + 1) or more, is the integer X^2 2^(128 (n - mx)) plus y^2's part floored, divided by
	// 4^c and floored again; the bits either floor drops are a sticky bit. N is the only integer
	// the squares' sum floors to, and floor(sqrt(N)) the root's floor, since no square lies
	// strictly between N and N + 1. The root S, n limbs with its top bit set, then carries the
	// rounding bits, and the remainder N - S^2 and the dropped bits the sticky one. The root comes
	// from limb_kernels::sqrt_approx first, and the exact one only where Ziv's test fails (#1810).
	template<unsigned nlimbs>
	inline constexpr void hypot_limbs(efloat<nlimbs>& result, const efloat<nlimbs>& x, const efloat<nlimbs>& y, unsigned precision) {
		using limb_kernels::limb_t;
		const std::size_t mx = x.limb_count();
		const std::size_t my = y.limb_count();
		const std::size_t n  = std::max<std::size_t>((static_cast<std::size_t>(precision) + 2 + 63) / 64, mx);
		const int64_t     d  = x.scale() - y.scale();   // >= 0, and at most a few limbs past the precision
		const std::size_t m  = std::max(mx, my);
		const limb_t* px = x.limb_data();   // read in place, as fma reads its operands (#1832)
		const limb_t* py = y.limb_data();
		limb_buffer buffer;
		limb_t* x2   = buffer.get(2 * mx + 2 * my + 2 * (2 * n + 1) + n + std::max(limb_kernels::scratch_limbs(m), limb_kernels::sqrt_scratch_limbs(n)));
		limb_t* y2   = x2 + 2 * mx;
		limb_t* np   = y2 + 2 * my;
		limb_t* t    = np + (2 * n + 1);
		limb_t* sp   = t + (2 * n + 1);
		limb_t* work = sp + n;
		limb_kernels::mul_n(x2, px, px, mx, work, true);
		limb_kernels::mul_n(y2, py, py, my, work, true);
		// N0 = (x^2 + y^2) 2^(2F - 2k) in 2n + 1 limbs: X^2 at 2F - 2(64 mx - 1) = 128 (n - mx), Y^2 2d lower
		const std::size_t len = 2 * n + 1;
		limb_kernels::load_window(np, len, x2, 2 * mx, 128 * static_cast<int64_t>(n - mx));
		bool sticky = limb_kernels::load_window(t, len, y2, 2 * my, 128 * static_cast<int64_t>(n) - 128 * static_cast<int64_t>(my) - 2 * d);
		limb_kernels::add_n(np, np, t, len);
		int64_t c = 0;
		if (np[2 * n] != 0u) {   // the sum is in [4, 8): N = N0 / 4, its root twice the frame's
			sticky = sticky || (np[0] & 3u) != 0u;
			limb_kernels::rshift(np, np, len, 2);
			c = 1;
		}
		// the root without its remainder first, as sqrt takes it (#1810): S, or a few units above,
		// rounds wherever Ziv's test passes on it; otherwise the exact root and remainder decide
		const unsigned plimbs = (precision + 31) / 32;
		for (std::size_t i = 0; i < 2 * n; ++i) t[i] = np[i];   // N, kept for the exact root
		limb_kernels::sqrt_approx(sp, np, n, work);
		const unsigned lost = static_cast<unsigned>(std::bit_width(limb_kernels::sqrt_approx_error(n) + 1));
		if (limb_kernels::can_round(sp, n, 32 * plimbs, static_cast<unsigned>(64 * n - 1) - lost)) {
			result.assign_rounded_limbs(false, sp, n, x.scale() + c, true, plimbs);
			return;
		}
		for (std::size_t i = 0; i < 2 * n; ++i) np[i] = t[i];
		const limb_t carry = limb_kernels::sqrtrem(sp, np, n, work);
		sticky = sticky || (carry != 0u);
		for (std::size_t i = 0; !sticky && i < n; ++i) sticky = (np[i] != 0u);
		result.assign_rounded_limbs(false, sp, n, x.scale() + c, sticky, plimbs);
	}

}  // namespace efloat_detail

// hypot(x, y) = sqrt(x^2 + y^2), correctly rounded in the current mode at the wider of the
// precisions x and y carry (#1783). IEEE-754 special cases:
//   - if either argument is +/-inf, the result is +inf (even if the other is NaN)
//   - otherwise if either argument is NaN, the result is NaN
//   - hypot(x, y) == hypot(y, x) == hypot(x, -y); hypot(0, y) == |y|
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> hypot(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
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
	// the larger by scale: arguments of one scale may come in either order
	const bool swapped = !y.iszero() && (x.iszero() || y.scale() > x.scale());
	const efloat<nlimbs>& larger  = swapped ? y : x;
	const efloat<nlimbs>& smaller = swapped ? x : y;
	if (smaller.iszero()) {   // hypot(x, 0) = |x|, exact; hypot(0, 0) = +0
		efloat<nlimbs> big(larger);
		big.setsign(false);
		efloat_detail::round_result(big, precision, carries);
		return big;
	}
	const int64_t k = larger.scale();
	const int64_t width = std::max<int64_t>(efloat_detail::significant_bits(larger), precision + 2);
	if (k - smaller.scale() > width + 8) {
		// hypot = big + c with 0 < c < small^2 / (2 big) < 2^(k - 2 width - 14): c lies wholly below
		// big's last bit at `width` bits, so big plus any such c rounds as the result does
		efloat<nlimbs> big(larger);
		big.setsign(false);
		efloat<nlimbs> c(1);
		c.setexponent(k - width - 2);
		efloat<nlimbs> result = efloat_detail::exact_sum(big, c, precision + 2);
		efloat_detail::round_result(result, precision, carries);
		return result;
	}
	// the squares, their sum and its root on limbs, rounded once (#1802); before, max sqrt(1 +
	// (min / max)^2) rounded four times and was off by up to an ulp (#1783)
	efloat<nlimbs> result;
	efloat_detail::hypot_limbs(result, larger, smaller, precision);
	efloat_detail::round_result(result, precision, carries);
	return result;
}

}  // namespace universal
}  // namespace sw
