// elastic multi-component floating-point arithmetic type standard header
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
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// EREAL_THROW_ARITHMETIC_EXCEPTION now defaults in ereal_impl.hpp, so that a
/// translation unit which includes core.hpp directly gets the same default this
/// umbrella used to supply (#1334, #1436). Defining it before this header still wins.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/ereal/core.hpp>

// The core reads scale(double) through native/manipulators_core.hpp. The full native
// support -- to_binary/to_hex/color_print on float and double -- used to arrive here
// through ereal_impl.hpp, so the umbrella keeps providing it.
#include <universal/native/ieee754.hpp>

////////////////////////////////////////////////////////////////////////////////////////
// useful functions to work with ereals
#include <universal/number/ereal/attributes.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/ereal/iostream.hpp>
// layer 2a: the string producers -- to_binary, to_triple, to_components, ...
#include <universal/number/ereal/manipulators.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/ereal/mathlib.hpp>
