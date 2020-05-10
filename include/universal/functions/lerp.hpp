#pragma once
// lerp.hpp: definition of a linear interpolation function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <type_traits>

namespace sw {
namespace function {

#if (__cplusplus < 202002L)
	/*
	template<typename Real,
			 typename = std::enable_if_t<std::is_floating_point<Real>::value>>
	Real lerp(Real a, Real b, Real interval) noexcept {
		return a + interval * (b - a);
	}
	*/

	template<typename Real,
		typename = std::enable_if_t<sw::unum::is_posit<Real>::value>>
	Real lerp(Real a, Real b) noexcept {
		return (a + b) * Real(0.5);
	}

#else
using std::lerp;
#endif

}  // namespace function
}  // namespace sw

