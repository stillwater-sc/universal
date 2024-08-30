#pragma once
// lerp.hpp: definition of a linear interpolation function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <type_traits>
#include <universal/traits/posit_traits.hpp>

#if (__cpp_lib_interpolate)

using std::lerp;

#endif

namespace sw { namespace universal {

	template<typename Real>
	Real lerp(Real a, Real b, Real interval) noexcept {
		return a + interval * (b - a);
	}

	template<typename Real>
	Real lerp(Real a, Real b) noexcept {
		return (a + b) * Real(0.5f);
	}

}} // namespace sw::universal

