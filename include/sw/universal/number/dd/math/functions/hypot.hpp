#pragma once
// hypot.hpp: hypot support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/shared/multidouble_exact.hpp>

namespace sw { namespace universal {

	// sqrt(x^2 + y^2) at full precision, scaled so that neither square overflows (#1667)
	inline dd hypot(const dd& x, const dd& y) { return multidouble::hypot(x, y); }

}} // namespace sw::universal
