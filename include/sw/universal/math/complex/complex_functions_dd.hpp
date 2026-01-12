#pragma once
// complex_functions_dd.hpp: native transcendental functions for complex<dd>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Native implementations of complex transcendental functions for double-double (dd) type.
// These preserve the full ~32 decimal digit precision of dd arithmetic.

#include <universal/number/dd/dd.hpp>
#include <universal/math/complex/complex_impl.hpp>

namespace sw { namespace universal {

// Mark dd as a high-precision type
template<>
struct is_high_precision<dd> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Exponential functions for complex<dd>

/// Complex exponential: exp(a + bi) = exp(a) * (cos(b) + i*sin(b))
/// Native dd implementation preserving full precision
template<>
inline complex<dd> exp(const complex<dd>& c) {
	dd exp_re = exp(c.real());
	dd cos_im = cos(c.imag());
	dd sin_im = sin(c.imag());
	return complex<dd>(exp_re * cos_im, exp_re * sin_im);
}

/// Complex natural logarithm: log(z) = log(|z|) + i*arg(z)
/// Native dd implementation preserving full precision
template<>
inline complex<dd> log(const complex<dd>& c) {
	dd r = c.real();
	dd i = c.imag();
	// |z| = sqrt(r^2 + i^2)
	dd magnitude = sqrt(r * r + i * i);
	// arg(z) = atan2(i, r)
	dd phase = atan2(i, r);
	return complex<dd>(sw::universal::log(magnitude), phase);
}

/// Complex base-10 logarithm: log10(z) = log(z) / log(10)
template<>
inline complex<dd> log10(const complex<dd>& c) {
	complex<dd> ln_c = log(c);
	return complex<dd>(ln_c.real() / dd_log10, ln_c.imag() / dd_log10);
}

////////////////////////////////////////////////////////////////////
// Power functions for complex<dd>

/// Complex square root
/// sqrt(z) = sqrt((|z| + re) / 2) + i * sign(im) * sqrt((|z| - re) / 2)
template<>
inline complex<dd> sqrt(const complex<dd>& c) {
	if (c.imag() == dd(0)) {
		if (c.real() >= dd(0)) {
			return complex<dd>(sw::universal::sqrt(c.real()), dd(0));
		} else {
			return complex<dd>(dd(0), sw::universal::sqrt(-c.real()));
		}
	}

	dd r = c.real();
	dd i = c.imag();
	dd magnitude = sw::universal::sqrt(r * r + i * i);

	dd real_part = sw::universal::sqrt((magnitude + r) / dd(2));
	dd imag_part = sw::universal::sqrt((magnitude - r) / dd(2));

	// Preserve sign of imaginary part
	if (i < dd(0)) {
		imag_part = -imag_part;
	}

	return complex<dd>(real_part, imag_part);
}

/// Complex power: base^exp using exp(exp * log(base))
template<>
inline complex<dd> pow(const complex<dd>& base, const complex<dd>& exponent) {
	if (base.real() == dd(0) && base.imag() == dd(0)) {
		// 0^anything = 0 (for positive real exponent)
		return complex<dd>(dd(0), dd(0));
	}
	return exp(exponent * log(base));
}

////////////////////////////////////////////////////////////////////
// Trigonometric functions for complex<dd>

/// Complex sine: sin(a + bi) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
template<>
inline complex<dd> sin(const complex<dd>& c) {
	dd r = c.real();
	dd i = c.imag();
	return complex<dd>(
		sw::universal::sin(r) * sw::universal::cosh(i),
		sw::universal::cos(r) * sw::universal::sinh(i)
	);
}

/// Complex cosine: cos(a + bi) = cos(a)*cosh(b) - i*sin(a)*sinh(b)
template<>
inline complex<dd> cos(const complex<dd>& c) {
	dd r = c.real();
	dd i = c.imag();
	return complex<dd>(
		sw::universal::cos(r) * sw::universal::cosh(i),
		-sw::universal::sin(r) * sw::universal::sinh(i)
	);
}

/// Complex tangent: tan(z) = sin(z) / cos(z)
template<>
inline complex<dd> tan(const complex<dd>& c) {
	return sin(c) / cos(c);
}

/// Complex arc sine: asin(z) = -i * log(i*z + sqrt(1 - z^2))
template<>
inline complex<dd> asin(const complex<dd>& c) {
	complex<dd> i_unit(dd(0), dd(1));
	complex<dd> one(dd(1), dd(0));
	return -i_unit * log(i_unit * c + sqrt(one - c * c));
}

/// Complex arc cosine: acos(z) = pi/2 - asin(z)
template<>
inline complex<dd> acos(const complex<dd>& c) {
	complex<dd> half_pi(dd_pi2, dd(0));
	return half_pi - asin(c);
}

/// Complex arc tangent: atan(z) = i/2 * log((1-iz)/(1+iz))
template<>
inline complex<dd> atan(const complex<dd>& c) {
	complex<dd> i_unit(dd(0), dd(1));
	complex<dd> one(dd(1), dd(0));
	complex<dd> half_i(dd(0), dd(0.5));
	return half_i * log((one - i_unit * c) / (one + i_unit * c));
}

////////////////////////////////////////////////////////////////////
// Hyperbolic functions for complex<dd>

/// Complex hyperbolic sine: sinh(a + bi) = sinh(a)*cos(b) + i*cosh(a)*sin(b)
template<>
inline complex<dd> sinh(const complex<dd>& c) {
	dd r = c.real();
	dd i = c.imag();
	return complex<dd>(
		sw::universal::sinh(r) * sw::universal::cos(i),
		sw::universal::cosh(r) * sw::universal::sin(i)
	);
}

/// Complex hyperbolic cosine: cosh(a + bi) = cosh(a)*cos(b) + i*sinh(a)*sin(b)
template<>
inline complex<dd> cosh(const complex<dd>& c) {
	dd r = c.real();
	dd i = c.imag();
	return complex<dd>(
		sw::universal::cosh(r) * sw::universal::cos(i),
		sw::universal::sinh(r) * sw::universal::sin(i)
	);
}

/// Complex hyperbolic tangent: tanh(z) = sinh(z) / cosh(z)
template<>
inline complex<dd> tanh(const complex<dd>& c) {
	return sinh(c) / cosh(c);
}

/// Complex inverse hyperbolic sine: asinh(z) = log(z + sqrt(z^2 + 1))
template<>
inline complex<dd> asinh(const complex<dd>& c) {
	complex<dd> one(dd(1), dd(0));
	return log(c + sqrt(c * c + one));
}

/// Complex inverse hyperbolic cosine: acosh(z) = log(z + sqrt(z^2 - 1))
template<>
inline complex<dd> acosh(const complex<dd>& c) {
	complex<dd> one(dd(1), dd(0));
	return log(c + sqrt(c * c - one));
}

/// Complex inverse hyperbolic tangent: atanh(z) = 0.5 * log((1+z)/(1-z))
template<>
inline complex<dd> atanh(const complex<dd>& c) {
	complex<dd> one(dd(1), dd(0));
	complex<dd> half(dd(0.5), dd(0));
	return half * log((one + c) / (one - c));
}

}} // namespace sw::universal
