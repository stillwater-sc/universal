#pragma once
// error_and_gamma.hpp: error and gamma functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded (correctly_rounded.hpp), in every configuration whose range fits the
// quad-double engine; the evaluation in double (shim.hpp) serves the rest.  libm's erfc and
// tgamma are given wider error bounds (8 and 16 ulps) than the other functions, and lgamma's
// is counted in ulps of max(|lgamma|, 1), because libm's lgamma is only accurate in absolute
// terms near its zeros.
//
// erfc is positive everywhere, so it saturates to minpos instead of underflowing.  tgamma has
// poles at 0 and the negative integers (NaR), is nonzero elsewhere, and saturates at both ends,
// with the sign of the gamma function.  lgamma, log|tgamma|, has the same poles.
//
// One limit: near the negative zeros of lgamma (x near -2.457, -2.747, ...) the quad-double
// reflection formula is accurate to about 2^-200 absolute, not relative.  For a 16- or 32-bit
// b-posit that still decides every input; a 64-bit one within about 2^-30 of such a zero may
// round incorrectly.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>
#include <universal/number/bposit/math/truncate.hpp>

namespace sw { namespace universal {

namespace bposit_math {

// a pole of the gamma function: 0 or a negative integer
template<typename B>
bool gamma_pole(const B& x) { return x.iszero() || (x.isneg() && integer_parity(x) >= 0); }

// the sign of Gamma(x) for x < 0, not a pole: negative where floor(x) is odd, that is where
// the integral part of |x| is even
template<typename B>
bool gamma_negative(const B& x) {
	if (!x.isneg()) return false;
	return integer_parity(integral<integral_rule::trunc>(abs(x))) == 0;
}

// lgamma(x) in double, to screen the range: libm's, unless rounding a 64-bit argument to double
// landed it on a pole, where quad-double's lgamma of the exact argument stands in
template<typename B>
double lgamma_estimate(const B& x) {
	const double l = std::lgamma(double(x));
	if (std::isfinite(l)) return l;
	return double(sw::universal::lgamma(to_qd(x)));
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> erf(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar() || x.iszero()) return x;
	if constexpr (!qd_engine<BP>) { return BP(std::erf(double(x))); }
	else {
		return evaluate<E>(x, [](double v) { return std::erf(v); }, [](const qd& xq) { return sw::universal::erf(xq); });
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> erfc(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if constexpr (!qd_engine<BP>) { return bposit_shim::nonzero<BP>(std::erfc(double(x)), false); }
	else {
		const double v = double(x);
		// erfc(v) < e^-v^2 / (v sqrt(pi)): saturate once that is far below minpos
		if (v > 1.0 && below_range<BP>(-v * v * 1.4426950408889634 - std::log2(v * 1.7724538509055159))) return BP(SpecificValue::minpos);
		if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<BP>) {
			if (auto r = from_libm<BP>(std::erfc(v), 8.0)) return *r;
		}
		return from_qd<BP>(sw::universal::erfc(to_qd(x)));
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tgamma(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if (gamma_pole(x)) return BP(SpecificValue::nar);
	const bool negative = gamma_negative(x);
	if constexpr (!qd_engine<BP>) {
		const double d = double(x);
		return bposit_shim::nonzero<BP>(std::tgamma(d), negative);
	}
	else {
		const double l2 = lgamma_estimate(x) * 1.4426950408889634;         // log2 |Gamma(x)|, to screen the range
		if (above_range<BP>(l2)) return BP(negative ? SpecificValue::maxneg : SpecificValue::maxpos);
		if (below_range<BP>(l2)) return BP(negative ? SpecificValue::minneg : SpecificValue::minpos);
		if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<BP>) {
			if (auto r = from_libm<BP>(std::tgamma(double(x)), 16.0)) return *r;
		}
		return from_qd<BP>(sw::universal::tgamma(to_qd(x)));
	}
}

template<engine E, unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> lgamma(const bposit<nbits, rs, es, bt>& x) {
	using BP = bposit<nbits, rs, es, bt>;
	if (x.isnar()) return x;
	if (gamma_pole(x)) return BP(SpecificValue::nar);
	if constexpr (!qd_engine<BP>) { return bposit_shim::finite<BP>(std::lgamma(double(x))); }
	else {
		const double l = lgamma_estimate(x);
		if (above_range<BP>(std::log2(std::fabs(l)))) return BP(l > 0 ? SpecificValue::maxpos : SpecificValue::maxneg);
		if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<BP>) {
			if (auto r = from_libm<BP>(std::lgamma(double(x)), 8.0, 1.0)) return *r;
		}
		return from_qd<BP>(sw::universal::lgamma(to_qd(x)), x.isneg() ? 0x1p-25 : 0.0);
	}
}

} // namespace bposit_math

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> erf(const bposit<nbits, rs, es, bt>& x) { return bposit_math::erf<bposit_math::engine::libm_then_qd>(x); }

// erfc(x) = 1 - erf(x), accurate where it is small
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> erfc(const bposit<nbits, rs, es, bt>& x) { return bposit_math::erfc<bposit_math::engine::libm_then_qd>(x); }

// the gamma function; NaR at its poles, 0 and the negative integers
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tgamma(const bposit<nbits, rs, es, bt>& x) { return bposit_math::tgamma<bposit_math::engine::libm_then_qd>(x); }

// log |Gamma(x)|; NaR at the poles
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> lgamma(const bposit<nbits, rs, es, bt>& x) { return bposit_math::lgamma<bposit_math::engine::libm_then_qd>(x); }

}} // namespace sw::universal
