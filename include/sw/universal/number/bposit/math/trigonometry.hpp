#pragma once
// trigonometry.hpp: trigonometric functions for the bounded posit
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
// Angles are in radians.  sin, cos and tan reduce their argument exactly modulo pi/2
// (reduce_pio2.hpp), so they are correct up to maxpos.  asin and acos are NaR outside
// [-1, 1].  tan is finite at every b-posit: none is an odd multiple of pi/2.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_math {

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sin(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::sin(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::sin(v); }, [&](const qd& xq) { (void)xq; return sin_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cos(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::cos(double(x))); }
	else {
		if (x.isnar()) return x;
		return evaluate<E>(x, [](double v) { return std::cos(v); }, [&](const qd& xq) { (void)xq; return cos_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tan(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::tan(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::tan(v); }, [&](const qd& xq) { (void)xq; return tan_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> asin(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::asin(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		if (abs(x) > BP(1)) return BP(SpecificValue::nar);
		if (abs(x) == BP(1)) return from_qd<BP>(x.isneg() ? -qd_pi_2 : qd_pi_2);
		return evaluate<E>(x, [](double v) { return std::asin(v); }, [&](const qd& xq) { (void)xq; return asin_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> acos(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::acos(double(x))); }
	else {
		if (x.isnar()) return x;
		if (abs(x) > BP(1)) return BP(SpecificValue::nar);
		if (x == BP(1)) return BP(0);
		if (x == BP(-1)) return from_qd<BP>(qd_pi);
		return evaluate<E>(x, [](double v) { return std::acos(v); }, [&](const qd& xq) { (void)xq; return acos_kernel(x); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atan(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if constexpr (!qd_engine<BP>) { return BP(std::atan(double(x))); }
	else {
		if (x.isnar() || x.iszero()) return x;
		return evaluate<E>(x, [](double v) { return std::atan(v); }, [&](const qd& xq) { (void)xq; return sw::universal::atan(xq); });
	}
}

// the angle of the point (x, y), in [-pi, pi]; atan2(0, 0) is 0, as in libm
template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atan2(const bposit<nbits, rs, es, bt>& y, const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar() || y.isnar()) return BP(SpecificValue::nar);
	if constexpr (!qd_engine<BP>) { return BP(std::atan2(double(y), double(x))); }
	else {
		if (y.iszero()) return x.isneg() ? from_qd<BP>(qd_pi) : BP(0);
		if (x.iszero()) return from_qd<BP>(y.isneg() ? -qd_pi_2 : qd_pi_2);
		if (x.ispos() && y.scale() - x.scale() < -86) return atan2_tiny(y, x);       // |y / x| < 2^-85
		return evaluate<E>(y, x, [](double a, double b) { return std::atan2(a, b); },
		                   [](const qd& a, const qd& b) { return sw::universal::atan2(a, b); });
	}
}

} // namespace bposit_math

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sin(const bposit<nbits, rs, es, bt>& x) { return bposit_math::sin<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cos(const bposit<nbits, rs, es, bt>& x) { return bposit_math::cos<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tan(const bposit<nbits, rs, es, bt>& x) { return bposit_math::tan<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> asin(const bposit<nbits, rs, es, bt>& x) { return bposit_math::asin<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> acos(const bposit<nbits, rs, es, bt>& x) { return bposit_math::acos<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atan(const bposit<nbits, rs, es, bt>& x) { return bposit_math::atan<bposit_math::engine::libm_then_qd>(x); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atan2(const bposit<nbits, rs, es, bt>& y, const bposit<nbits, rs, es, bt>& x) { return bposit_math::atan2<bposit_math::engine::libm_then_qd>(y, x); }

}} // namespace sw::universal
