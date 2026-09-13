// bfloat16 arithmetic type standard header
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
/// required std libraries 
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// BFLOAT_ENABLE_LITERALS and BFLOAT_THROW_ARITHMETIC_EXCEPTION now default in
/// bfloat16_impl.hpp, beside the code they govern, so that a translation unit which
/// includes core.hpp directly gets the same defaults this umbrella used to supply
/// (#1334, #1436). Defining either before this header still wins, as before.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/bfloat16/core.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with bfloats
// layer 2a: the string producers -- parse, to_binary, to_hex, to_triple, color_print
#include <universal/number/bfloat16/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/bfloat16/iostream.hpp>
#include <universal/number/bfloat16/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/bfloat16/mathlib.hpp>
