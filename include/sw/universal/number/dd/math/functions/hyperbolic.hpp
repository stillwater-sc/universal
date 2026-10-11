#pragma once
// hyperbolic.hpp: hyperbolic functions of double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Evaluated in the type itself (#1667): they used to go through double, worth 53 bits.  See
// number/shared/multidouble_special.hpp for the forms that keep relative accuracy near 0 and 1.
#include <universal/number/shared/multidouble_special.hpp>

namespace sw { namespace universal {

	inline dd sinh(const dd& x) { return multidouble::sinh(x); }

	inline dd cosh(const dd& x) { return multidouble::cosh(x); }

	inline dd tanh(const dd& x) { return multidouble::tanh(x); }

	inline dd asinh(const dd& x) { return multidouble::asinh(x); }

	inline dd acosh(const dd& x) { return multidouble::acosh(x); }

	inline dd atanh(const dd& x) { return multidouble::atanh(x); }

}} // namespace sw::universal
