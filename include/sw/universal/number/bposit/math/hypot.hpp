#pragma once
// hypot.hpp: hypotenuse for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

// sqrt(x^2 + y^2) without intermediate overflow; NaR if either argument is NaR
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> hypot(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) {
	if (x.isnar() || y.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::hypot(double(x), double(y)));
}

// sqrt(x^2 + y^2 + z^2)
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> hypot(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y, const bposit<nbits, rs, es, bt>& z) {
	if (x.isnar() || y.isnar() || z.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::hypot(double(x), double(y), double(z)));
}

}} // namespace sw::universal
