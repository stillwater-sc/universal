// arbitrary configuration 2-base logarithmic number arithmetic type standard header
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _LNS2B_STANDARD_HEADER_
#define _LNS2B_STANDARD_HEADER_

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
#if !defined(LNS2B_ENABLE_LITERALS)
// default is to enable them
#define LNS2B_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(LNS2B_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define LNS2B_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/lns2b/exceptions.hpp>
#include <universal/number/lns2b/lns2b_fwd.hpp>
#include <universal/number/lns2b/lns2b_impl.hpp>
#include <universal/number/lns2b/lns2b_traits.hpp>
#include <universal/number/lns2b/numeric_limits.hpp>

// useful functions to work with logarithmic numbers
#include <universal/number/lns2b/manipulators.hpp>
#include <universal/number/lns2b/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
//#include <universal/number/lns2b/mathlib.hpp>

#endif // _LNS2B_STANDARD_HEADER_
