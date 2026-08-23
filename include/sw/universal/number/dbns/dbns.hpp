// arbitrary configuration double base number system (DBNS) type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
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
#if !defined(DBNS_ENABLE_LITERALS)
// default is to enable them
#define DBNS_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(DBNS_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define DBNS_THROW_ARITHMETIC_EXCEPTION 0
#endif
// the fused dot product accumulator (quire, via fdp.hpp) must honor the same
// exception policy as the dbns it accumulates for (#1226)
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
#define QUIRE_THROW_ARITHMETIC_EXCEPTION DBNS_THROW_ARITHMETIC_EXCEPTION
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/dbns/exceptions.hpp>
#include <universal/number/dbns/dbns_fwd.hpp>
#include <universal/number/dbns/dbns_impl.hpp>
#include <universal/number/dbns/dbns_traits.hpp>
#include <universal/number/dbns/numeric_limits.hpp>

// useful functions to work with logarithmic numbers
#include <universal/number/dbns/manipulators.hpp>
#include <universal/number/dbns/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
// NOTE: the full mathlib references a math/ subdirectory that does not exist yet, so it
// stays disabled; math_functions.hpp holds the functions that are wired in (e.g. fma).
//#include <universal/number/dbns/mathlib.hpp>
#include <universal/number/dbns/math_functions.hpp>
