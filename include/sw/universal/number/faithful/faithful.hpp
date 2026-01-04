// compensated evaluation, or faithfully rounded, number system type standard header
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
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
#if !defined(FAITHFUL_ENABLE_LITERALS)
// default is to enable them
#define FAITHFUL_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(FAITHFUL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define FAITHFUL_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/faithful/exceptions.hpp>
//#include <universal/number/faithful/faithful_fwd.hpp>
#include <universal/number/faithful/faithful_impl.hpp>
#include <universal/number/faithful/faithful_traits.hpp>
#include <universal/number/faithful/numeric_limits.hpp>

// useful functions to work with faithful numbers
//#include <universal/number/faithful/manipulators.hpp>
//#include <universal/number/faithful/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
// #include <universal/number/faithful/mathlib.hpp>
