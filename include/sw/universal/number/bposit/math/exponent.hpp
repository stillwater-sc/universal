#pragma once
// exponent.hpp: exponential functions for the bounded posit
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
// e^x, 2^x and 10^x are positive for every real x, so beyond the range they saturate to
// maxpos and minpos, never to NaR or 0.  The screen also keeps the kernels inside double's range.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_math {

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if constexpr (!qd_engine<BP>) { return bposit_shim::nonzero<BP>(std::exp(double(x)), false); }
	else {
		if (above_range<BP>(double(x) * 1.4426950408889634)) return BP(SpecificValue::maxpos);
		if (below_range<BP>(double(x) * 1.4426950408889634)) return BP(SpecificValue::minpos);
		return evaluate<E>(x, [](double v) { return std::exp(v); }, [&](const qd& xq) { (void)xq; return sw::universal::exp(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp2(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if constexpr (!qd_engine<BP>) { return bposit_shim::nonzero<BP>(std::exp2(double(x)), false); }
	else {
		if (above_range<BP>(double(x))) return BP(SpecificValue::maxpos);
		if (below_range<BP>(double(x))) return BP(SpecificValue::minpos);
		return evaluate<E>(x, [](double v) { return std::exp2(v); }, [&](const qd& xq) { (void)xq; return sw::universal::exp(xq * qd_ln2); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp10(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if constexpr (!qd_engine<BP>) { return bposit_shim::nonzero<BP>(std::pow(10.0, double(x)), false); }
	else {
		if (above_range<BP>(double(x) * 3.3219280948873622)) return BP(SpecificValue::maxpos);
		if (below_range<BP>(double(x) * 3.3219280948873622)) return BP(SpecificValue::minpos);
		return evaluate<E>(x, [](double v) { return std::pow(10.0, v); }, [](const qd& xq) { return sw::universal::exp(xq * qd_ln10); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> expm1(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::expm1(double(x))); }
	else {
		if (above_range<BP>(double(x) * 1.4426950408889634)) return BP(SpecificValue::maxpos);
		return evaluate<E>(x, [](double v) { return std::expm1(v); }, [&](const qd& xq) { (void)xq; return expm1_qd(xq); });
	}
}

} // namespace bposit_math

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp(const bposit<nbits, rs, es, bt>& x) { return bposit_math::exp<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp2(const bposit<nbits, rs, es, bt>& x) { return bposit_math::exp2<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp10(const bposit<nbits, rs, es, bt>& x) { return bposit_math::exp10<bposit_math::engine::libm_then_qd>(x); }

// e^x - 1, accurate for small x; it lies in (-1, inf)
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> expm1(const bposit<nbits, rs, es, bt>& x) { return bposit_math::expm1<bposit_math::engine::libm_then_qd>(x); }

}} // namespace sw::universal
