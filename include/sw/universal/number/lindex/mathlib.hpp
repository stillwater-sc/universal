#pragma once
// mathlib.hpp: the lindex math library (#1349 part 5, #1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every function rounds once, to nearest even on the psi grid, through lindex::round_psi:
// it composes level-index operations on unrounded psi points in the working type and
// evaluates again in dd_cascade when a double result lands near a midpoint, as the
// arithmetic does.  exp and log of a value >= 1 are exact steps of one level.
//
// The trigonometric, hyperbolic, error and gamma functions, fmod and remainder (#1715)
// evaluate on the value X where the result is bounded or follows X closely, and in efloat,
// widened until the rounding is decided, where their error grows with X (value_domain.hpp).
#include <universal/number/lindex/math/constants.hpp>
#include <universal/number/lindex/math/classify.hpp>
#include <universal/number/lindex/math/exponent.hpp>
#include <universal/number/lindex/math/hypot.hpp>
#include <universal/number/lindex/math/logarithm.hpp>
#include <universal/number/lindex/math/minmax.hpp>
#include <universal/number/lindex/math/next.hpp>
#include <universal/number/lindex/math/pow.hpp>
#include <universal/number/lindex/math/sqrt.hpp>
#include <universal/number/lindex/math/truncate.hpp>
#include <universal/number/lindex/math/trigonometry.hpp>
#include <universal/number/lindex/math/hyperbolic.hpp>
#include <universal/number/lindex/math/error_and_gamma.hpp>
#include <universal/number/lindex/math/fractional.hpp>
