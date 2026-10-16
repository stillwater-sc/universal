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
//     accumulator_add_product(q, a, b)               q += a * b, exactly
//     accumulator_value(q, std::type_identity<R>{})  round q to R, once
//     accumulator_merge(q, other)                    q += other, exactly
//
// With this header, an MTL5 kernel instantiated as, for example,
//
//     using P = posit<32, 2>;
//     mtl::dot<quire<P>, P>(x, y);
//     sparse_lu_solve<quire<P>>(A, x, b);
//
// accumulates in the quire and rounds once, with no bridge header in between.
// accumulator_merge combines the partial quires of a parallel or distributed
// reduction exactly, so the result doesn't depend on how the terms were split.
//
// The functions apply to quire<NT, capacity, LimbType> whenever NT has an
// ADL-visible quire_mul and quire_resolve, which every Universal number type
// with a quire provides in its fdp.hpp (posit, bposit, cfloat, fixpnt, lns,
// dbns). The element type must be exactly NT. Native float and double quires
// are excluded: their quire_mul takes built-in types, so argument-dependent
// lookup can't find it from here.
//
// Relates to stillwater-sc/mtl5#582

#include <concepts>
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

/// q += a * b, with the product formed exactly
template<typename NumberType, unsigned capacity, typename LimbType, typename Value>
	requires quire_accumulable<NumberType, capacity, LimbType> && std::same_as<Value, NumberType>
void accumulator_add_product(quire<NumberType, capacity, LimbType>& q, const Value& a, const Value& b) {
	q += quire_mul(a, b);
}

/// The single rounding: quire_resolve to the element type, convert_to otherwise
template<typename NumberType, unsigned capacity, typename LimbType, typename Result>
	requires quire_accumulable<NumberType, capacity, LimbType>
Result accumulator_value(const quire<NumberType, capacity, LimbType>& q, std::type_identity<Result>) {
	if constexpr (std::is_same_v<Result, NumberType>) {
		return quire_resolve(q);
	}
	else {
		return q.template convert_to<Result>();
	}
}

/// q += other, exactly: combines the partial quires of a split reduction
template<typename NumberType, unsigned capacity, typename LimbType>
	requires quire_accumulable<NumberType, capacity, LimbType>
void accumulator_merge(quire<NumberType, capacity, LimbType>& q, const quire<NumberType, capacity, LimbType>& other) {
	q += other;
}

}} // namespace sw::universal
