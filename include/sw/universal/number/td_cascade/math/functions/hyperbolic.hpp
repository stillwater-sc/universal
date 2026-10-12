#pragma once
// hyperbolic.hpp: hyperbolic functions of triple-double cascade floating-point
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

	inline td_cascade sinh(const td_cascade& x) { return multidouble::sinh(x); }

	inline td_cascade cosh(const td_cascade& x) { return multidouble::cosh(x); }

	inline td_cascade tanh(const td_cascade& x) { return multidouble::tanh(x); }

	inline td_cascade asinh(const td_cascade& x) { return multidouble::asinh(x); }

	inline td_cascade acosh(const td_cascade& x) { return multidouble::acosh(x); }

	inline td_cascade atanh(const td_cascade& x) { return multidouble::atanh(x); }

}} // namespace sw::universal
