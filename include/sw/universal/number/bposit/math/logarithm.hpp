#pragma once
// logarithm.hpp: logarithms for the bounded posit
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
// The logarithm of 0 is a pole and of a negative value a domain error; both are NaR.  Every
// other result is finite and inside the range of every b-posit.  Near 1, where log cancels,
// the kernel takes log1p of the exact x - 1.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_math {

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::log(double(x))); }
	else {
		if (x.isnar() || x.isneg() || x.iszero()) return BP(SpecificValue::nar);
		return evaluate<E>(x, [](double v) { return std::log(v); }, [&](const qd& xq) { (void)xq; return log_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log2(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::log2(double(x))); }
	else {
		if (x.isnar() || x.isneg() || x.iszero()) return BP(SpecificValue::nar);
		return evaluate<E>(x, [](double v) { return std::log2(v); }, [&](const qd& xq) { (void)xq; return log_kernel(x) * qd_lge; });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log10(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::log10(double(x))); }
	else {
		if (x.isnar() || x.isneg() || x.iszero()) return BP(SpecificValue::nar);
		return evaluate<E>(x, [](double v) { return std::log10(v); }, [&](const qd& xq) { (void)xq; return log_kernel(x) / qd_ln10; });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log1p(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::log1p(double(x))); }
	else {
		if (x.isnar() || !(BP(-1) < x)) return BP(SpecificValue::nar);
		if (x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::log1p(v); }, [&](const qd& xq) { (void)xq; return log1p_kernel(x); });
	}
}

} // namespace bposit_math

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log(const bposit<nbits, rs, es, bt>& x) { return bposit_math::log<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log2(const bposit<nbits, rs, es, bt>& x) { return bposit_math::log2<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log10(const bposit<nbits, rs, es, bt>& x) { return bposit_math::log10<bposit_math::engine::libm_then_qd>(x); }

// log(1 + x), accurate for small x; NaR at x = -1 and below
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log1p(const bposit<nbits, rs, es, bt>& x) { return bposit_math::log1p<bposit_math::engine::libm_then_qd>(x); }

}} // namespace sw::universal
