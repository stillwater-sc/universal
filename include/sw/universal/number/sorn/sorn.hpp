// sorn arithmetic type standard header
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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

////////////////////////////////////////////////////////////////////////////////////////
// SORN_ENABLE_LITERALS and SORN_THROW_ARITHMETIC_EXCEPTION now default in sorn_impl.hpp,
// so that a translation unit which includes core.hpp directly gets the same defaults this
// umbrella used to supply (#1334, #1436). Defining either before this header still wins.

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation unit
// that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/sorn/core.hpp>
// layer 2: the text -- type_tag, the getInt/getConfig/getDT members, to_binary,
// color_print, then the two operator<< overloads
#include <universal/number/sorn/sorn_type_tag.hpp>
#include <universal/number/sorn/manipulators.hpp>
#include <universal/number/sorn/iostream.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
// #include <universal/number/sorn/mathlib.hpp>
