#pragma once
// manipulators_core.hpp: numeric field extraction for native floating-point types -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The bit-manipulation half of native/manipulators.hpp (#1334), split out for the same
// reason ieee754_core.hpp was split out of ieee754.hpp: sign(), scale(), exponent(),
// fraction(), significand() and the fieldBits() accessors are pure arithmetic on the
// encoding, but they shared a header with to_triple(), to_hex() and color_print(), which
// pull <sstream>, <iomanip>, <ostream> and <istream>.
//
// That matters beyond tidiness: scale() is called from rational, dd, qd, ereal, efloat,
// faithful and floatcascade, so every one of those types dragged four stream headers into
// its arithmetic path to reach a function that does nothing but mask and shift.
//
//     #include <universal/native/manipulators.hpp>        // everything, as before
//     #include <universal/native/manipulators_core.hpp>   // field extraction only
#include <cstdint>       // std::uint16_t / uint32_t / uint64_t
#include <cmath>         // frexpl, on the long double path of scale()
#include <type_traits>   // std::enable_if / std::is_floating_point
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/native/ieee754_parameter.hpp>
#include <universal/native/nonconst_bitcast.hpp>

namespace sw { namespace universal {

	namespace internal {
		// internal function to extract the scale, that is, the de-biased exponent
		//
		// Mask the exponent and the fraction out of the encoding INDEPENDENTLY, each
		// with its own mask. The earlier version cleared only the sign bit and then
		// reused that one variable as both the exponent (after a right shift) and the
		// fraction (captured before the shift), so its "fraction" still carried the
		// exponent bits. That is harmless only for as long as the subnormal branch is
		// entered exclusively for subnormals, whose exponent bits are zero -- it makes
		// the decoder silently wrong the moment that branch is entered for anything
		// else, which is exactly what happened on MSVC (see below).
		//
		// Use sw::bit_cast, and take the constexpr-ness of this function from it via
		// BIT_CAST_CONSTEXPR, rather than the memmove-based BitCast<>. A constexpr
		// function that can never be evaluated in a constant expression -- which is
		// what "constexpr" plus a memmove call adds up to -- is ill-formed, no
		// diagnostic required, and MSVC /O2 (Release only; Debug and RelWithDebInfo
		// were fine) miscompiled the guard so the subnormal correction was applied to
		// every normal value. gcc and clang were unaffected at every -O level. The
		// neighbouring extractFields() in extract_fields.hpp already decodes this way
		// and stayed correct in the same Release binary.
		template<typename Uint, typename Real>
		BIT_CAST_CONSTEXPR int _extractExponent(Real v) noexcept {
			static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
			constexpr Uint emask = static_cast<Uint>(ieee754_parameter<Real>::emask);
			constexpr Uint fmask = static_cast<Uint>(ieee754_parameter<Real>::fmask);
			const Uint bc{ sw::bit_cast<Uint>(v) };
			const Uint rawExponent = static_cast<Uint>((bc & emask) >> ieee754_parameter<Real>::fbits);
			const Uint rawFraction = static_cast<Uint>(bc & fmask);
			// de-bias
			int e = static_cast<int>(rawExponent) - static_cast<int>(ieee754_parameter<Real>::bias);
			if (rawExponent == 0) {  // a subnormal or zero encoding
				int msb = static_cast<int>(find_msb(rawFraction));
				e -= (static_cast<int>(ieee754_parameter<Real>::fbits) - msb);
			}
			return e;
		}

		// internal function to extract fraction bits
        template<typename Uint, typename Real>
        Uint _extractFraction(Real v) noexcept {
	        static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
	        Uint raw{BitCast<Uint>(v)};
	        raw &= ieee754_parameter<Real>::fmask;
	        return raw;
        }

        // internal function to extract significand: TODO: only works for normal numbers
        template<typename Uint, typename Real>
        Uint _extractSignificand(Real v) noexcept {
	        static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
	        Uint raw{BitCast<Uint>(v)};
	        raw &= ieee754_parameter<Real>::fmask;
	        raw |= ieee754_parameter<Real>::hmask;  // add the hidden bit
	        return raw;
        }

	}  // namespace internal


	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
    constexpr bool sign(Real v) noexcept {
		return (v < Real(0.0));
	}

	template<typename Real, typename = typename ::std::enable_if<::std::is_floating_point<Real>::value, Real>::type>
    BIT_CAST_CONSTEXPR int scale(Real v) noexcept {
	    int _e{0};
	    if constexpr (sizeof(Real) == 2) {  // half precision floating-point
		    _e = internal::_extractExponent<std::uint16_t>(v);
	    } else if constexpr (sizeof(Real) == 4) {  // single precision floating-point
		    _e = internal::_extractExponent<std::uint32_t>(v);
	    } else if constexpr (sizeof(Real) == 8) {  // double precision floating-point
		    _e = internal::_extractExponent<std::uint64_t>(v);
	    } else if constexpr (sizeof(Real) == 16) {  // long double precision floating-point
		    // long double frac = frexpl(v, &_e);
		    frexpl(v, &_e);
		    _e -= 1;
	    }
	    return _e;
    }

    template<typename Real, typename = typename ::std::enable_if<::std::is_floating_point<Real>::value, Real>::type>
    BIT_CAST_CONSTEXPR int exponent(Real v) noexcept {
	    return scale(v);
    }

	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real>::type
	>
    unsigned long long fractionBits(Real v) noexcept {
		std::uint64_t _f{ 0 };
		if constexpr (sizeof(Real) == 2) { // half precision floating-point
		    _f = internal::_extractFraction<std::uint16_t>(v);
		}
		else if constexpr (sizeof(Real) == 4) { // single precision floating-point
		    _f = internal::_extractFraction<std::uint32_t>(v);
		}
		else if constexpr (sizeof(Real) == 8) { // double precision floating-point
		    _f = internal::_extractFraction<std::uint64_t>(v);
		}
		else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
			_f = 0;
		}
		return _f;
	}

	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real>::type
	>
    Real fraction(Real v) noexcept {
		Real          r{ 0 };
		std::uint64_t fractionbits{ 0 };
		if constexpr (sizeof(Real) == 2) { // half precision floating-point
		    fractionbits = internal::_extractFraction<std::uint16_t>(v);
		    r            = Real(fractionbits) / Real(1u << 10);
		}
		else if constexpr (sizeof(Real) == 4) { // single precision floating-point
		    fractionbits = internal::_extractFraction<std::uint32_t>(v);
			r            = Real(fractionbits) / Real(1ul << 23);
		}
		else if constexpr (sizeof(Real) == 8) { // double precision floating-point
		    fractionbits = internal::_extractFraction<std::uint64_t>(v);
			r            = Real(fractionbits) / Real(1ull << 52);
		}
		else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
			fractionbits = 0;
		    // long double does not have a standardized bit layout
		}
		return r;
	}

	// extract significand as a floating-point value in [1.0, 2.0): only valid for normal numbers
	template<typename Real, typename = typename ::std::enable_if<::std::is_floating_point<Real>::value, Real>::type>
    Real significand(Real v) noexcept {
	    Real          r{0};
	    std::uint64_t significantbits{0};
	    if constexpr (sizeof(Real) == 2) {  // half precision floating-point
		    significantbits = internal::_extractSignificand<std::uint16_t>(v);
		    r               = Real(significantbits) / Real(1u << 10);
	    }
	    else if constexpr (sizeof(Real) == 4) {  // single precision floating-point
		    significantbits = internal::_extractSignificand<std::uint32_t>(v);
		    r               = Real(significantbits) / Real(1ul << 23);
	    } 
		else if constexpr (sizeof(Real) == 8) {  // double precision floating-point
		    significantbits = internal::_extractSignificand<std::uint64_t>(v);
		    r               = Real(significantbits) / Real(1ull << 52);
	    } 
		else if constexpr (sizeof(Real) == 16) {  // long double precision floating-point
		    significantbits = 0;
		    // long double does not have a standardized bit layout
	    }
	    return r;
    }

	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real>::type
	>
    std::uint64_t significandBits(Real v) noexcept {
	    std::uint64_t significantbits{0};
		if constexpr (sizeof(Real) == 2) { // half precision floating-point
		    significantbits = internal::_extractSignificand<std::uint16_t>(v);
		}
		else if constexpr (sizeof(Real) == 4) { // single precision floating-point
		    significantbits = internal::_extractSignificand<std::uint32_t>(v);
		}
		else if constexpr (sizeof(Real) == 8) { // double precision floating-point
		    significantbits = internal::_extractSignificand<std::uint64_t>(v);
		}
		else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
		    significantbits = 0;
		}
	    return significantbits;
	}

}} // namespace sw::universal
