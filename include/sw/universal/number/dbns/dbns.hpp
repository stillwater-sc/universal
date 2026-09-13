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
///
/// DBNS_ENABLE_LITERALS, DBNS_THROW_ARITHMETIC_EXCEPTION and the
/// QUIRE_THROW_ARITHMETIC_EXCEPTION cascade now default in dbns_impl.hpp, beside the
/// code they govern, so that a translation unit which includes core.hpp directly gets
/// the same defaults this umbrella used to supply (#1334, #1436). Defining any of them
/// before this header still wins, as before.
///
/// DBNS_TRACE_CONVERSION is new: it switches on the (a,b) search trace in
/// convert_ieee754, which used to be a local `constexpr bool bDebug = false` that no
/// caller could reach.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/dbns/core.hpp>

// useful functions to work with logarithmic numbers
// layer 2a: the string producers -- to_binary, to_hex, pretty_print, color_print
#include <universal/number/dbns/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/dbns/iostream.hpp>
// layer 3: introspection -- debugConstexprParameters
#include <universal/number/dbns/debug.hpp>
#include <universal/number/dbns/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
// NOTE: the full mathlib references a math/ subdirectory that does not exist yet, so it
// stays disabled; math_functions.hpp holds the functions that are wired in (e.g. fma).
//#include <universal/number/dbns/mathlib.hpp>
#include <universal/number/dbns/math_functions.hpp>
