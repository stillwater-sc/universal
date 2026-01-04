// arbitrary logarithmic number arithmetic type standard header
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
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
#if !defined(LNS_ENABLE_LITERALS)
// default is to enable them
#define LNS_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(LNS_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#endif
#if !defined(BITBLOCK_THROW_ARITHMETIC_EXCEPTION)
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION LNS_THROW_ARITHMETIC_EXCEPTION
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/lns/exceptions.hpp>
#include <universal/number/lns/lns_fwd.hpp>
#include <universal/number/lns/lns_impl.hpp>
#include <universal/number/lns/lns_traits.hpp>
#include <universal/number/lns/numeric_limits.hpp>

// useful functions to work with logarithmic numbers
#include <universal/number/lns/manipulators.hpp>
#include <universal/number/lns/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/lns/mathlib.hpp>
