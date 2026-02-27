// arbitrary configuration decimal floating-point arithmetic standard header
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
#if !defined(DFLOAT_ENABLE_LITERALS)
// default is to enable them
#define DFLOAT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(DFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DFLOAT_EXCEPT noexcept
#else
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
#define DFLOAT_EXCEPT
#else
#define DFLOAT_EXCEPT noexcept
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native sqrt implementation
//
#if !defined(DFLOAT_NATIVE_SQRT)
#define DFLOAT_NATIVE_SQRT 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/dfloat/exceptions.hpp>
#include <universal/number/dfloat/dfloat_fwd.hpp>
#include <universal/number/dfloat/dfloat_impl.hpp>
#include <universal/traits/dfloat_traits.hpp>
#include <universal/number/dfloat/numeric_limits.hpp>

// useful functions to work with dfloats
#include <universal/number/dfloat/attributes.hpp>
#include <universal/number/dfloat/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/dfloat/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for IEEE 754-2008 decimal floating-point configurations
namespace sw { namespace universal {

// BID encoding (default)
using decimal32  = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
using decimal64  = dfloat<16, 8, DecimalEncoding::BID, uint32_t>;
#ifdef __SIZEOF_INT128__
using decimal128 = dfloat<34, 12, DecimalEncoding::BID, uint32_t>;
#endif

// DPD encoding variants
using decimal32_dpd  = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;
using decimal64_dpd  = dfloat<16, 8, DecimalEncoding::DPD, uint32_t>;
#ifdef __SIZEOF_INT128__
using decimal128_dpd = dfloat<34, 12, DecimalEncoding::DPD, uint32_t>;
#endif

}}  // namespace sw::universal
