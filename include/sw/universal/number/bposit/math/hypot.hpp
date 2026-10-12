#pragma once
// hypot.hpp: hypotenuse for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded for every input (correctly_rounded.hpp), in every configuration whose
// range fits the quad-double engine, |scale| <= 600; the evaluation in double of phase 1
// (shim.hpp) serves the rest.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_math {

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> hypot(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar() || y.isnar()) return BP(SpecificValue::nar);
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::hypot(double(x), double(y))); }
	else {
		return evaluate<E>(x, y, [](double a, double b) { return std::hypot(a, b); },
		                   [](const qd& a, const qd& b) { return hypot_qd(a, b); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> hypot(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y, const bposit<nbits, rs, es, bt>& z) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar() || y.isnar() || z.isnar()) return BP(SpecificValue::nar);
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::hypot(double(x), double(y), double(z))); }
	else {
		if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<BP>) {
			if (auto r = from_libm<BP>(std::hypot(double(x), double(y), double(z)))) return *r;
		}
		return from_qd<BP>(hypot_qd(to_qd(x), to_qd(y), to_qd(z)));
	}
}

} // namespace bposit_math

// sqrt(x^2 + y^2) without intermediate overflow; NaR if either argument is NaR
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> hypot(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit_math::hypot<bposit_math::engine::libm_then_qd>(x, y); }

// sqrt(x^2 + y^2 + z^2)
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> hypot(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y, const bposit<nbits, rs, es, bt>& z) { return bposit_math::hypot<bposit_math::engine::libm_then_qd>(x, y, z); }

}} // namespace sw::universal
