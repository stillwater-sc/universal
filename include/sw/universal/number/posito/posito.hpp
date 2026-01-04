// posito.hpp: arbitrary configuration fixed-size posit standard header
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
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable error-free posit format I/O
#if !defined(POSITO_ERROR_FREE_IO_FORMAT)
// default is to print double values in human-readable form
#define POSITO_ERROR_FREE_IO_FORMAT 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(POSITO_ENABLE_LITERALS)
// default is to enable them
#define POSITO_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for posit arithmetic errors
// left to application to enable
#if !defined(POSITO_THROW_ARITHMETIC_EXCEPTION)
// default is to use NaR as a signalling error
#define POSITO_THROW_ARITHMETIC_EXCEPTION 0

// this is problematic: both posit and posito rely on value<> and bitblock<>
// so when both arithmetic types are used in the same program
// the first configuration of VALUE and BITBLOCK will stick.
// TODO: the limb-based posit implementation will resolve this
// as it removes the dependency on value and bitblock
#if !defined(VALUE_THROW_ARITHMETIC_EXCEPTION)
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#endif
#if !defined(BITBLOCK_THROW_ARITHMETIC_EXCEPTION)
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#endif
#else
// for the composite value<> class use the posito configuration
// TODO: this is a temporary solution until the limb-based posit implementation is available
#if !defined(VALUE_THROW_ARITHMETIC_EXCEPTION)
#define VALUE_THROW_ARITHMETIC_EXCEPTION POSITO_THROW_ARITHMETIC_EXCEPTION
#endif
#if !defined(BITBLOCK_THROW_ARITHMETIC_EXCEPTION)
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION POSITO_THROW_ARITHMETIC_EXCEPTION
#endif
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
#include <universal/number/posit/posit.hpp>   // some shared functions that do not depend on the posit vs posito separation
#include <universal/number/posito/exceptions.hpp>
#include <universal/number/posito/posito_fwd.hpp>
#include <universal/number/posito/posito_parse.hpp>
#include <universal/number/posito/posito_impl.hpp>
#include <universal/traits/posito_traits.hpp>
#include <universal/number/posito/numeric_limits.hpp>

// useful functions to work with posits
#include <universal/number/posito/manipulators.hpp>
#include <universal/number/posito/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// the posito exact dot product
//#include <universal/number/posito/fdp.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary functions math library
#include <universal/number/posito/mathlib.hpp>
