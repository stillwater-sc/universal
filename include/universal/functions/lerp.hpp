#pragma once
// lerp.hpp: definition of a linear interpolation function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <type_traits>
#include <universal/traits/posit_traits.hpp>

namespace sw {
namespace function {

#if (__cplusplus < 202002L)

	template<typename Real>
	Real lerp(Real a, Real b, Real interval) noexcept {
		return a + interval * (b - a);
	}

	template<typename Real>
	// sw::unum::enable_if_posit<Real> // as return type, when not a posit it would be a void
	Real
	lerp(Real a, Real b) noexcept {
		return (a + b) * Real(0.5);
	}

#else
using std::lerp;
#endif

}  // namespace function
}  // namespace sw

