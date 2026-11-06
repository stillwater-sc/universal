#pragma once
// mathlib.hpp: the math functions of the faithful number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Until #1950 this header was another type's, which included 14 headers from a faithful/math/
// directory that did not exist, so it could not be included and faithful had no math functions.
//
// What faithful has:
//   classify.hpp  isnan, isinf, isfinite, fpclassify, isnormal, isdenorm, iszero, signbit,
//                 copysign, fabs (abs() is defined with the type)
//   minmax.hpp    fmin, fmax
//   sqrt.hpp      sqrt, compensated: about twice the FloatingPointType's precision
//   truncate.hpp  floor, ceil, trunc, round, exact (#1954)
//   fractional.hpp fmod, exact (#1954)
//   exponent.hpp  exp, exp2, exp10, expm1, compensated (#1955)
//   logarithm.hpp log, log2, log10, log1p, compensated (#1955)
//   trigonometry.hpp sin, cos, tan, asin, acos, atan, atan2, compensated, with the exact
//                 Payne-Hanek reduction (#1956)
//   hyperbolic.hpp sinh, cosh, tanh, asinh, acosh, atanh, compensated (#1957)
//   pow.hpp       pow: a few units while |y log x| <= 1, growing linearly beyond (#1957)
//   cbrt.hpp      cbrt, compensated (#1957)
//   hypot.hpp     hypot, without overflow or underflow (#1957)
//
// The compensated functions evaluate in faithful arithmetic, not through the FloatingPointType,
// which would throw away half the pair's precision (#1952). Each is measured against a wider
// reference in numeric/faithful/, and its worst error is in its header.
#include <universal/number/faithful/math/functions/classify.hpp>
#include <universal/number/faithful/math/functions/minmax.hpp>
#include <universal/number/faithful/math/functions/sqrt.hpp>
#include <universal/number/faithful/math/functions/truncate.hpp>
#include <universal/number/faithful/math/functions/fractional.hpp>
#include <universal/number/faithful/math/functions/exponent.hpp>
#include <universal/number/faithful/math/functions/logarithm.hpp>
#include <universal/number/faithful/math/functions/trigonometry.hpp>
#include <universal/number/faithful/math/functions/hyperbolic.hpp>
#include <universal/number/faithful/math/functions/pow.hpp>
#include <universal/number/faithful/math/functions/cbrt.hpp>
#include <universal/number/faithful/math/functions/hypot.hpp>
