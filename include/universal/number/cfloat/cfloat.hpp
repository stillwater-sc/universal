// arbitrary configuration classic floating-point arithmetic standard header
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _CFLOAT_STANDARD_HEADER_
#define _CFLOAT_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS

// compiler specific configuration for long double support
#include <universal/utility/long_double.hpp>
// compiler specific configuration for C++20 bit_cast
#include <universal/utility/bit_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(CFLOAT_ENABLE_LITERALS)
// default is to enable them
#define CFLOAT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(CFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native sqrt implementation
// 
#if !defined(CFLOAT_NATIVE_SQRT)
#define CFLOAT_NATIVE_SQRT 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/cfloat/exceptions.hpp>
#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/traits/cfloat_traits.hpp>
#include <universal/number/cfloat/numeric_limits.hpp>

// useful functions to work with cfloats
#include <universal/number/cfloat/attributes.hpp>
#include <universal/number/cfloat/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/cfloat/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for industry standard floating point configurations
namespace sw { namespace universal {

// IEEE-754
// IEEE-754 quarter precision floating-point
using quarter  = cfloat<  8, 2, uint16_t, true, false, false>;
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
// IEEE-754 quad (128bit) precision floating-point
using quad     = cfloat<128, 15, uint32_t, true, false, false>;
using fp128    = quad;
// IEEE-754 octo (256bit) precision floating-point
using octo     = cfloat<256, 19, uint32_t, true, false, false>;
using fp256    = octo;

// DL
// Google brain float
using bfloat16 = cfloat<16, 8, std::uint16_t, false, false, false>;
using msfp8    = cfloat<8, 2, std::uint8_t, false, false, false>;
using msfp9    = cfloat<9, 3, std::uint16_t, false, false, false>;
using amd24    = cfloat<24, 8, std::uint32_t, false, false, false>;

}}  // namespace sw::universal


#endif
