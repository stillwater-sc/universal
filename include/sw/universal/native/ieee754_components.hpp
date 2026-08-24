#pragma once
// ieee754_components.hpp: sign/exponent/fraction extraction from native float and double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// THE PATTERN (#1334): ieee754_float.hpp and ieee754_double.hpp are named like
// core headers and are included by everything, but their contents are almost
// entirely TEXT -- to_hex, to_binary, to_triple, to_base2_scientific,
// color_print. Carrying those meant <sstream> and <iomanip> entered the include
// graph of every translation unit that merely decodes a native float.
//
// The one genuinely core function in each was ieee_components(), so that is what
// moved here. This header is 17k preprocessed lines with ZERO stream headers,
// where the two it came from cost ~62k with four.
#include <cstdint>
#include <tuple>
#include <limits>
// architecture.hpp defines the UNIVERSAL_ARCH_* macros that gate the
// long_double_decoder unions in ieee754_decoder.hpp; without it no branch fires
// and ieee_components(long double) cannot see a decoder.
#include <universal/utility/architecture.hpp>
#include <universal/utility/long_double.hpp>   // LONG_DOUBLE_SUPPORT
#include <universal/utility/bit_cast.hpp>
#include <universal/native/ieee754_decoder.hpp>

namespace sw { namespace universal {

// ieee_components returns a tuple of sign, exponent, and fraction
inline std::tuple<bool, int, uint32_t> ieee_components(float fp)
{
	static_assert(std::numeric_limits<float>::is_iec559,
		"This function only works when float complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");

	float_decoder fd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, uint32_t>(
		static_cast<bool>(fd.parts.sign), 
		static_cast<int>(fd.parts.exponent),
		static_cast<uint32_t>(fd.parts.fraction) 
	);
}

// ieee_components returns a tuple of sign, exponent, and fraction
inline std::tuple<bool, int, std::uint64_t> ieee_components(double fp)
{
	static_assert(std::numeric_limits<double>::is_iec559,
		"This function only works when double complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");

	double_decoder dd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, std::uint64_t>(
		static_cast<bool>(dd.parts.sign), 
		static_cast<int>(dd.parts.exponent),
		static_cast<std::uint64_t>(dd.parts.fraction) 
	);
}

#if (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
/* GNU GCC/G++. --------------------------------------------- */

// ieee_components returns a tuple of sign, exponent, and fraction.
inline std::tuple<bool, int, std::uint64_t> ieee_components(long double fp) {
	static_assert(std::numeric_limits<double>::is_iec559,
		"This function only works when double complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(long double) == 16, "This function only works when long double is 16 bytes.");

	long_double_decoder dd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int, std::uint64_t>(
		static_cast<bool>(dd.parts.sign),
		static_cast<int>(dd.parts.exponent),
		static_cast<std::uint64_t>(dd.parts.fraction)
		);
}

#elif defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

// __arm__ which is defined for 32bit arm, and 32bit arm only.
// __aarch64__ which is defined for 64bit arm, and 64bit arm only.

#if defined(UNIVERSAL_ARCH_POWER)
	//////////////////////////////////////////////////////////////////////////////////////////
	///        POWER architecture

#elif defined(UNIVERSAL_ARCH_X86_64)
	//////////////////////////////////////////////////////////////////////////////////////////
	///        X86 architecture

	// ieee_components returns a tuple of sign, exponent, and fraction.
	inline std::tuple<bool, int, std::uint64_t> ieee_components(long double fp) {
		static_assert(std::numeric_limits<long double>::is_iec559,
			"This function only works when long double complies with IEC 559 (IEEE 754)");

		long_double_decoder ld{ fp }; // initializes the first member of the union
		// Reading inactive union parts is forbidden in constexpr :-(
		return std::make_tuple<bool, int, std::uint64_t>(
			static_cast<bool>(ld.parts.sign),
			static_cast<int>(ld.parts.exponent),
			static_cast<std::uint64_t>(ld.parts.fraction)
			);
	}

#else
	//////////////////////////////////////////////////////////////////////////////////////////
	///        not POWER, and not X86
	///        could be ARM or RISC-V

#if __LDBL_MANT_DIG__ == 113
	//////////////////////////////////////////////////////////////////////////////////////////
	///        128-bit IEEE binary128 long double (e.g. Clang targeting Android aarch64)

	// ieee_components returns a tuple of sign, exponent, and fraction.
	inline std::tuple<bool, int, std::uint64_t> ieee_components(long double fp) {
		static_assert(std::numeric_limits<long double>::is_iec559,
			"This function only works when long double complies with IEC 559 (IEEE 754)");

		long_double_decoder ld{ fp }; // initializes the first member of the union
		// Reading inactive union parts is forbidden in constexpr :-(
		return std::make_tuple<bool, int, std::uint64_t>(
			static_cast<bool>(ld.parts.sign),
			static_cast<int>(ld.parts.exponent),
			static_cast<std::uint64_t>(ld.parts.fraction)
			);
	}

#else
	//////////////////////////////////////////////////////////////////////////////////////////
	///        long double == double (e.g. Apple Clang on macOS aarch64)

	// ieee_components returns a tuple of sign, exponent, and fraction
	inline std::tuple<bool, int, std::uint64_t> ieee_components(long double number)	{
		static_assert(std::numeric_limits<long double>::is_iec559,
			"This function only works when double complies with IEC 559 (IEEE 754)");

        double_decoder decoder;
        decoder.d = number; // implicit cast to double
		// Reading inactive union parts is forbidden in constexpr :-(
		return std::make_tuple<bool, int, std::uint64_t>(
			static_cast<bool>(decoder.parts.sign),
			static_cast<int>(decoder.parts.exponent),
			static_cast<std::uint64_t>(decoder.parts.fraction)
		);
	}

#endif
#endif
//---- end of Clang


#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
// Visual C++ compiler is 15.00.20706.01, the _MSC_FULL_VER will be 15002070601

// Visual C++ does not support long double, it is just an alias for double
inline std::tuple<bool, int, std::uint64_t> ieee_components(long double fp) {
	return ieee_components(double(fp));
}


#elif defined(__riscv)
/* RISC-V G++ tool chain */

#else
// unidentified compiler

#endif

// specialization for IEEE long double precision floats
}} // namespace sw::universal
