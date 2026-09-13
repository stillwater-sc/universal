// <universal/number/areal/areal.hpp>: arbitrary real arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS

// compiler specific configuration for long double support
#include <universal/utility/long_double.hpp>
// compiler specific configuration for C++20 bit_cast
#include <universal/utility/bit_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// AREAL_ENABLE_LITERALS and AREAL_THROW_ARITHMETIC_EXCEPTION now default in
/// areal_impl.hpp, beside the code they govern, so that a translation unit which includes
/// core.hpp directly gets the same defaults this umbrella used to supply (#1334, #1436).
/// Defining either before this header still wins, as before.
///
/// TRACE_CONVERSION, which switches on the conversion tracing, has always defaulted in
/// areal_impl.hpp; its <iostream> include now sits inside that guard too.

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/areal/core.hpp>

// layer 2a: the string producers -- to_string, to_binary, to_hex, pretty_print, color_print
#include <universal/number/areal/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/areal/iostream.hpp>
// layer 3: introspection -- constexprClassParameters
#include <universal/number/areal/debug.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/areal/math_functions.hpp>
