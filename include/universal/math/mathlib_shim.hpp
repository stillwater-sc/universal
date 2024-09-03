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
/// generic versions of the standard math library, shimming into the math stdlib
#include <universal/math/stub/abs.hpp>
#include <universal/math/stub/classify.hpp>
#include <universal/math/stub/complex.hpp>
#include <universal/math/stub/error_and_gamma.hpp>
#include <universal/math/stub/exponent.hpp>
#include <universal/math/stub/fractional.hpp>
#include <universal/math/stub/hyperbolic.hpp>
#include <universal/math/stub/hypot.hpp>
#include <universal/math/stub/logarithm.hpp>
#include <universal/math/stub/minmax.hpp>
#include <universal/math/stub/next.hpp>
#include <universal/math/stub/pow.hpp>
#include <universal/math/stub/sqrt.hpp>
#include <universal/math/stub/trigonometry.hpp>
#include <universal/math/stub/truncate.hpp>

#endif
