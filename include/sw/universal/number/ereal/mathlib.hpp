#pragma once
// mathlib.hpp: definition of mathematical functions for ereal adaptive-precision arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// These functions are arbitrary-precision approximations computed in ereal's own
// expansion arithmetic (Taylor series / Newton iteration with argument reduction),
// NOT double-precision shims. Accuracy tracks the operand's precision up to ereal's
// architectural ceiling: Shewchuk expansion arithmetic caps maxlimbs at 19 (the last
// limb must stay above DBL_MIN to preserve the non-overlapping property), so ~19*53
// bits ~= 300 decimal digits.
//
// Cross-checked at ereal<19>: exp(1) and pi (via atan) reproduce independently
// generated ~1000-digit constant literals (mpmath: e, pi) to ~290 digits, and
// sqrt/log/sin/erf/gamma/... agree with the efloat multi-digit sibling to the same
// level. This is corroboration between two implementations plus known constants,
// not a proof against a third-party oracle. For unbounded precision use efloat.
// See #582.
//
// (A few loops cast a series term to double only to test its magnitude for loop
// termination; the term itself is computed in full ereal precision.)

// The math functions compute; none of them formats text, so the core is all they
// need (#1334). math/functions/complex.hpp still reaches <complex>, which is
// libstdc++'s <sstream>, so the mathlib as a whole is not I/O-free.
#include <universal/number/ereal/core.hpp>

// High-precision math constants (parsed/derived, not double-truncated -- #1002).
// Included before the function headers so they can reference the shared constants.
#include <universal/number/ereal/math/constants/ereal_constants.hpp>

// Numeric operations
#include <universal/number/ereal/math/functions/numerics.hpp>
#include <universal/number/ereal/math/functions/classify.hpp>

// Low-complexity functions
#include <universal/number/ereal/math/functions/error_and_gamma.hpp>
#include <universal/number/ereal/math/functions/fractional.hpp>
#include <universal/number/ereal/math/functions/hypot.hpp>
#include <universal/number/ereal/math/functions/minmax.hpp>
#include <universal/number/ereal/math/functions/truncate.hpp>
#include <universal/number/ereal/math/functions/next.hpp>

// Medium-complexity functions
#include <universal/number/ereal/math/functions/cbrt.hpp>
#include <universal/number/ereal/math/functions/hyperbolic.hpp>
#include <universal/number/ereal/math/functions/exponent.hpp>
#include <universal/number/ereal/math/functions/logarithm.hpp>
#include <universal/number/ereal/math/functions/pow.hpp>

// High-complexity functions
#include <universal/number/ereal/math/functions/sqrt.hpp>
#include <universal/number/ereal/math/functions/trigonometry.hpp>

// Complex number support (sw::universal::complex<ereal>)
#include <universal/number/ereal/math/functions/complex.hpp>

namespace sw { namespace universal {

	// pown(ereal, int) is defined in ereal_impl.hpp, not here: the core's parse() and
	// to_digits() call it, so a translation unit that includes only core.hpp needs it
	// (#1334). dd keeps its pown in dd_impl.hpp for the same reason.

	// Note: abs() is already defined in ereal_impl.hpp

	// Note: exp(), expm1(), exp2(), exp10() are defined in math/functions/exponent.hpp
	// Note: pow() is defined in math/functions/pow.hpp; pown() in ereal_impl.hpp
	// Note: sinh(), cosh(), tanh(), asinh(), acosh(), atanh() are defined in math/functions/hyperbolic.hpp
	// Note: log(), log2(), log10(), log1p() are defined in math/functions/logarithm.hpp
	// Note: sqrt(), cbrt() are defined in math/functions/sqrt.hpp and cbrt.hpp
	// Note: sin(), cos(), tan(), asin(), acos(), atan(), atan2() are defined in math/functions/trigonometry.hpp
	// Note: floor(), ceil(), trunc(), round(), rint(), nearbyint() are defined in math/functions/truncate.hpp
	// Note: fmod(), remainder(), modf() are defined in math/functions/fractional.hpp
	// Note: min(), max(), fdim() are defined in math/functions/minmax.hpp
	// Note: hypot() is defined in math/functions/hypot.hpp
	// Note: erf(), erfc(), tgamma(), lgamma() are defined in math/functions/error_and_gamma.hpp
	// Note: copysign(), frexp(), ldexp(), scalbn(), logb(), ilogb(), fma() are defined in math/functions/numerics.hpp
	// Note: fpclassify(), isnan(), isinf(), isfinite(), isnormal(), signbit() are defined in math/functions/classify.hpp
	// Note: nextafter(), nexttoward() are defined in math/functions/next.hpp

	// The high-precision implementations above are complete: sqrt/cbrt (Newton),
	// exp/log/expm1/log1p (Taylor with argument reduction), pow (via exp/log),
	// hyperbolic (via exp), and sin/cos/tan/asin/acos/atan/atan2 (Taylor with
	// argument reduction) all compute in ereal expansion arithmetic.
	//
	// Note: complex<ereal> support (is_universal_number + real/imag/conj) is in math/functions/complex.hpp
	//
	// Remaining mathlib work tracked under #582: a per-call precision-request API
	// (e.g. sqrt(x, 200)) remains a possible future enhancement. <cmath> parity is
	// complete (fdim/modf/rint/nearbyint #1165, fma/scalbn/logb/ilogb #1166) and the
	// complex<ereal> binding landed with #1167.

}} // namespace sw::universal
