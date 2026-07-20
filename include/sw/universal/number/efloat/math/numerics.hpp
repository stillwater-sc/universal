// numerics.hpp: numeric support functions (frexp, ldexp) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

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

}  // namespace universal
}  // namespace sw
