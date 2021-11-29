#pragma once
// lerp.hpp: definition of a linear interpolation function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <type_traits>
#include <universal/traits/posit_traits.hpp>

namespace sw::universal {

#if (__cplusplus < 202002L)

	template<typename Real>
	Real lerp(Real a, Real b, Real interval) noexcept {
		return a + interval * (b - a);
	}

	template<typename Real>
	Real lerp(Real a, Real b) noexcept {
		return (a + b) * Real(0.5f);
	}

#else
using std::lerp;
#endif

}  // namespace sw::universal

