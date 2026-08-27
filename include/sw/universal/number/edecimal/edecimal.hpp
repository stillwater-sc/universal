// adaptive precision decimal integer arithmetic type standard header
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
/// INCLUDE FILES that make up the library
///
/// Layered per #1334. The two switches above must be defined BEFORE core.hpp, which is
/// why they precede it rather than sitting with the other directives at the top.
///
///     core.hpp          arithmetic, no streams
///     manipulators.hpp  to_binary / to_string
///     iostream.hpp      operator<< / operator>>
///
/// A translation unit that only computes can include core.hpp directly and skip the
/// stream headers entirely.
#include <universal/number/edecimal/core.hpp>
#include <universal/number/edecimal/manipulators.hpp>
#include <universal/number/edecimal/iostream.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/edecimal/mathlib.hpp>
