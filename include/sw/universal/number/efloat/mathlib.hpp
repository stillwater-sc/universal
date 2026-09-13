// mathlib.hpp: master mathematical library include for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
// The math functions compute; none of them formats text, so the core is all they
// need (#1334). math/complex.hpp still reaches <complex>, which is libstdc++'s
// <sstream>, so the mathlib as a whole is not I/O-free.
#include <universal/number/efloat/core.hpp>

#include <universal/number/efloat/math/sqrt.hpp>
#include <universal/number/efloat/math/hypot.hpp>
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/logarithm.hpp>
#include <universal/number/efloat/math/classify.hpp>
#include <universal/number/efloat/math/numerics.hpp>
#include <universal/number/efloat/math/truncate.hpp>
#include <universal/number/efloat/math/constants/efloat_constants.hpp>
#include <universal/number/efloat/math/trigonometry.hpp>
#include <universal/number/efloat/math/hyperbolic.hpp>
#include <universal/number/efloat/math/pow.hpp>
#include <universal/number/efloat/math/fractional.hpp>
#include <universal/number/efloat/math/next.hpp>
#include <universal/number/efloat/math/minmax.hpp>
#include <universal/number/efloat/math/error_and_gamma.hpp>
#include <universal/number/efloat/math/complex.hpp>
