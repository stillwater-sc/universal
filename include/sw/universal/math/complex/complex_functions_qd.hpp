#pragma once
// complex_functions_qd.hpp: native transcendental functions for complex<qd>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Native implementations of complex transcendental functions for quad-double (qd) type.
// These preserve the full ~64 decimal digit precision of qd arithmetic.

#include <universal/number/qd/qd.hpp>
#include <universal/math/complex/complex_impl.hpp>

namespace sw { namespace universal {

// Mark qd as a high-precision type
template<>
struct is_high_precision<qd> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Exponential functions for complex<qd>

/// Complex exponential: exp(a + bi) = exp(a) * (cos(b) + i*sin(b))
/// Native qd implementation preserving full precision
template<>
inline complex<qd> exp(const complex<qd>& c) {
	qd exp_re = exp(c.real());
	qd cos_im = cos(c.imag());
	qd sin_im = sin(c.imag());
	return complex<qd>(exp_re * cos_im, exp_re * sin_im);
}

/// Complex natural logarithm: log(z) = log(|z|) + i*arg(z)
/// Native qd implementation preserving full precision
template<>
inline complex<qd> log(const complex<qd>& c) {
	qd r = c.real();
	qd i = c.imag();
	// |z| = sqrt(r^2 + i^2)
	qd magnitude = sqrt(r * r + i * i);
	// arg(z) = atan2(i, r)
	qd phase = atan2(i, r);
	return complex<qd>(sw::universal::log(magnitude), phase);
}

/// Complex base-10 logarithm: log10(z) = log(z) / ln(10)
template<>
inline complex<qd> log10(const complex<qd>& c) {
	complex<qd> ln_c = log(c);
	return complex<qd>(ln_c.real() / qd_ln10, ln_c.imag() / qd_ln10);
}

////////////////////////////////////////////////////////////////////
// Power functions for complex<qd>

/// Complex square root
/// sqrt(z) = sqrt((|z| + re) / 2) + i * sign(im) * sqrt((|z| - re) / 2)
template<>
inline complex<qd> sqrt(const complex<qd>& c) {
	if (c.imag() == qd(0)) {
		if (c.real() >= qd(0)) {
			return complex<qd>(sw::universal::sqrt(c.real()), qd(0));
		} else {
			return complex<qd>(qd(0), sw::universal::sqrt(-c.real()));
		}
	}

	qd r = c.real();
	qd i = c.imag();
	qd magnitude = sw::universal::sqrt(r * r + i * i);

	qd real_part = sw::universal::sqrt((magnitude + r) / qd(2));
	qd imag_part = sw::universal::sqrt((magnitude - r) / qd(2));

	// Preserve sign of imaginary part
	if (i < qd(0)) {
		imag_part = -imag_part;
	}

	return complex<qd>(real_part, imag_part);
}

/// Complex power: base^exp using exp(exp * log(base))
template<>
inline complex<qd> pow(const complex<qd>& base, const complex<qd>& exponent) {
	if (base.real() == qd(0) && base.imag() == qd(0)) {
		// 0^anything = 0 (for positive real exponent)
		return complex<qd>(qd(0), qd(0));
	}
	return exp(exponent * log(base));
}

////////////////////////////////////////////////////////////////////
// Trigonometric functions for complex<qd>

/// Complex sine: sin(a + bi) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
template<>
inline complex<qd> sin(const complex<qd>& c) {
	qd r = c.real();
	qd i = c.imag();
	return complex<qd>(
		sw::universal::sin(r) * sw::universal::cosh(i),
		sw::universal::cos(r) * sw::universal::sinh(i)
	);
}

/// Complex cosine: cos(a + bi) = cos(a)*cosh(b) - i*sin(a)*sinh(b)
template<>
inline complex<qd> cos(const complex<qd>& c) {
	qd r = c.real();
	qd i = c.imag();
	return complex<qd>(
		sw::universal::cos(r) * sw::universal::cosh(i),
		-sw::universal::sin(r) * sw::universal::sinh(i)
	);
}

/// Complex tangent: tan(z) = sin(z) / cos(z)
template<>
inline complex<qd> tan(const complex<qd>& c) {
	return sin(c) / cos(c);
}

/// Complex arc sine: asin(z) = -i * log(i*z + sqrt(1 - z^2))
template<>
inline complex<qd> asin(const complex<qd>& c) {
	complex<qd> i_unit(qd(0), qd(1));
	complex<qd> one(qd(1), qd(0));
	return -i_unit * log(i_unit * c + sqrt(one - c * c));
}

/// Complex arc cosine: acos(z) = pi/2 - asin(z)
template<>
inline complex<qd> acos(const complex<qd>& c) {
	complex<qd> half_pi(qd_pi_2, qd(0));
	return half_pi - asin(c);
}

/// Complex arc tangent: atan(z) = i/2 * log((1-iz)/(1+iz))
template<>
inline complex<qd> atan(const complex<qd>& c) {
	complex<qd> i_unit(qd(0), qd(1));
	complex<qd> one(qd(1), qd(0));
	complex<qd> half_i(qd(0), qd(0.5));
	return half_i * log((one - i_unit * c) / (one + i_unit * c));
}

////////////////////////////////////////////////////////////////////
// Hyperbolic functions for complex<qd>

/// Complex hyperbolic sine: sinh(a + bi) = sinh(a)*cos(b) + i*cosh(a)*sin(b)
template<>
inline complex<qd> sinh(const complex<qd>& c) {
	qd r = c.real();
	qd i = c.imag();
	return complex<qd>(
		sw::universal::sinh(r) * sw::universal::cos(i),
		sw::universal::cosh(r) * sw::universal::sin(i)
	);
}

/// Complex hyperbolic cosine: cosh(a + bi) = cosh(a)*cos(b) + i*sinh(a)*sin(b)
template<>
inline complex<qd> cosh(const complex<qd>& c) {
	qd r = c.real();
	qd i = c.imag();
	return complex<qd>(
		sw::universal::cosh(r) * sw::universal::cos(i),
		sw::universal::sinh(r) * sw::universal::sin(i)
	);
}

/// Complex hyperbolic tangent: tanh(z) = sinh(z) / cosh(z)
template<>
inline complex<qd> tanh(const complex<qd>& c) {
	return sinh(c) / cosh(c);
}

/// Complex inverse hyperbolic sine: asinh(z) = log(z + sqrt(z^2 + 1))
template<>
inline complex<qd> asinh(const complex<qd>& c) {
	complex<qd> one(qd(1), qd(0));
	return log(c + sqrt(c * c + one));
}

/// Complex inverse hyperbolic cosine: acosh(z) = log(z + sqrt(z^2 - 1))
template<>
inline complex<qd> acosh(const complex<qd>& c) {
	complex<qd> one(qd(1), qd(0));
	return log(c + sqrt(c * c - one));
}

/// Complex inverse hyperbolic tangent: atanh(z) = 0.5 * log((1+z)/(1-z))
template<>
inline complex<qd> atanh(const complex<qd>& c) {
	complex<qd> one(qd(1), qd(0));
	complex<qd> half(qd(0.5), qd(0));
	return half * log((one + c) / (one - c));
}

}} // namespace sw::universal
