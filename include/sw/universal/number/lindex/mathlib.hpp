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
// trigonometric, hyperbolic, error and gamma functions, fmod and remainder are not here yet;
// see #1715.
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
