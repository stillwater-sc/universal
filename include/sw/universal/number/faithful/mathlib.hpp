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
//
// What it does not have yet (#1952): exp, log, the trigonometric and hyperbolic functions, pow. They
// cannot evaluate through the FloatingPointType, which would throw away half the pair's
// precision, so they need compensated algorithms of their own, as dd's in dd/math/functions.
#include <universal/number/faithful/math/functions/classify.hpp>
#include <universal/number/faithful/math/functions/minmax.hpp>
#include <universal/number/faithful/math/functions/sqrt.hpp>
#include <universal/number/faithful/math/functions/truncate.hpp>
#include <universal/number/faithful/math/functions/fractional.hpp>
