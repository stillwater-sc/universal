#pragma once
// qd_cascade.hpp: quad-double floating-point arithmetic using floatcascade<4>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(QD_CASCADE_ENABLE_LITERALS)
// default is to enable them
#define QD_CASCADE_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(QD_CASCADE_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define QD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#define QD_CASCADE_EXCEPT noexcept
#else
#if QD_CASCADE_THROW_ARITHMETIC_EXCEPTION
#define QD_CASCADE_EXCEPT
#else
#define QD_CASCADE_EXCEPT noexcept
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
/// INCLUDE FILES that make up the library
///
/// Layered per #1334:
///     core.hpp          arithmetic, no streams
///     manipulators.hpp  type_tag / to_binary / to_triple
///     iostream.hpp      operator<< / operator>>
///
/// A translation unit that only computes can include core.hpp directly and skip the
/// stream headers entirely. to_string() and parse() are in the CORE, not the text
/// layers: assign(const std::string&) calls parse(), and to_string() concatenates a
/// std::string without ever touching a stream.
#include <universal/number/qd_cascade/core.hpp>
#include <universal/traits/qd_cascade_traits.hpp>
#include <universal/number/qd_cascade/manipulators.hpp>
#include <universal/number/qd_cascade/iostream.hpp>
#include <universal/number/qd_cascade/attributes.hpp>

/// the report builders stay at the umbrella: they pull <sstream>/<iomanip> by design
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

#include <universal/number/qd_cascade/math/constants/qd_cascade_constants.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/qd_cascade/mathlib.hpp>
//#include <universal/number/qc_cascade/mathext.hpp>


