#pragma once
// hyperbolic.hpp: hyperbolic functions for the bounded posit
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
// sinh and cosh saturate beyond the range.  acosh is NaR below 1, and atanh is NaR outside
// (-1, 1), with poles at +/-1.  Near 0 and 1, where the textbook formulas cancel, the
// kernels use expm1 and log1p forms (qd_kernels.hpp).
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_math {

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sinh(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::sinh(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		if (above_range<BP>(std::fabs(double(x)) * 1.4426950408889634)) return BP(x.isneg() ? SpecificValue::maxneg : SpecificValue::maxpos);
		return evaluate<E>(x, [](double v) { return std::sinh(v); }, [&](const qd& xq) { (void)xq; return sinh_qd(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cosh(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::cosh(double(x))); }
	else {
		if (x.isnar()) return x;
		if (above_range<BP>(std::fabs(double(x)) * 1.4426950408889634)) return BP(SpecificValue::maxpos);
		return evaluate<E>(x, [](double v) { return std::cosh(v); }, [&](const qd& xq) { (void)xq; return cosh_qd(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tanh(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::tanh(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::tanh(v); }, [&](const qd& xq) { (void)xq; return tanh_qd(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> asinh(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::asinh(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::asinh(v); }, [&](const qd& xq) { (void)xq; return asinh_qd(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> acosh(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::acosh(double(x))); }
	else {
		if (x.isnar() || x < BP(1)) return BP(SpecificValue::nar);
		if (x == BP(1)) return BP(0);
		return evaluate<E>(x, [](double v) { return std::acosh(v); }, [&](const qd& xq) { (void)xq; return acosh_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atanh(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::atanh(double(x))); }
	else {
		if (x.isnar() || !(abs(x) < BP(1))) return BP(SpecificValue::nar);
		if (x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::atanh(v); }, [&](const qd& xq) { (void)xq; return atanh_kernel(x); });
	}
}

} // namespace bposit_math

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sinh(const bposit<nbits, rs, es, bt>& x) { return bposit_math::sinh<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cosh(const bposit<nbits, rs, es, bt>& x) { return bposit_math::cosh<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tanh(const bposit<nbits, rs, es, bt>& x) { return bposit_math::tanh<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> asinh(const bposit<nbits, rs, es, bt>& x) { return bposit_math::asinh<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> acosh(const bposit<nbits, rs, es, bt>& x) { return bposit_math::acosh<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atanh(const bposit<nbits, rs, es, bt>& x) { return bposit_math::atanh<bposit_math::engine::libm_then_qd>(x); }

}} // namespace sw::universal
