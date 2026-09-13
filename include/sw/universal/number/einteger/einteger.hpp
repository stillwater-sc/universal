// einteger arithmetic type standard header
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
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
///
/// EINTEGER_ENABLE_LITERALS and EINTEGER_THROW_ARITHMETIC_EXCEPTION now default in
/// einteger_impl.hpp, beside the code they govern, so that a translation unit which
/// includes core.hpp directly gets the same defaults this umbrella used to supply
/// (#1334, #1436). Defining either before this header still wins, as before.

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/einteger/core.hpp>

// The core reads IEEE-754 fields through native/ieee754_core.hpp. The full native
// support -- to_binary/to_hex/color_print on float and double -- used to arrive here
// through einteger_impl.hpp, so the umbrella keeps providing it.
#include <universal/native/ieee754.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with eintegers
// layer 2a: the string producers -- type_tag, to_binary, to_hex
#include <universal/number/einteger/manipulators.hpp>
// layer 2b: the <iostream> half -- convert_to_string, operator<< / operator>>
#include <universal/number/einteger/iostream.hpp>
// layer 3: introspection -- showLimbs, showLimbValues
#include <universal/number/einteger/debug.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
//#include <universal/number/einteger/math_functions.hpp>
