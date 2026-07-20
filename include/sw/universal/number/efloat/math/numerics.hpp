// numerics.hpp: numeric support functions (frexp, ldexp, scalbn, logb, ilogb, fma) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>    // FP_ILOGB0, FP_ILOGBNAN
#include <climits>  // INT_MAX

namespace sw {
namespace universal {

// ldexp(x, exp) = x * 2^exp. Exact: it only shifts efloat's binary exponent, so
// no rounding occurs and the mantissa is untouched. Zero/inf/nan pass through.
template<unsigned nlimbs>
constexpr efloat<nlimbs> ldexp(const efloat<nlimbs>& x, int exp) {
	if (exp == 0 || x.iszero() || x.isinf() || x.isnan())
		return x;
	efloat<nlimbs> result(x);
	result.setexponent(x.scale() + static_cast<std::int64_t>(exp));
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

// fma(x, y, z) = x*y + z. efloat's product carries no intermediate rounding at the
// working precision, so this is at least as accurate as a hardware fused
// multiply-add. 0*inf yields NaN through efloat's own multiply, matching IEEE.
template<unsigned nlimbs>
constexpr efloat<nlimbs> fma(const efloat<nlimbs>& x, const efloat<nlimbs>& y, const efloat<nlimbs>& z) {
	return x * y + z;
}

}  // namespace universal
}  // namespace sw
