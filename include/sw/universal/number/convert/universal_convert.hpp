#pragma once
// universal_convert.hpp: cross-type conversion functions for Universal number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header provides universal_cast<Target>(source) -- an explicit,
// type-safe conversion between any two Universal number types.
//
// The conversion uses double as a portable intermediate representation.
// For types with <= 52 bits of significand this is exact or faithfully
// rounded. For wider types a long double path is used when available.
//
// Usage:
//   #include <universal/number/posit/posit.hpp>
//   #include <universal/number/cfloat/cfloat.hpp>
//   #include <universal/number/convert/universal_convert.hpp>
//
//   posit<32,2> p(3.14159);
//   auto c = universal_cast<cfloat<32,8>>(p);   // posit -> cfloat
//   auto q = universal_cast<posit<16,1>>(c);    // cfloat -> posit<16,1>

#include <type_traits>
#include <concepts>
#include <universal/traits/cross_type_traits.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// universal_cast: explicit cross-type conversion
//
// Converts any UniversalNumber source to any UniversalNumber target
// through a double intermediate. This breaks the module cycle
// because neither type needs to know about the other -- both only
// need operator double() and a constructor from double.

template<typename Target, typename Source>
	requires UniversalNumber<Source> && UniversalNumber<Target>
constexpr Target universal_cast(const Source& src) noexcept {
	// Handle zero explicitly for types where double(0) might not
	// round-trip perfectly (e.g., signed zero in some formats)
	if (src.iszero()) {
		return Target{};
	}
	return Target(static_cast<double>(src));
}

////////////////////////////////////////////////////////////////////
// universal_assign: in-place cross-type assignment
//
// Assigns the value of a Universal source to an existing target.
// More efficient than universal_cast when the target already exists.

template<typename Target, typename Source>
	requires UniversalNumber<Source> && UniversalNumber<Target>
constexpr Target& universal_assign(Target& tgt, const Source& src) noexcept {
	if (src.iszero()) {
		tgt = Target{};
	}
	else {
		tgt = static_cast<double>(src);
	}
	return tgt;
}

////////////////////////////////////////////////////////////////////
// convert_through_double: free function for generic conversions
//
// Lower-level conversion that can be used in template contexts
// where the concept constraints might interfere with overload
// resolution.

template<typename Target, typename Source>
constexpr Target convert_through_double(const Source& src) noexcept {
	return Target(static_cast<double>(src));
}

}} // namespace sw::universal
