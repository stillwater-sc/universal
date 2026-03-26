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
// Conversion precision strategy (selected at compile time via if constexpr):
//
//   max(src_precision, tgt_precision) <= 52 bits  ->  double   (exact)
//   max(src_precision, tgt_precision) >  52 bits  ->  long double (80-bit on x86/x87)
//
// Note: on MSVC and ARM, long double == double (both 64-bit), so those platforms
// get the same precision as the double path for the wider branch.
// Types wider than long double's significand (63 bits on x86) need a proper
// adaptive-precision intermediate (efloat/ereal) or an expression-template layer
// that analyzes the conversion AST and selects the minimum sufficient precision at
// compile time -- see GitHub issue #197 for that future work.
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
// detail: precision-aware intermediate cast helpers

namespace detail {

template<typename Float, typename Target, typename Source>
constexpr Target cast_via(const Source& src) noexcept {
	return Target(static_cast<Float>(src));
}

} // namespace detail

////////////////////////////////////////////////////////////////////
// universal_cast: explicit cross-type conversion
//
// Selects double or long double as the intermediate type based on
// the significand widths of Source and Target, determined at compile
// time via if constexpr. This avoids the double bottleneck for types
// whose precision exceeds double's 52-bit significand.

template<typename Target, typename Source>
	requires UniversalNumber<Source> && UniversalNumber<Target>
constexpr Target universal_cast(const Source& src) noexcept {
	// Zero short-circuit: avoids signed-zero edge cases
	if (src.iszero()) return Target{};

	constexpr unsigned srcPrec = precision_bits_v<Source>;
	constexpr unsigned tgtPrec = precision_bits_v<Target>;
	constexpr unsigned maxPrec = (srcPrec > tgtPrec) ? srcPrec : tgtPrec;

	if constexpr (maxPrec <= 52u) {
		// Both types fit within double's 52-bit significand -- exact.
		return detail::cast_via<double, Target>(src);
	}
	else {
		// Use long double (80-bit extended on x86/x87).
		// On MSVC/ARM where long double == double this is the same precision,
		// but the intent is preserved for platforms that support 80-bit.
		return detail::cast_via<long double, Target>(src);
	}
}

////////////////////////////////////////////////////////////////////
// universal_assign: in-place cross-type assignment
//
// Same precision-aware intermediate selection as universal_cast.

template<typename Target, typename Source>
	requires UniversalNumber<Source> && UniversalNumber<Target>
constexpr Target& universal_assign(Target& tgt, const Source& src) noexcept {
	if (src.iszero()) {
		tgt = Target{};
		return tgt;
	}

	constexpr unsigned srcPrec = precision_bits_v<Source>;
	constexpr unsigned tgtPrec = precision_bits_v<Target>;
	constexpr unsigned maxPrec = (srcPrec > tgtPrec) ? srcPrec : tgtPrec;

	if constexpr (maxPrec <= 52u) {
		tgt = static_cast<double>(src);
	}
	else {
		tgt = static_cast<long double>(src);
	}
	return tgt;
}

////////////////////////////////////////////////////////////////////
// convert_through_double: lower-level fallback for template contexts
// where concept constraints might interfere with overload resolution.
// Prefer universal_cast in new code.

template<typename Target, typename Source>
constexpr Target convert_through_double(const Source& src) noexcept {
	return Target(static_cast<double>(src));
}

}} // namespace sw::universal
