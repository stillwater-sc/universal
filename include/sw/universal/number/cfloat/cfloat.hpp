// arbitrary configuration classic floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <cstdint>       // the fixed-width integer types
#include <limits>        // std::numeric_limits
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(CFLOAT_ENABLE_LITERALS)
// default is to enable them
#define CFLOAT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(CFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define CFLOAT_EXCEPT noexcept
#else
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
#define CFLOAT_EXCEPT
#else
#define CFLOAT_EXCEPT noexcept
#endif
#endif
// the fused dot product accumulator (quire, via fdp.hpp) must honor the same
// exception policy as the cfloat it accumulates for (#1226)
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
#define QUIRE_THROW_ARITHMETIC_EXCEPTION CFLOAT_THROW_ARITHMETIC_EXCEPTION
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native sqrt implementation
// 
#if !defined(CFLOAT_NATIVE_SQRT)
#define CFLOAT_NATIVE_SQRT 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/cfloat/core.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with cfloats
// layer 2a: the string producers -- to_binary, to_triple, parse, color_print
#include <universal/number/cfloat/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/cfloat/iostream.hpp>
// layer 3: introspection -- constexprClassParameters, ReportCfloatClassParameters
#include <universal/number/cfloat/debug.hpp>
#include <universal/number/cfloat/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/cfloat/mathlib.hpp>
#include <universal/number/cfloat/mathext.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// fused dot product / quire accumulation support (quire_mul), matching posit.hpp
#include <universal/number/cfloat/fdp.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for industry standard floating point configurations
namespace sw { namespace universal {

// IEEE-754
using fp4      = cfloat<  4, 2, uint8_t, true, true, false>;
// IEEE-754 quarter precision floating-point
using quarter  = cfloat<  8, 2, uint8_t, true, false, false>;
using fp8      = quarter;
// IEEE-754 half precision floating-point
using half     = cfloat< 16, 5, uint16_t, true, false, false>;
using fp16     = half;
// IEEE-754 single precision floating-point
using single   = cfloat< 32, 8, uint32_t, true, false, false>;
using fp32     = single;
// IEEE-754 double precision floating-point
using duble    = cfloat< 64, 11, uint32_t, true, false, false>;
using fp64     = duble;
// IEEE-754 extended precision floating-point
using xtndd    = cfloat< 80, 11, uint32_t, true, false, false>;
using fp80     = xtndd;
// IEEE-754 quad (128bit) precision floating-point
using quad     = cfloat<128, 15, uint32_t, true, false, false>;
using fp128    = quad;
// IEEE-754 octo (256bit) precision floating-point
using octo     = cfloat<256, 19, uint32_t, true, false, false>;
using fp256    = octo;

// DL
// Google brain float
using bfloat_t = cfloat<16, 8, std::uint16_t, true, false, false>;
using msfp8    = cfloat<8, 2, std::uint8_t, false, false, false>;
using msfp9    = cfloat<9, 3, std::uint16_t, false, false, false>;
using amd24    = cfloat<24, 8, std::uint32_t, false, false, false>;

// FP8 formats for DL
// By default we enable both subnormals and max-exponent values
// as the number of encodings is severely limited (128 vs 256 samples)
using fp8e2m5  = cfloat<8, 2, std::uint8_t, true, true, false>;
using fp8e3m4  = cfloat<8, 3, std::uint8_t, true, true, false>;
using fp8e4m3  = cfloat<8, 4, std::uint8_t, true, true, false>;
using fp8e5m2  = cfloat<8, 5, std::uint8_t, true, true, false>;

// helpers

// ShowRepresentations prints the different output formats for the Scalar type
// TODO: guard with cfloat trait
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, Scalar f) {
	auto defaultPrecision = ostr.precision(); // save stream state

	constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10; 	// floating-point attribute for printing scientific format

	Scalar v(f); // convert to target cfloat
	ostr << "scientific   : " << std::setprecision(max_digits10) << v << '\n';
	ostr << "triple form  : " << to_triple(v) << '\n';
	ostr << "binary form  : " << to_binary(v, true) << '\n';
	ostr << "color coded  : " << color_print(v) << '\n';

	ostr << std::setprecision(defaultPrecision);
}

}}  // namespace sw::universal
