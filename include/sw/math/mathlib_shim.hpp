// <sw/math/mathlib.hpp>: standard header of the stillwater math library shim layer  
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
#ifndef _STILLWATER_MATHLIB_STANDARD_HEADER_
#define _STILLWATER_MATHLIB_STANDARD_HEADER_

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library

///////////////////////////////////////////////////////////////////////////////////////
/// generic versions of the standard math library, shimming into the math stdlib
#include <math/stub/abs.hpp>
#include <math/stub/classify.hpp>
#include <math/stub/complex.hpp>
#include <math/stub/error_and_gamma.hpp>
#include <math/stub/exponent.hpp>
#include <math/stub/fractional.hpp>
#include <math/stub/hyperbolic.hpp>
#include <math/stub/hypot.hpp>
#include <math/stub/logarithm.hpp>
#include <math/stub/minmax.hpp>
#include <math/stub/next.hpp>
#include <math/stub/pow.hpp>
#include <math/stub/sqrt.hpp>
#include <math/stub/trigonometry.hpp>
#include <math/stub/truncate.hpp>

#endif
