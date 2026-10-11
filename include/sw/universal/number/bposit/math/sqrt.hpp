#pragma once
// sqrt.hpp: square and cube roots for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded for every input (correctly_rounded.hpp), in every configuration whose
// range fits the quad-double engine, |scale| <= 600; the evaluation in double of phase 1
// (shim.hpp) serves the rest.
//
// A square root is never a midpoint, and an exact one (a perfect square) is decided at once.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_math {

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sqrt(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::sqrt(double(x))); }
	else {
		if (x.isnar() || x.isneg()) return BP(SpecificValue::nar);
		if (x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::sqrt(v); }, [&](const qd& xq) { (void)xq; return sw::universal::sqrt(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cbrt(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::cbrt(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::cbrt(v); }, [&](const qd& xq) { (void)xq; return cbrt_qd(xq); });
	}
}

} // namespace bposit_math

// the square root; NaR for a negative argument
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sqrt(const bposit<nbits, rs, es, bt>& x) { return bposit_math::sqrt<bposit_math::engine::libm_then_qd>(x); }

// the cube root, defined for every real
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cbrt(const bposit<nbits, rs, es, bt>& x) { return bposit_math::cbrt<bposit_math::engine::libm_then_qd>(x); }

}} // namespace sw::universal
