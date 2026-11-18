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
//   max(src_precision, tgt_precision) <= 52 bits              ->  double      (exact)
//   max(src_precision, tgt_precision) <= long double fraction  ->  long double (80-bit on x86/x87)
//   wider, floating-point Target                               ->  a double expansion of the source
//   otherwise                                                  ->  long double, or double without it
//
// Note: on MSVC and ARM, long double == double (both 64-bit), so those platforms
// get the same precision as the double path for the wider branch. Where LONG_DOUBLE_SUPPORT is 0
// (MSVC among them) the types leave out their long double conversions, so the wider branch uses
// double as well (#2107).
//
// The double expansion (#2133): past the intermediate's precision, universal_cast<qd>(dd) used to
// keep at most 64 bits on every platform. The source is now split into a sum of doubles, each the
// rounded remainder of the previous ones, with every step checked to reconstruct the source exactly,
// and the target accumulates the parts. A target that holds the source's bits receives them all,
// dd -> qd keeps its 106; a narrower target gets each part's rounding, faithful rather than
// correctly rounded. A source the split cannot represent exactly (a residual its arithmetic rounds,
// a value outside double's range or at the top of the source's, a NaN or infinity, a zero with its
// sign, a part the target flushes to zero) takes the long double or double path as before, so no
// conversion loses bits it used to keep. An integer or fixed-point target or source keeps that path
// too: a target would round each part on its own, and a source's conversion from double can saturate.
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
#include <limits>
#include <universal/utility/directives.hpp>   // LONG_DOUBLE_SUPPORT
#include <universal/traits/cross_type_traits.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// detail: precision-aware intermediate cast helpers

namespace detail {

template<typename Float, typename Target, typename Source>
constexpr Target cast_via(const Source& src) noexcept {
	return Target(static_cast<Float>(src));
}

// the fraction bits the long double / double intermediate carries
inline constexpr unsigned intermediate_fraction_bits =
#if LONG_DOUBLE_SUPPORT
	static_cast<unsigned>(std::numeric_limits<long double>::digits - 1);
#else
	52u;
#endif

// the double expansion needs floating-point source arithmetic and a floating-point target that
// accumulates. An integer or fixed-point source is excluded: Source(h) can saturate (integer<64>:
// h = 2^63 for 2^63 - 1 saturates back to 2^63 - 1, and the reconstruction check passes on the wrong
// part), and an unsigned kind's subtraction checks its domain (NaturalNumber 42 - 42 throws).
template<typename Target, typename Source>
concept double_expansion_castable = UniversalFloatingPoint<Target> && UniversalFloatingPoint<Source> &&
	std::numeric_limits<Source>::is_specialized &&
	requires(const Source& s, double d, Target& t) {
		Source(d);
		{ s - s } -> std::convertible_to<Source>;
		{ s + s } -> std::convertible_to<Source>;
		{ s == s } -> std::convertible_to<bool>;
		Target(d);
		t += Target(d);
	};

// src as a sum of doubles, accumulated in Target; false when the split is not exact
template<typename Target, typename Source>
constexpr bool cast_via_double_expansion(const Source& src, Target& out) noexcept {
	constexpr unsigned srcPrec = precision_bits_v<Source>;
	constexpr int maxParts = static_cast<int>(srcPrec / 53u) + 3;
	constexpr double dmax = std::numeric_limits<double>::max();
	// a part at the top of the source's range may saturate in Source(h) and still reconstruct: decline it
	const double sourceTop = static_cast<double>(std::numeric_limits<Source>::max()) * 0.5;
	double parts[maxParts]{};
	int n = 0;
	Source rest = src;
	while (n < maxParts && !rest.iszero()) {
		const double h = static_cast<double>(rest);
		if (!(h >= -dmax && h <= dmax) || h == 0.0) return false;   // NaN, infinity, or underflow
		if (!(h < sourceTop && h > -sourceTop)) return false;        // near the source's max: may saturate
		const Source hs(h);
		const Source next = rest - hs;
		if (!(hs + next == rest)) return false;   // the source's arithmetic rounded the residual
		parts[n++] = h;
		rest = next;
	}
	if (n == 0 || !rest.iszero()) return false;   // a zero keeps its sign on the plain path
	Target t(parts[0]);
	for (int i = 1; i < n; ++i) {
		const Target part(parts[i]);
		if (part.iszero()) return false;   // a nonzero part the target flushes: the plain path keeps it
		t += part;
	}
	out = t;
	return true;
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
	// a zero goes through the intermediate as well, which carries its sign: Target{} turned -0 into +0
	// (#2059). precision_bits_v counts fraction bits, without the hidden bit, so <= 52 is what fits
	// a double's 53-bit significand.
	constexpr unsigned srcPrec = precision_bits_v<Source>;
	constexpr unsigned tgtPrec = precision_bits_v<Target>;
	constexpr unsigned maxPrec = (srcPrec > tgtPrec) ? srcPrec : tgtPrec;

	if constexpr (maxPrec > detail::intermediate_fraction_bits && detail::double_expansion_castable<Target, Source>) {
		// wider than the intermediate: the double expansion, when the split is exact
		Target t{};
		if (detail::cast_via_double_expansion<Target>(src, t)) return t;
	}
	if constexpr (maxPrec <= 52u || !LONG_DOUBLE_SUPPORT) {
		// Both types fit within double's 52-bit significand -- exact. Without long double
		// support the types have no long double conversion to go through.
		return detail::cast_via<double, Target>(src);
	}
	else {
		// Use long double (80-bit extended on x86/x87).
		// On MSVC/ARM where long double == double this is the same precision,
		// but the intent is preserved for platforms that support 80-bit.
#if LONG_DOUBLE_SUPPORT
		return detail::cast_via<long double, Target>(src);
#else
		return detail::cast_via<double, Target>(src);   // not reached: the branch above takes it
#endif
	}
}

////////////////////////////////////////////////////////////////////
// universal_assign: in-place cross-type assignment
//
// Same precision-aware intermediate selection as universal_cast.

template<typename Target, typename Source>
	requires UniversalNumber<Source> && UniversalNumber<Target>
constexpr Target& universal_assign(Target& tgt, const Source& src) noexcept {
	constexpr unsigned srcPrec = precision_bits_v<Source>;
	constexpr unsigned tgtPrec = precision_bits_v<Target>;
	constexpr unsigned maxPrec = (srcPrec > tgtPrec) ? srcPrec : tgtPrec;

	if constexpr (maxPrec > detail::intermediate_fraction_bits && detail::double_expansion_castable<Target, Source>) {
		Target t{};
		if (detail::cast_via_double_expansion<Target>(src, t)) {
			tgt = t;
			return tgt;
		}
	}
	if constexpr (maxPrec <= 52u || !LONG_DOUBLE_SUPPORT) {
		tgt = static_cast<double>(src);
	}
	else {
#if LONG_DOUBLE_SUPPORT
		tgt = static_cast<long double>(src);
#endif
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
