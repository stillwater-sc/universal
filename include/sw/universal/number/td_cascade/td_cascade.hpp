#pragma once
// td_cascade.hpp: Triple-double cascade arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
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
#if !defined(TD_CASCADE_ENABLE_LITERALS)
// default is to enable them
#	define TD_CASCADE_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(TD_CASCADE_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#	define TD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#	define TD_CASCADE_EXCEPT                     noexcept
#else
#	if TD_CASCADE_THROW_ARITHMETIC_EXCEPTION
#		define TD_CASCADE_EXCEPT
#	else
#		define TD_CASCADE_EXCEPT noexcept
#	endif
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/td_cascade/exceptions.hpp>
#include <universal/number/td_cascade/td_cascade_fwd.hpp>
#include <universal/number/td_cascade/td_cascade_impl.hpp>
#include <universal/number/td_cascade/numeric_limits.hpp>
#include <universal/traits/td_cascade_traits.hpp>
// useful functions to work with double-doubles
#include <universal/number/td_cascade/manipulators.hpp>
#include <universal/number/td_cascade/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// mathematical constants library
#include <universal/number/td_cascade/math/constants/td_cascade_constants.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/td_cascade/mathlib.hpp>
// #include <universal/number/dd/mathext.hpp>
