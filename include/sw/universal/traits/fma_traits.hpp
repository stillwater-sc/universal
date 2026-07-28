#pragma once
// fma_traits.hpp: compile-time capability trait + concept for fused multiply-add
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// has_fma<T> detects whether a type T provides a fused multiply-add reachable by the
// standard ADL two-step -- `using std::fma; fma(a, b, c)`. This resolves true for the
// native floating-point types (via std::fma) and for every Universal number type that
// defines a free fma(a,b,c) in namespace sw::universal (posit, cfloat, fixpnt, integer,
// bfloat16, areal, takum, lns, dbns, dd, qd, efloat, ereal, ...), and false otherwise.
// Detection is by SFINAE, so a type is picked up automatically once its fma lands -- no
// per-type list to maintain.
//
// Generic accumulator / dot-product kernels use this to choose a single-rounding fma
// accumulation path where one is available, and the FusedMultiplyAddable concept
// (C++20) to constrain such kernel templates.
//
// Usage:
//   static_assert(has_fma_v<sw::universal::posit<32,2>>);
//   static_assert(has_fma_v<double>);
//   template<sw::universal::FusedMultiplyAddable Real> Real kernel(...);
//
// Sub-issue of #1189 (universal fma). Relates to #1198. Precedent: quire_traits.hpp.

#include <cmath>          // std::fma for the native types
#include <type_traits>
#include <utility>        // std::declval

namespace sw { namespace universal {

namespace detail {

	// Bring std::fma into scope so the unqualified call below also finds the native
	// overloads; ADL supplies the sw::universal::fma overloads for Universal types.
	using std::fma;

	// Require the result type to be exactly T (a type-PRESERVING fma). This rejects the
	// std::fma-via-implicit-conversion fallback that a merely double-convertible type
	// would otherwise match (that path returns double, not T, and would round-trip
	// through binary64 -- not a single-rounding fma for T).
	template<typename T>
	auto has_fma_probe(int) -> std::enable_if_t<
		std::is_same_v<
			std::decay_t<decltype(fma(std::declval<const T&>(), std::declval<const T&>(), std::declval<const T&>()))>,
			T>,
		std::true_type>;

	template<typename T>
	std::false_type has_fma_probe(...);

} // namespace detail

// has_fma<T>: true iff `fma(a, b, c)` is well-formed for T via the ADL two-step.
template<typename T>
struct has_fma : decltype(detail::has_fma_probe<T>(0)) {};

template<typename T>
inline constexpr bool has_fma_v = has_fma<T>::value;

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
// FusedMultiplyAddable: a type usable in a single-rounding fma accumulation kernel.
template<typename T>
concept FusedMultiplyAddable = has_fma_v<T>;
#endif

}} // namespace sw::universal
