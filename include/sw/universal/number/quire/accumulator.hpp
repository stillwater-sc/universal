#pragma once
// accumulator.hpp: the quire as a pluggable super-accumulator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A linear-algebra library that lets the caller choose how an inner product is
// accumulated (MTL5's mtl::math::accumulator_traits<Acc, Value>) needs four
// operations on the accumulator: clear it, assign one value, add a product, and
// round the result out once. For a quire these are free functions, found by
// argument-dependent lookup on the quire type, so the library names them and
// Universal defines them, and neither includes the other:
//
//     accumulator_clear(q)                           q = 0
//     accumulator_assign(q, v)                       q = v, exactly
//     accumulator_add_product(q, a, b)               q += a * b (see below)
//     accumulator_value(q, std::type_identity<R>{})  round q to R
//     accumulator_merge(q, other)                    q += other, exactly
//
// With this header, an MTL5 kernel instantiated as, for example,
//
//     using P = posit<32, 2>;
//     mtl::dot<quire<P>, P>(x, y);
//     sparse_lu_solve<quire<P>>(A, x, b);
//
// accumulates in the quire, with no bridge header in between. accumulator_merge
// combines the partial quires of a parallel or distributed reduction exactly, so
// the result doesn't depend on how the terms were split.
//
// What is exact depends on the number type's quire_mul. For posit, bposit,
// cfloat and fixpnt the product is exact, so a sum of products is accumulated
// exactly and rounded once. For lns and dbns, quire_mul forms the product
// through double and rounds it (see lns/fdp.hpp and dbns/fdp.hpp): the
// accumulation of those rounded products is still exact, but each term carries
// one product rounding.
//
// accumulator_value rounds to nearest, ties to even, exactly once when the
// result is the element type (quire_resolve) or float/double (rounded here
// from the full quire, not through convert_to<>(), which truncates to 53 bits).
// Any other result type is reached through the correctly rounded double, so it
// may round twice.
//
// The functions apply to quire<NT, capacity, LimbType> whenever NT has an
// ADL-visible quire_mul and quire_resolve, which every Universal number type
// with a quire provides in its fdp.hpp (posit, bposit, cfloat, fixpnt, lns,
// dbns). The element type must be exactly NT. Native float and double quires
// are excluded: their quire_mul takes built-in types, so argument-dependent
// lookup can't find it from here.
//
// Relates to stillwater-sc/mtl5#582

#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <universal/number/quire/quire_fwd.hpp>

namespace sw { namespace universal {

// Does NT provide the exact product and the single round-out a quire needs?
template<typename NumberType, unsigned capacity, typename LimbType>
concept quire_accumulable = !std::is_arithmetic_v<NumberType> &&
	requires(const NumberType& a, const quire<NumberType, capacity, LimbType>& q) {
		quire_mul(a, a);
		{ quire_resolve(q) } -> std::convertible_to<NumberType>;
	};

/// q = 0
template<typename NumberType, unsigned capacity, typename LimbType>
	requires quire_accumulable<NumberType, capacity, LimbType>
void accumulator_clear(quire<NumberType, capacity, LimbType>& q) {
	q.clear();
}

/// q = v, exactly: v enters the quire as the unrounded product v * 1
template<typename NumberType, unsigned capacity, typename LimbType, typename Value>
	requires quire_accumulable<NumberType, capacity, LimbType> && std::same_as<Value, NumberType>
void accumulator_assign(quire<NumberType, capacity, LimbType>& q, const Value& v) {
	q.clear();
	q += quire_mul(v, NumberType(1));
}

/// q += a * b. The product is exact for posit, bposit, cfloat and fixpnt;
/// lns and dbns round it once in their quire_mul.
template<typename NumberType, unsigned capacity, typename LimbType, typename Value>
	requires quire_accumulable<NumberType, capacity, LimbType> && std::same_as<Value, NumberType>
void accumulator_add_product(quire<NumberType, capacity, LimbType>& q, const Value& a, const Value& b) {
	q += quire_mul(a, b);
}

/// Round a quire to float or double: to nearest, ties to even, once, including
/// gradual underflow. Reads the full quire, so the bits below the target's
/// precision decide the rounding rather than being dropped.
template<typename Real, typename NumberType, unsigned capacity, typename LimbType>
	requires std::is_floating_point_v<Real> && (std::numeric_limits<Real>::digits <= 64)
Real quire_round_to(const quire<NumberType, capacity, LimbType>& q) {
	using Quire = quire<NumberType, capacity, LimbType>;
	if (q.isnar()) return std::numeric_limits<Real>::quiet_NaN();
	if (q.iszero()) return q.sign() ? -Real(0) : Real(0);

	constexpr int digits = std::numeric_limits<Real>::digits;
	constexpr int emin   = std::numeric_limits<Real>::min_exponent - 1;  // scale of the smallest normal
	constexpr int radix  = static_cast<int>(Quire::radix_point);
	const auto& bits = q.accumulator();
	auto bit = [&](int e) {                       // the quire bit of weight 2^e
		int i = e + radix;
		return i >= 0 && i < static_cast<int>(Quire::qbits) && bits.test(static_cast<unsigned>(i));
	};

	const int s = q.scale();                      // weight of the leading one
	// lowest kept weight: digits bits below the leading one, but no finer than
	// the target's subnormal spacing
	int lsb = (s - digits + 1 > emin - digits + 1) ? s - digits + 1 : emin - digits + 1;

	std::uint64_t m = 0;
	for (int e = s; e >= lsb; --e) m = (m << 1) | (bit(e) ? 1u : 0u);
	const bool guard = bit(lsb - 1);
	bool sticky = false;
	for (int e = lsb - 2; e >= -radix && !sticky; --e) sticky = bit(e);
	if (guard && (sticky || (m & 1u))) {
		// a carry out of all-ones is 2^digits: for float and double that still
		// fits in m and converts exactly, but with a 64-bit significand (x87
		// long double) it would wrap, so renormalize to 2^(digits-1) * 2^(lsb+1)
		if (m == std::numeric_limits<std::uint64_t>::max()) {
			m = std::uint64_t(1) << 63;
			++lsb;
		}
		else {
			++m;
		}
	}

	Real r = std::ldexp(static_cast<Real>(m), lsb);  // exact, or overflows to inf
	return q.sign() ? -r : r;
}

/// The round-out. To the element type: quire_resolve, one rounding. To float or
/// double: quire_round_to, one rounding. Otherwise through the correctly rounded
/// double, which may round twice.
template<typename NumberType, unsigned capacity, typename LimbType, typename Result>
	requires quire_accumulable<NumberType, capacity, LimbType>
Result accumulator_value(const quire<NumberType, capacity, LimbType>& q, std::type_identity<Result>) {
	if constexpr (std::is_same_v<Result, NumberType>) {
		return quire_resolve(q);
	}
	else if constexpr (std::is_same_v<Result, float> || std::is_same_v<Result, double>) {
		return quire_round_to<Result>(q);
	}
	else {
		return static_cast<Result>(quire_round_to<double>(q));
	}
}

/// q += other, exactly: combines the partial quires of a split reduction
template<typename NumberType, unsigned capacity, typename LimbType>
	requires quire_accumulable<NumberType, capacity, LimbType>
void accumulator_merge(quire<NumberType, capacity, LimbType>& q, const quire<NumberType, capacity, LimbType>& other) {
	q += other;
}

}} // namespace sw::universal
