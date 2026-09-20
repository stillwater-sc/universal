#pragma once
// elreal_reference_digits.hpp: exact decimal-agreement oracle for elreal ZBCL values.
//
// Converts a ZBCL<FpType> to its EXACT dyadic value and reports how many leading
// significant decimal digits it agrees with a decimal reference string (e.g. the
// 320-digit constant strings in include/sw/math/constants/reference_constants.hpp).
// This is the verification substrate for validating elreal constants and
// transcendentals to hundreds of digits (issues #1048 / #1049): a host-double
// tolerance check (~16 digits) cannot see high-precision operator bugs, but an
// exact comparison can -- it caught e_zbcl summing host-double 1/n! terms (only
// ~17 digits correct) and euler_gamma_zbcl returning a bare double literal.
//
// Method: a ZBCL is an exact dyadic rational  sum_i (v_i * 2^exp_i)  with each v_i
// an IEEE double, so it equals  M * 2^s  for integers M, s (the einteger-backed
// `dyadic` oracle in dyadic_exact.hpp). A decimal reference "d.dddd..." is the
// exact rational  N / 10^frac. The two are compared by cross-multiplication in
// einteger -- no rounding, no shared code path with elreal arithmetic.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>   // dyadic, and the agreed_decimal_digits comparators it carries (#1566)

namespace sw { namespace universal {

// Exact dyadic value of a ZBCL: sum of (block value exactly) * 2^block.exp. Each
// block value v_i is an IEEE float widened losslessly to double, so the result is
// exact for any host FpType. `maxBlocks` bounds the (otherwise lazy) materialisation.
template <typename FpType>
inline dyadic zbcl_to_dyadic(const ZBCL<FpType>& z, std::size_t maxBlocks = 512) {
	// The exact dyadic value relies on widening each block value v_i to double
	// LOSSLESSLY. That holds for every host the library uses (bfloat16 k=8,
	// fp16 k=11, float k=24, double k=53) but not for a host wider than double
	// (e.g. long double k=64), which would silently make the "exact" oracle
	// inexact. Reject that at compile time.
	static_assert(std::numeric_limits<FpType>::digits <= std::numeric_limits<double>::digits,
	              "zbcl_to_dyadic widens each block value to double exactly; an FpType "
	              "wider than double (e.g. long double) would lose precision");
	dyadic acc;  // 0
	for (const auto& b : z.take(maxBlocks)) {
		dyadic d = dyadic::from_double(static_cast<double>(b.v));
		d.scale += static_cast<int>(b.exp);   // block value == v * 2^exp; b.exp is
		                                      // a wide integer<256> (#1066) -- the
		                                      // offset fits int for any real host
		                                      // (combined exponent ~ +/- 1074).
		acc = acc + d;
	}
	return acc;
}

// Convenience overload: agreement of a ZBCL directly against a reference string.
template <typename FpType>
inline int agreed_decimal_digits(const ZBCL<FpType>& z, std::string_view ref, int cap = 320) {
	return agreed_decimal_digits(zbcl_to_dyadic(z), ref, cap);
}

// Convenience overload: relative agreement of two ZBCL values directly.
template <typename FpType>
inline int agreed_decimal_digits(const ZBCL<FpType>& a, const ZBCL<FpType>& b, int cap = 320) {
	return agreed_decimal_digits(zbcl_to_dyadic(a), zbcl_to_dyadic(b), cap);
}

}}  // namespace sw::universal
