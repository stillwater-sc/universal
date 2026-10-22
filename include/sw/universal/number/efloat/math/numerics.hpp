// numerics.hpp: numeric support functions (frexp, ldexp, scalbn, logb, ilogb, fma) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>    // FP_ILOGB0, FP_ILOGBNAN
#include <cstdint>
#include <climits>  // INT_MAX
#include <algorithm>
#include <initializer_list>
#include <universal/number/efloat/math/exponent.hpp>   // exact_product, exact_sum, round_result

namespace sw {
namespace universal {

// ldexp(x, exp) = x * 2^exp. Exact: it only shifts efloat's binary exponent, so
// no rounding occurs and the mantissa is untouched, unless the result leaves the exponent
// range, where it overflows or underflows as a product does (#1786). Zero/inf/nan pass through.
template<unsigned nlimbs>
constexpr efloat<nlimbs> ldexp(const efloat<nlimbs>& x, int exp) {
	if (exp == 0 || x.iszero() || x.isinf() || x.isnan())
		return x;
	efloat<nlimbs> result(x);
	result.setexponent(x.scale() + static_cast<std::int64_t>(exp));
	result.normalize();
	return result;
}

// frexp(x, exp) splits x into a normalized fraction m in [0.5, 1) and an integer
// power of two, such that x == m * 2^(*exp). The binade of x is [2^k, 2^(k+1))
// with k = scale(), so m = x * 2^-(k+1) (leading bit at 2^-1) and *exp = k + 1.
// Exact: only the exponent moves. frexp(0) -> 0 with *exp = 0; inf/nan pass
// through with *exp = 0 (matching <cmath>).
template<unsigned nlimbs>
constexpr efloat<nlimbs> frexp(const efloat<nlimbs>& x, int* exp) {
	if (x.iszero() || x.isinf() || x.isnan()) {
		if (exp != nullptr)
			*exp = 0;
		return x;
	}
	if (exp != nullptr)
		*exp = static_cast<int>(x.scale() + 1);
	efloat<nlimbs> mantissa(x);
	mantissa.setexponent(-1);  // leading bit at 2^-1 -> |mantissa| in [0.5, 1)
	return mantissa;
}

// scalbn(x, n) = x * 2^n -- identical to ldexp for a radix-2 type.
template<unsigned nlimbs>
constexpr efloat<nlimbs> scalbn(const efloat<nlimbs>& x, int n) {
	return ldexp(x, n);
}

// logb(x): the unbiased radix-2 exponent of x as a floating-point value,
// floor(log2(|x|)) -- exactly scale() for a normalized efloat. logb(0) = -inf
// (raises DivisionByZero), logb(+/-inf) = +inf, logb(nan) = nan.
template<unsigned nlimbs>
constexpr efloat<nlimbs> logb(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(false);
		return inf;
	}
	if (x.iszero()) {
		efloat<nlimbs> ninf;
		ninf.setinf(true);
		if (!std::is_constant_evaluated())
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		return ninf;
	}
	return efloat<nlimbs>(static_cast<long long>(x.scale()));
}

// ilogb(x): logb as an int, with the <cmath> special values.
template<unsigned nlimbs>
inline int ilogb(const efloat<nlimbs>& x) {
	if (x.isnan())
		return FP_ILOGBNAN;
	if (x.isinf())
		return INT_MAX;
	if (x.iszero())
		return FP_ILOGB0;
	return static_cast<int>(x.scale());
}

// fma(x, y, z) = x y + z, correctly rounded in the current mode at the wider of the precisions
// x, y and z carry (#1783): the product is exact, and the sum rounds once. Before, x * y rounded
// to the operands' precision before z was added, and the result rounded twice. The special
// values come from efloat's own multiply and add, as IEEE's do: 0 inf is NaN.
template<unsigned nlimbs>
inline efloat<nlimbs> fma(const efloat<nlimbs>& x, const efloat<nlimbs>& y, const efloat<nlimbs>& z) {
	if (x.isnan() || y.isnan() || z.isnan() || x.isinf() || y.isinf()) return x * y + z;
	if (z.isinf()) return z;   // x y is finite, though x * y may overflow: inf - inf would be NaN
	const bool carries = x.carries_precision() || y.carries_precision() || z.carries_precision();
	unsigned precision = x.get_precision();
	if (carries) {
		precision = 0;
		for (const efloat<nlimbs>* v : { &x, &y, &z }) {
			if (v->carries_precision()) precision = std::max(precision, v->get_precision());
		}
	}
	using E = efloat<nlimbs>;
	const int64_t s = x.iszero() || y.iszero() ? 0 : x.scale() + y.scale();   // the product's scale is s or s + 1
	if (s >= E::min_scale && s < E::max_scale) {
		E result = efloat_detail::exact_sum(efloat_detail::exact_product(x, y), z, precision + 2);
		efloat_detail::round_result(result, precision, carries);
		return result;
	}
	// The exact product leaves the exponent range (#1786), though x y + z may not. Scale both
	// terms by 2^-t, t the larger one's scale, so the larger lies in [1, 4): a term more than 2^62
	// binades below it is negligible at any precision, and is raised to 2^(min_scale + 2), where it
	// still lies far below the larger's last bit and exact_sum proxies it alike. The sum rounds at
	// the precision, and 2^t then scales it back, overflowing or underflowing as a product would.
	const int64_t t = z.iszero() ? s : std::max(s, z.scale());
	auto offset = [t](int64_t e) -> int64_t {   // e - t, or min_scale + 2 if that is lower
		if (e == t) return 0;
		return (e < t + E::min_scale + 2) ? E::min_scale + 2 : e - t;   // here t is z's scale, in range
	};
	E xs(x), ys(y), zs(z);
	xs.setexponent(0);
	ys.setexponent(offset(s));
	if (!zs.iszero()) zs.setexponent(offset(z.scale()));
	E result = efloat_detail::exact_sum(efloat_detail::exact_product(xs, ys), zs, precision + 2);
	efloat_detail::round_result(result, precision, carries);
	if (!result.iszero()) {
		const int64_t e = result.scale();   // e in [min_scale, 2], t in [2 min_scale, 2 max_scale]
		int64_t scaled = 0;
		if (t > E::max_scale - e)          scaled = E::max_scale + 1;   // overflows
		else if (t < E::min_scale - 1 - e) scaled = E::min_scale - 2;   // below min / 2: underflows
		else                               scaled = e + t;
		result.setexponent(scaled);
		result.normalize();
	}
	return result;
}

}  // namespace universal
}  // namespace sw
