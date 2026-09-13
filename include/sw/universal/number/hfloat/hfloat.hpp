// IBM System/360 hexadecimal floating-point arithmetic standard header
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
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// HFLOAT_ENABLE_LITERALS, HFLOAT_THROW_ARITHMETIC_EXCEPTION and HFLOAT_EXCEPT now
/// default in hfloat_impl.hpp, beside the code they govern, so that a translation unit
/// which includes core.hpp directly gets the same defaults this umbrella used to supply
/// (#1334, #1436). Defining any of them before this header still wins, as before.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/hfloat/core.hpp>

// useful functions to work with hfloats
// layer 2a: the string producers -- to_binary, to_hex, to_native, type_tag, color_print
#include <universal/number/hfloat/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/hfloat/iostream.hpp>
#include <universal/number/hfloat/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/hfloat/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for IBM System/360 hexadecimal floating-point configurations
namespace sw { namespace universal {

// Standard IBM System/360 HFP formats
using hfp32    = hfloat<6, 7, uint32_t>;    // 32-bit short precision
using hfp64    = hfloat<14, 7, uint32_t>;   // 64-bit long precision
using hfp128   = hfloat<28, 7, uint32_t>;   // extended precision: 1+7+112 = 120 bits, stored in 128 bits

}}  // namespace sw::universal
