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
#include <universal/verification/dyadic_exact.hpp>

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
		d.scale += b.exp;       // block value == v * 2^exp
		acc = acc + d;
	}
	return acc;
}

// Number of leading significant decimal digits to which the exact value V agrees
// with the positive decimal reference string `ref` (e.g. "3.14159..."). Returns
// `cap` if they agree to at least `cap` digits (or are exactly equal).
//
//   relative error  = |V - ref| / |ref|
//   with  V == Vn/Vd  and  ref == N / 10^frac, the error numerator (cross-
//   multiplied) is  |Vn*10^frac - N*Vd|  over denominator  Vd*N, so the value
//   agrees to >= d digits iff  |Vn*10^frac - N*Vd| * 10^d  <=  Vd*N.
inline int agreed_decimal_digits(const dyadic& V, std::string_view ref, int cap = 320) {
	using bigint = dyadic::bigint;

	// Parse the positive reference into numerator N and fractional-digit count.
	// Reject malformed input rather than silently skipping it: for a verification
	// oracle an unsupported reference (a typo like "3e10", a sign, whitespace)
	// must fail fast, not be reinterpreted as a different number.
	std::string digits;
	int frac = -1;                        // -1 until the '.' is seen
	for (char c : ref) {
		if (c == '.') {
			if (frac >= 0) throw std::invalid_argument("decimal reference: multiple '.'");
			frac = 0;
			continue;
		}
		if (c >= '0' && c <= '9') {
			digits.push_back(c);
			if (frac >= 0) ++frac;
			continue;
		}
		throw std::invalid_argument("decimal reference: unsupported character");
	}
	if (digits.empty()) throw std::invalid_argument("decimal reference: no digits");
	if (frac < 0) frac = 0;               // integer-only reference
	bigint N; N.assign(digits);           // ref == N / 10^frac

	// V == Vn / Vd  (Vd a power of two, or 1).
	bigint Vn = V.numerator, Vd(1);
	if (V.scale >= 0) Vn <<= V.scale; else Vd <<= (-V.scale);

	bigint tenFrac; tenFrac.assign("1" + std::string(static_cast<std::size_t>(frac), '0'));
	bigint diff = Vn * tenFrac - N * Vd;  // error numerator (signed)
	if (diff.sign()) diff.setsign(false); // |error numerator|
	bigint denom = Vd * N;                // positive

	if (diff.iszero()) return cap;        // exact to the cap

	// largest d with diff * 10^d <= denom
	bigint cur = diff, ten; ten.assign("10");
	for (int d = 1; d <= cap; ++d) {
		cur = cur * ten;
		if (denom < cur) return d - 1;    // diff*10^d > denom: agreement lost at digit d
	}
	return cap;
}

// Convenience overload: agreement of a ZBCL directly against a reference string.
template <typename FpType>
inline int agreed_decimal_digits(const ZBCL<FpType>& z, std::string_view ref, int cap = 320) {
	return agreed_decimal_digits(zbcl_to_dyadic(z), ref, cap);
}

// Number of leading significant decimal digits to which exact value A agrees with
// exact value B (relative to B). For identity residual checks where neither side
// has a closed-form decimal reference -- e.g. exp(a+b) == exp(a)*exp(b) or the
// sin(a+b) expansion (#1049): build both sides as exact dyadics and compare them
// directly, with no shared code path through elreal arithmetic.
//
//   relative error = |A - B| / |B|
//   with  A == An/Ad,  B == Bn/Bd  (denominators powers of two or 1), the cross-
//   multiplied error numerator is  |An*Bd - Bn*Ad|  over denominator  |Ad*Bn|, so
//   A agrees with B to >= d digits iff  |An*Bd - Bn*Ad| * 10^d  <=  |Ad*Bn|.
inline int agreed_decimal_digits(const dyadic& A, const dyadic& B, int cap = 320) {
	using bigint = dyadic::bigint;

	bigint An = A.numerator, Ad(1);
	if (A.scale >= 0) An <<= A.scale; else Ad <<= (-A.scale);
	bigint Bn = B.numerator, Bd(1);
	if (B.scale >= 0) Bn <<= B.scale; else Bd <<= (-B.scale);

	bigint diff = An * Bd - Bn * Ad;      // error numerator (signed)
	if (diff.sign()) diff.setsign(false); // |error numerator|
	bigint denom = Ad * Bn;               // |B| numerator (Ad > 0)
	if (denom.sign()) denom.setsign(false);
	if (denom.iszero()) throw std::invalid_argument("agreed_decimal_digits: zero reference value");

	if (diff.iszero()) return cap;        // exact to the cap

	bigint cur = diff, ten; ten.assign("10");
	for (int d = 1; d <= cap; ++d) {
		cur = cur * ten;
		if (denom < cur) return d - 1;    // diff*10^d > denom: agreement lost at digit d
	}
	return cap;
}

// Convenience overload: relative agreement of two ZBCL values directly.
template <typename FpType>
inline int agreed_decimal_digits(const ZBCL<FpType>& a, const ZBCL<FpType>& b, int cap = 320) {
	return agreed_decimal_digits(zbcl_to_dyadic(a), zbcl_to_dyadic(b), cap);
}

}}  // namespace sw::universal
