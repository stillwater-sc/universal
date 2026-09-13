// decimal fixed-point arithmetic type standard header
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
/// DFIXPNT_THROW_ARITHMETIC_EXCEPTION, and its forwarding to the blockdecimal building
/// block's BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION, now live in dfixpnt_impl.hpp, ahead of
/// the blockdecimal include, so that a translation unit which includes core.hpp directly
/// gets the same behavior this umbrella used to supply (#1334, #1436). Defining either
/// before this header still wins, as before.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation unit
// that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/dfixpnt/core.hpp>

// useful functions to work with dfixpnts
#include <universal/number/dfixpnt/attributes.hpp>
// layer 2a: the string producers -- type_tag, type_field, to_binary, to_native, color_print
#include <universal/number/dfixpnt/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/dfixpnt/iostream.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/dfixpnt/mathlib.hpp>
