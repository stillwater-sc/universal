// <universal/number/interval/interval.hpp>: parameterized interval arithmetic type standard header
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

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for interval arithmetic errors
// left to application to enable
#if !defined(INTERVAL_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define INTERVAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/interval/exceptions.hpp>
#include <universal/number/interval/interval_fwd.hpp>
#include <universal/number/interval/interval_impl.hpp>
#include <universal/number/interval/interval_traits.hpp>
#include <universal/number/interval/numeric_limits.hpp>
#include <universal/number/interval/manipulators.hpp>
