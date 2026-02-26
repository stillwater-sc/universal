// IBM System/360 hexadecimal floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/compiler.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(HFLOAT_ENABLE_LITERALS)
#define HFLOAT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
#if !defined(HFLOAT_THROW_ARITHMETIC_EXCEPTION)
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define HFLOAT_EXCEPT noexcept
#else
#if HFLOAT_THROW_ARITHMETIC_EXCEPTION
#define HFLOAT_EXCEPT
#else
#define HFLOAT_EXCEPT noexcept
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/hfloat/exceptions.hpp>
#include <universal/number/hfloat/hfloat_fwd.hpp>
#include <universal/number/hfloat/hfloat_impl.hpp>
#include <universal/traits/hfloat_traits.hpp>
#include <universal/number/hfloat/numeric_limits.hpp>

// useful functions to work with hfloats
#include <universal/number/hfloat/attributes.hpp>
#include <universal/number/hfloat/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/hfloat/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for IBM System/360 hexadecimal floating-point configurations
namespace sw { namespace universal {

// Standard IBM System/360 HFP formats
using hfloat_short    = hfloat<6, 7, uint32_t>;    // 32-bit short precision
using hfloat_long     = hfloat<14, 7, uint32_t>;   // 64-bit long precision
using hfloat_extended = hfloat<28, 7, uint32_t>;   // 128-bit extended precision

}}  // namespace sw::universal
