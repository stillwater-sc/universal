// <universal/integer/integer>: arbitrary integer arithmetic type standard header
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
///  BEHAVIORAL COMPILATION SWITCHES
//
// INTEGER_ENABLE_LITERALS and INTEGER_THROW_ARITHMETIC_EXCEPTION now default in
// integer_impl.hpp, beside the #if blocks that test them, so core.hpp gets the same
// defaults this umbrella does. A caller can still set either before the include.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/integer/core.hpp>
// the complete blocktriple, so existing callers of integer::normalize() (the quire)
// need no new include. The core only forward-declares it (#1334).
#include <universal/internal/blocktriple/blocktriple.hpp>

/// useful functions to work with integers
#include <universal/number/integer/primes.hpp>
#include <universal/number/integer/sieves.hpp>
// layer 2a: the string producers -- to_binary, to_hex, to_string, color_print
#include <universal/number/integer/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/integer/iostream.hpp>
#include <universal/number/integer/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math library specialized for integer<>
#include <universal/number/integer/mathlib.hpp>
