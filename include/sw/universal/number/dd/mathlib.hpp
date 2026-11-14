#pragma once
// mathlib.hpp: definition of mathematical functions for the double-double floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/dd/math/functions/numerics.hpp>

#include <universal/number/dd/math/functions/classify.hpp>
#include <universal/number/dd/math/functions/exponent.hpp>
#include <universal/number/dd/math/functions/fractional.hpp>
#include <universal/number/dd/math/functions/logarithm.hpp>
#include <universal/number/dd/math/functions/minmax.hpp>
#include <universal/number/dd/math/functions/next.hpp>
#include <universal/number/dd/math/functions/pow.hpp>
#include <universal/number/dd/math/functions/sqrt.hpp>
// hypot after sqrt: its shared implementation calls sqrt
#include <universal/number/dd/math/functions/hypot.hpp>
#include <universal/number/dd/math/functions/trigonometry.hpp>
#include <universal/number/dd/math/functions/truncate.hpp>

#include <universal/number/dd/math/functions/cbrt.hpp>
// last: the shared implementations call exp, log, sqrt and sin
#include <universal/number/dd/math/functions/hyperbolic.hpp>
#include <universal/number/dd/math/functions/error_and_gamma.hpp>

// complex<dd> computes abs, arg and the complex transcendentals in dd arithmetic, not through
// std::complex<double>. The trait is registered with the mathlib, as ereal and efloat register
// theirs, so the path does not depend on whether number/dd/math/complex/complex.hpp is included:
// without it, complex<dd> used to fall back to 53 bits (#2060).
#include <universal/math/complex/complex_traits.hpp>
namespace sw { namespace universal {
template<>
struct is_high_precision<dd> : std::true_type {};
}}
