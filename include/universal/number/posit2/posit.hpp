// <universal/posit/posit.hpp>: arbitrary configuration fixed-size posit standard header
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _POSIT_STANDARD_HEADER_
#define _POSIT_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/native/nonconstexpr754.hpp>  // temporary until we can make posit constexpr

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable error-free posit format I/O
#if !defined(POSIT_ERROR_FREE_IO_FORMAT)
// default is to print double values in human-readable form
#define POSIT_ERROR_FREE_IO_FORMAT 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(POSIT_ENABLE_LITERALS)
// default is to enable them
#define POSIT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for posit arithmetic errors
// left to application to enable
#if !defined(POSIT_THROW_ARITHMETIC_EXCEPTION)
// default is to use NaR as a signalling error
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#else
// for the value<> class assume the same behavior as requested for posits
#define VALUE_THROW_ARITHMETIC_EXCEPTION POSIT_THROW_ARITHMETIC_EXCEPTION
#endif

////////////////////////////////////////////////////////////////////////////////////////
///                         END OF BEHAVIOR SWITCHES                                 ///
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
/// define generic number system traits to be specialized by the posit library
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/posit/exceptions.hpp>
#include <universal/number/posit2/posit_fwd.hpp>
//#include <universal/number/posit/posit_parse.hpp>
#include <universal/number/posit2/posit_impl.hpp>
#include <universal/traits/posit_traits.hpp>
#include <universal/number/posit2/numeric_limits.hpp>

// fast specializations for special posit configurations
// enable fast implementations of the standard posits
// POSIT_FAST_SPECIALIZATION when set will turn on all fast implementations
// Each implementation defines a macros POSIT_FAST_POSIT_`nbits`_`es,
// and includes the fast implementation if set to 1.
// For example, POSIT_FAST_POSIT_8_0, when set to 1, will enable the fast implementation of posit<8,0>.
// The individual POSIT_FAST_### macros enable fine grain control over which configurations
// use fast code.
//#include <universal/number/posit2/specializations.hpp>

////////////////////////////////////////////////////////////////////////////////////////
// useful functions to work with posits
#include <universal/number/posit2/manipulators.hpp>
#include <universal/number/posit2/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// the quire that enables user-controlled rounding
//#include <universal/number/quire/exceptions.hpp>
//#include <universal/number/posit/quire.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// the posit exact dot product
//#include <universal/number/posit2/fdp.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/posit2/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// numerical functions
//#include <universal/number/posit2/twoSum.hpp>


#endif
