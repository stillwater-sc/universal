#pragma once
// constexpr_math.hpp: aggregator for compile-time math functions.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// sw::math::constexpr_math
// -----------------------------------------------------------------------------
// A self-contained library of constexpr math functions for IEEE-754 float and
// double. Designed to support compile-time construction of number systems whose
// encoding requires a transcendental (lns, takum, dbns), but independently
// useful outside Universal as well.
//
// All functions are pure constexpr -- no runtime dispatch, no platform
// intrinsics, no external tooling for coefficient generation. Series truncation
// uses coefficients that are derivable from first principles (e.g., odd
// reciprocals for the artanh series of log2).
//
// Header layout follows the parallel facility include/sw/math/functions/:
//   constexpr_math/detail.hpp -- shared IEEE-754 guards and constants
//   constexpr_math/log2.hpp   -- base-2 logarithm
//   constexpr_math/exp2.hpp   -- base-2 exponential
//   ...future: log.hpp, exp.hpp, pow.hpp, sqrt.hpp
//
// Example:
//   #include <math/constexpr_math.hpp>
//   constexpr double v = sw::math::constexpr_math::log2(8.0);  // == 3.0
//   constexpr double w = sw::math::constexpr_math::exp2(3.0);  // == 8.0

#include <math/constexpr_math/log2.hpp>
#include <math/constexpr_math/exp2.hpp>
