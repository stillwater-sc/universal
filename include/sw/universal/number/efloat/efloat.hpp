// elastic multi-digit floating-point arithmetic type standard header
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
#include <universal/utility/directives.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 
#include <iostream>
#include <iomanip>
#include <vector>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// EFLOAT_ENABLE_LITERALS and EFLOAT_THROW_ARITHMETIC_EXCEPTION now default in
/// efloat_impl.hpp, so that a translation unit which includes core.hpp directly gets
/// the same defaults this umbrella used to supply (#1334, #1436). Defining either
/// before this header still wins, as before.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/efloat/core.hpp>
//#include <universal/number/efloat/numeric_limits.hpp>

// The core reads IEEE-754 fields through native/ieee754_core.hpp and
// native/manipulators_core.hpp. The full native support -- to_binary/to_hex/
// color_print on float and double -- used to arrive here through efloat_impl.hpp,
// so the umbrella keeps providing it.
#include <universal/native/ieee754.hpp>

////////////////////////////////////////////////////////////////////////////////////////
// useful functions to work with efloats
#include <universal/number/efloat/attributes.hpp>
// layer 2a: the string producers -- to_binary, type_tag, components, to_triple, ...
#include <universal/number/efloat/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/efloat/iostream.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/efloat/mathlib.hpp>
