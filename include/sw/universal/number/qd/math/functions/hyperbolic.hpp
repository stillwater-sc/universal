#pragma once
// hyperbolic.hpp: hyperbolic functions of quad-double floating-point
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

	inline qd sinh(const qd& x) { return multidouble::sinh(x); }

	inline qd cosh(const qd& x) { return multidouble::cosh(x); }

	inline qd tanh(const qd& x) { return multidouble::tanh(x); }

	inline qd asinh(const qd& x) { return multidouble::asinh(x); }

	inline qd acosh(const qd& x) { return multidouble::acosh(x); }

	inline qd atanh(const qd& x) { return multidouble::atanh(x); }

	// both at once, as the former native implementation offered
	inline void sincosh(const qd& a, qd& s, qd& c) { s = multidouble::sinh(a); c = multidouble::cosh(a); }

}} // namespace sw::universal
