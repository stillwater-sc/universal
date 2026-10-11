#pragma once
// trigonometry.hpp: trigonometric functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
//
// Angles are in radians.  asin and acos are NaR outside [-1, 1].
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sin(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::sin(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cos(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::cos(double(x))); }

// tan is finite at every b-posit: no b-posit is an odd multiple of pi/2
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tan(const bposit<nbits, rs, es, bt>& x) { return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::tan(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> asin(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::asin(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> acos(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::acos(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atan(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::atan(double(x))); }

// the angle of the point (x, y), in [-pi, pi]; atan2(0, 0) is 0, as in libm
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atan2(const bposit<nbits, rs, es, bt>& y, const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar() || y.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	return bposit<nbits, rs, es, bt>(std::atan2(double(y), double(x)));
}

}} // namespace sw::universal
