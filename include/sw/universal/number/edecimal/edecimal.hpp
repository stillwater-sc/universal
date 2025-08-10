// adaptive precision decimal integer arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _EDECIMAL_STANDARD_HEADER_
#define _EDECIMAL_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/compiler.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <limits>
#include <regex>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(EDECIMAL_ENABLE_LITERALS)
// default is to enable them
#define EDECIMAL_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(EDECIMAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr as a signalling error
#define EDECIMAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// support functions
#include <universal/native/ieee754.hpp>
#include <universal/string/strmanip.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/edecimal/exceptions.hpp>
#include <universal/number/edecimal/edecimal_fwd.hpp>
#include <universal/number/edecimal/edecimal_impl.hpp>
#include <universal/number/edecimal/numeric_limits.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/edecimal/mathlib.hpp>

#endif
