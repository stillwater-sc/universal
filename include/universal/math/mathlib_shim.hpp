// <universal/math/mathlib.hpp>: standard header of the universal math library shim layer  
//   which injects the standard math library functions for native IEEE-754 types into 
//   the sw::universal namespace.
//   
//   For example, std::abs(float) is shimmed to 
//   template<typename NativeFloat> sw::universal::abs(NativeFloat)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef _UNIVERSAL_MATHLIB_STANDARD_HEADER_
#define _UNIVERSAL_MATHLIB_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/math/math_functions.hpp>

#endif
