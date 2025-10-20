#pragma once
// manipulators.hpp: definition of manipulation functions for native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <universal/utility/find_msb.hpp>
#include <universal/native/ieee754_parameter.hpp>
#include <universal/native/ieee754_type_tag.hpp>
#include <universal/native/nonconst_bitcast.hpp>
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// internal function to extract exponent bits
	namespace internal {
		// internal function to extract exponent
		template<typename Uint, typename Real>
		constexpr int _extractExponent(Real v) noexcept {
			static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
			Uint raw{BitCast<Uint>(v)};
			raw &= static_cast<Uint>(~ieee754_parameter<Real>::smask);
			Uint frac{raw};
			raw >>= ieee754_parameter<Real>::fbits;
			// de-bias
			int e = static_cast<int>(raw) - static_cast<int>(ieee754_parameter<Real>::bias);
			if (raw == 0) {  // a subnormal encoding
				int msb = static_cast<int>(find_msb(frac));
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

        // internal function to extract significand
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
    constexpr int scale(Real v) noexcept {
	    int _e{0};
	    if constexpr (sizeof(Real) == 2) {  // half precision floating-point
		    _e = internal::_extractExponent<std::uint16_t>(v);
	    }
	    if constexpr (sizeof(Real) == 4) {  // single precision floating-point
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
    constexpr int exponent(Real v) noexcept {
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

	// print representations of an IEEE-754 floating-point
	template<typename Real>
    void valueRepresentations(Real value, bool showhex = false) noexcept {
		using namespace sw::universal;
		std::cout << "IEEE-754 type : " << type_tag<Real>() << '\n';

		std::cout << "binary : " << to_binary(value) << '\n';
		std::cout << "triple : " << to_triple(value) << '\n';
		std::cout << "base2  : " << to_base2_scientific(value) << '\n';
		std::cout << "base10 : " << value << '\n';
		std::cout << "color  : " << color_print(value) << '\n';
	    if (showhex) std::cout << "hex    : " << to_hex(value) << '\n';
	}

	// return in triple form (sign, scale, fraction)
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string to_triple(Real number, bool bNibbleMarker = false) noexcept {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, sign, rawExponent, rawFraction, bits);

		// print sign bit
		s << '(' << (sign ? '-' : '+') << ',';

		// exponent 
		// the exponent value used in the arithmetic is the exponent shifted by a bias 
		// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
		// (i.e. for 2^(e - 127) to be one, e must be 127). 
		// Exponents range from -126 to +127 because exponents of -127 (all 0s) and 128 (all 1s) are reserved for special numbers.
		if (rawExponent == 0) {
			s << "denorm, ";
		}
		else if (rawExponent == ieee754_parameter<Real>::eallset) {
			s << "super, ";
		}
		else {
			int scale = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;
			s << std::setw(4) << scale << ", ";
		}

		// print fraction bits
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		s << "0b";
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << ((rawFraction & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}

		s << ')';
		return s.str();
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string to_base2_scientific(Real number) noexcept {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, sign, rawExponent, rawFraction, bits);

		s << (sign == 1 ? "-" : "+") << "1.";
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << ((rawFraction & mask) ? '1' : '0');
			mask >>= 1;
		}
		s << "e2^" << std::showpos << (rawExponent - ieee754_parameter<Real>::bias);

		return s.str();
	}

	// generate a hex formatted string for a native IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string to_hex(Real number) noexcept {
		std::stringstream s;
		s << std::hexfloat << number;
		return s.str();
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>	
	std::string pretty_print(const RealType f) noexcept {
		return std::string("TBD");
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>	
	std::string info_print(const RealType f, int printPrecision = 17) noexcept {
		return std::string("TBD");
	}


	// generate a color coded binary string for a native single/double/long double IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string color_print(Real number, bool nibbleMarker = false) noexcept {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits;
		extractFields(number, sign, rawExponent, rawFraction, bits);

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);

		// print sign bit
		s << red << (sign ? '1' : '0'); // << '.';

		// print exponent bits
		{
			uint64_t mask = (1 << (ieee754_parameter<Real>::ebits - 1));
			for (int i = (ieee754_parameter<Real>::ebits - 1); i >= 0; --i) {
				s << cyan << ((rawExponent & mask) ? '1' : '0');
				if (nibbleMarker && i > 0 && i % 4 == 0) s << cyan << '\'';
				mask >>= 1;
			}
		}

		//	s << '.';

			// print fraction bits
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << magenta << ((rawFraction & mask) ? '1' : '0');
			if (nibbleMarker && i > 0 && i % 4 == 0) s << magenta << '\'';
			mask >>= 1;
		}

		s << def;
		return s.str();
	}

}} // namespace sw::universal
