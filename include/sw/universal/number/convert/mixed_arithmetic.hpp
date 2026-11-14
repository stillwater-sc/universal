#pragma once
// mixed_arithmetic.hpp: mixed-type arithmetic operators for Universal number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header provides binary arithmetic operators (+, -, *, /) and
// comparison operators (==, !=, <, <=, >, >=) for mixed Universal
// number type expressions.
//
// Promotion rule (promote_t in traits/cross_type_traits.hpp, #2059): the
// result type is the operand type that covers both operands, ranked by
// dynamic range first, then precision, with a tie-break that does not
// depend on operand order: a + b and b + a have the same type.
//
// Usage:
//   #include <universal/number/posit/posit.hpp>
//   #include <universal/number/cfloat/cfloat.hpp>
//   #include <universal/number/convert/mixed_arithmetic.hpp>
//
//   posit<32,2> p(3.14);
//   cfloat<32,8> c(2.71);
//   auto sum = p + c;    // result type: cfloat<32,8> (max_exponent 128 against posit's 120)

#include <type_traits>
#include <concepts>
#include <universal/traits/cross_type_traits.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// Mixed-type arithmetic operators
//
// These are enabled only when:
//   1. Both operands satisfy UniversalNumber
//   2. The operands are different types (same-type ops are handled
//      by each number system's own operators)
//
// The computation is performed in double precision and converted
// to the promoted result type. This is correct for types with
// <= 52 bits of precision and provides a reasonable approximation
// for wider types.

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr promote_t<T1, T2> operator+(const T1& lhs, const T2& rhs) {
	using Result = promote_t<T1, T2>;
	return Result(static_cast<double>(lhs) + static_cast<double>(rhs));
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr promote_t<T1, T2> operator-(const T1& lhs, const T2& rhs) {
	using Result = promote_t<T1, T2>;
	return Result(static_cast<double>(lhs) - static_cast<double>(rhs));
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr promote_t<T1, T2> operator*(const T1& lhs, const T2& rhs) {
	using Result = promote_t<T1, T2>;
	return Result(static_cast<double>(lhs) * static_cast<double>(rhs));
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr promote_t<T1, T2> operator/(const T1& lhs, const T2& rhs) {
	using Result = promote_t<T1, T2>;
	return Result(static_cast<double>(lhs) / static_cast<double>(rhs));
}

////////////////////////////////////////////////////////////////////
// Mixed-type comparison operators
//
// Comparisons are performed in double precision. This provides
// correct ordering for all types with <= 52 bits of precision.

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr bool operator==(const T1& lhs, const T2& rhs) {
	return static_cast<double>(lhs) == static_cast<double>(rhs);
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr bool operator!=(const T1& lhs, const T2& rhs) {
	return static_cast<double>(lhs) != static_cast<double>(rhs);
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr bool operator<(const T1& lhs, const T2& rhs) {
	return static_cast<double>(lhs) < static_cast<double>(rhs);
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr bool operator<=(const T1& lhs, const T2& rhs) {
	return static_cast<double>(lhs) <= static_cast<double>(rhs);
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr bool operator>(const T1& lhs, const T2& rhs) {
	return static_cast<double>(lhs) > static_cast<double>(rhs);
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr bool operator>=(const T1& lhs, const T2& rhs) {
	return static_cast<double>(lhs) >= static_cast<double>(rhs);
}

////////////////////////////////////////////////////////////////////
// Mixed-type compound assignment operators
//
// These convert the RHS to the LHS type, then use the LHS type's
// native compound assignment.

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr T1& operator+=(T1& lhs, const T2& rhs) {
	lhs += T1(static_cast<double>(rhs));
	return lhs;
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr T1& operator-=(T1& lhs, const T2& rhs) {
	lhs -= T1(static_cast<double>(rhs));
	return lhs;
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr T1& operator*=(T1& lhs, const T2& rhs) {
	lhs *= T1(static_cast<double>(rhs));
	return lhs;
}

template<typename T1, typename T2>
	requires UniversalNumber<T1> && UniversalNumber<T2>
	         && (!std::is_same_v<T1, T2>)
constexpr T1& operator/=(T1& lhs, const T2& rhs) {
	lhs /= T1(static_cast<double>(rhs));
	return lhs;
}

}} // namespace sw::universal
