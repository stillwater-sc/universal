// decimal fixed-point arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/compiler.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for dfixpnt arithmetic errors
// left to application to enable
#if !defined(DFIXPNT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/dfixpnt/exceptions.hpp>
#include <universal/number/dfixpnt/dfixpnt_fwd.hpp>
#include <universal/number/dfixpnt/dfixpnt_impl.hpp>
#include <universal/traits/dfixpnt_traits.hpp>
#include <universal/number/dfixpnt/numeric_limits.hpp>

// useful functions to work with dfixpnts
#include <universal/number/dfixpnt/attributes.hpp>
#include <universal/number/dfixpnt/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/dfixpnt/mathlib.hpp>
