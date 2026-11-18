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
// and the target accumulates the parts with at most one rounding, at the last part, so the result is
// the source rounded once in the target's own rounding. A target that holds the source's bits
// receives them all, dd -> qd keeps its 106. A narrowing cast that would round more than once, as
// qd -> dd usually does, declines to the plain path. A source the split cannot represent exactly (a residual its arithmetic rounds,
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

class dd;
class qd;
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating> class cfloat;

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

// The double expansion is an opt-in for binary floating-point types whose every value is an exact
// sum of doubles and whose conversion from a double in range is exact or correctly rounded: dd, qd
// and cfloat. A structural test admits types where that fails and the checks cannot tell (#2133):
//   - an integer source saturates in Source(h) (integer<64>: 2^63 - 1 splits as 2^63, which saturates
//     back), and an unsigned kind's subtraction is domain checked (NaturalNumber 42 - 42 throws);
//   - a logarithmic source is not a finite sum of doubles: Source(double(lns sqrt(2))) rounds back to
//     the same lns, so the split reports one exact part, sqrt(2) to 53 bits;
//   - hfloat truncates and saturates where double(maxpos) rounds up to 2^60, so a saturated part
//     reads as exact.
template<typename T> struct is_binary_expansion_type : std::false_type {};
template<> struct is_binary_expansion_type<dd> : std::true_type {};
template<> struct is_binary_expansion_type<qd> : std::true_type {};
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
struct is_binary_expansion_type<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>> : std::true_type {};

template<typename Target, typename Source>
concept double_expansion_castable = is_binary_expansion_type<Target>::value && is_binary_expansion_type<Source>::value &&
	std::numeric_limits<Source>::is_specialized && std::numeric_limits<Target>::is_specialized &&
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
	// At most one rounding, at the end: every part converts to the target exactly, and every addition
	// but the last is exact, so the result is the exact sum of the first parts rounded once with the
	// last, the source's value rounded once, in the target's own rounding (to nearest, or hfloat's
	// truncation). Anything else declines: a part that overflows (dd(2^400, -2^340) -> hfloat<6,7>
	// summed maxpos and maxneg to 0) or flushes to 0, or a rounding before the last part (qd(1, 2^-54,
	// -2^-108) -> hfloat<14,7> truncated 1 + 2^-54 to 1, then subtracted, 1 - 2^-56 below the source).
	// a part within a factor of two of the target's max() is declined before it converts: saturation
	// there can round back to h on the way to double
	const double targetTop = static_cast<double>(std::numeric_limits<Target>::max()) * 0.5;
	auto exactPart = [targetTop](double h, Target& part) {
		if (!(h < targetTop && h > -targetTop)) return false;
		part = Target(h);
		return static_cast<double>(part) == h;
	};
	Target t{};
	if (!exactPart(parts[0], t)) return false;
	for (int i = 1; i < n; ++i) {
		Target part{};
		if (!exactPart(parts[i], part)) return false;
		const Target sum = t + part;
		const bool exact = (sum - t == part);   // Fast2Sum: |part| <= |t|, so the difference is exact
		if (!exact && i + 1 < n) return false;   // a rounding with parts still to come
		t = sum;
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
