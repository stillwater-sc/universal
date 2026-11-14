#pragma once
// complex_functions.hpp: transcendental functions for complex<T>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Two paths (#2026):
//  - a high-precision T (is_high_precision<T>: ereal, efloat, dd, qd) computes every function
//    in T from T's own real functions, so the result carries T's precision. The formulas are
//    the stable ones: Kahan's for the inverse functions ("Branch Cuts for Complex Elementary
//    Functions", 1987), Kahan's tanh, a hypot-scaled sqrt, and log1p for log near |z| = 1.
//    Branch cuts are the C99 ones, and the special values are those of C99 Annex G, taken
//    case by case so that T's real functions only ever see finite arguments.
//  - any other T goes through std::complex<double>, which is as precise as T or more for the
//    types of 53 bits or fewer.

#include <complex>
#include <cmath>
#include <universal/math/complex/complex_impl.hpp>
#include <universal/math/complex/complex_native_real.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// Helper: convert to std::complex<double> and back
// This is the strategy for the types that are not high precision

namespace detail {

template<ComplexCompatible T>
std::complex<double> to_std_complex(const complex<T>& c) {
	return std::complex<double>(static_cast<double>(c.real()), static_cast<double>(c.imag()));
}

template<ComplexCompatible T>
complex<T> from_std_complex(const std::complex<double>& c) {
	return complex<T>(T(c.real()), T(c.imag()));
}

} // namespace detail

////////////////////////////////////////////////////////////////////
// Native kernels: complex functions computed in T

namespace detail { namespace cx {

// i * z and -i * z
template<typename T> complex<T> times_i(const complex<T>& z) { return complex<T>(-z.imag(), z.real()); }
template<typename T> complex<T> times_minus_i(const complex<T>& z) { return complex<T>(z.imag(), -z.real()); }

// exp(x + iy) = exp(x) (cos y + i sin y)
template<typename T>
complex<T> cexp(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (y == T(0)) return complex<T>(x, y);                  // NaN + i0
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x)) {
		if (!sign_bit(x)) {                                      // +inf
			if (y == T(0)) return complex<T>(x, y);
			if (!is_finite(y)) return complex<T>(x, quiet_nan<T>());
			return complex<T>(x * cos(y), x * odd_sin(y));
		}
		if (!is_finite(y)) return complex<T>(T(0), T(0));     // signs unspecified
		return complex<T>(mul(T(0), cos(y)), mul(T(0), odd_sin(y)));   // +0 cis(y)
	}
	if (!is_finite(y)) return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	if (y == T(0)) return complex<T>(exp(x), y);
	const T e = exp(x);
	return complex<T>(e * cos(y), e * odd_sin(y));
}

// log(z) = log|z| + i arg(z); near |z| = 1, log|z| = log1p((x - 1)(x + 1) + y^2) / 2
template<typename T>
complex<T> clog(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_inf(x) || is_inf(y)) return complex<T>(infinity<T>(), atan2_annexf(y, x));
	if (is_nan(x) || is_nan(y)) return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	const T theta = atan2_annexf(y, x);
	if (x == T(0) && y == T(0)) return complex<T>(-infinity<T>(), theta);
	const T h = hypot_annexf(x, y);
	T re;
	if (h > T(0.5) && h < T(2)) {
		re = log1p((x - T(1)) * (x + T(1)) + y * y) / T(2);
	}
	else {
		re = log(h);
	}
	return complex<T>(re, theta);
}

// principal square root: t = sqrt((|x| + |z|) / 2), the other part y / (2t)
template<typename T>
complex<T> csqrt(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_inf(y)) return complex<T>(infinity<T>(), y);
	if (is_inf(x)) {
		if (!sign_bit(x)) return complex<T>(x, is_nan(y) ? y : with_sign(T(0), y));
		if (is_nan(y)) return complex<T>(y, infinity<T>());   // sign of the imaginary part unspecified
		return complex<T>(T(0), with_sign(infinity<T>(), y));
	}
	if (is_nan(x) || is_nan(y)) return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	if (x == T(0) && y == T(0)) return complex<T>(T(0), y);
	const T h = hypot_annexf(x, y);
	const T t = sqrt(magnitude(x) / T(2) + h / T(2));
	if (!sign_bit(x)) return complex<T>(t, div(y, T(2) * t));
	return complex<T>(magnitude(y) / (T(2) * t), with_sign(t, y));
}

// sinh(x + iy) = sinh x cos y + i cosh x sin y
template<typename T>
complex<T> csinh(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (y == T(0)) return complex<T>(x, y);
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x)) {
		if (y == T(0)) return complex<T>(x, y);
		if (!is_finite(y)) return complex<T>(x, quiet_nan<T>());   // sign of the real part unspecified
		return complex<T>(x * cos(y), infinity<T>() * odd_sin(y));
	}
	if (!is_finite(y)) {
		if (x == T(0)) return complex<T>(x, quiet_nan<T>());       // sign of the zero unspecified
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (y == T(0)) return complex<T>(odd_sinh(x), y);
	return complex<T>(mul(odd_sinh(x), cos(y)), mul(cosh(x), odd_sin(y)));
}

// cosh(x + iy) = cosh x cos y + i sinh x sin y
template<typename T>
complex<T> ccosh(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (y == T(0)) return complex<T>(x, y);                    // sign of the zero unspecified
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x)) {
		if (y == T(0)) return complex<T>(infinity<T>(), mul(with_sign(T(0), x), y));
		if (!is_finite(y)) return complex<T>(infinity<T>(), quiet_nan<T>());
		return complex<T>(infinity<T>() * cos(y), x * odd_sin(y));
	}
	if (!is_finite(y)) {
		if (x == T(0)) return complex<T>(quiet_nan<T>(), T(0));    // sign of the zero unspecified
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (y == T(0)) return complex<T>(cosh(x), mul(with_sign(T(0), x), y));
	return complex<T>(cosh(x) * cos(y), mul(odd_sinh(x), odd_sin(y)));
}

// tanh(x + iy). For |x| <= 20 Kahan's form, which is accurate next to the poles:
//   t = tan y, s = sinh x, beta = 1 + t^2, rho = sqrt(1 + s^2)
//   tanh z = (beta rho s + i t) / (1 + beta s^2)
// Past 20, cosh 2x dwarfs cos 2y, and
//   Re = sign(x) (1 - (exp(-2|x|) + cos 2y) / (cosh 2|x| + cos 2y)), Im = sin 2y / (cosh 2x + cos 2y)
// Past 350 the real part is +-1 to every precision we carry and the imaginary part is +-0.
template<typename T>
complex<T> ctanh(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (y == T(0)) return complex<T>(x, y);
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x)) {
		const T one = with_sign(T(1), x);
		if (!is_finite(y)) return complex<T>(one, T(0));            // sign of the zero unspecified
		return complex<T>(one, with_sign(T(0), mul(odd_sin(y), cos(y))));
	}
	if (!is_finite(y)) {
		if (x == T(0)) return complex<T>(x, quiet_nan<T>());
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	const T ax = magnitude(x);
	if (ax > T(350)) {
		return complex<T>(with_sign(T(1), x), with_sign(T(0), mul(odd_sin(y), cos(y))));
	}
	if (ax > T(20)) {
		const T c2y = cos(T(2) * y);
		const T denom = cosh(T(2) * ax) + c2y;
		const T re = T(1) - (exp(T(-2) * ax) + c2y) / denom;
		return complex<T>(with_sign(re, x), div(odd_sin(mul(T(2), y)), denom));
	}
	const T t = odd_tan(y);
	const T s = odd_sinh(x);
	const T beta = T(1) + t * t;
	const T rho = sqrt(T(1) + s * s);
	const T denom = T(1) + beta * s * s;
	return complex<T>(div(mul(beta * rho, s), denom), div(t, denom));
}

// Kahan's asin for finite z, from s1 = sqrt(1 - z) and s2 = sqrt(1 + z):
//   Re = atan(x / Re(s1 s2)), Im = asinh(Im(conj(s1) s2))
template<typename T>
complex<T> kahan_asin(const complex<T>& z) {
	const complex<T> s1 = csqrt(complex<T>(T(1) - z.real(), -z.imag()));
	const complex<T> s2 = csqrt(complex<T>(T(1) + z.real(), z.imag()));
	const T re = atan2_annexf(z.real(), sub(mul(s1.real(), s2.real()), mul(s1.imag(), s2.imag())));
	const T im = odd_asinh(sub(mul(s1.real(), s2.imag()), mul(s1.imag(), s2.real())));
	return complex<T>(re, im);
}

// asinh(z) = -i asin(iz); the special values are Annex G's for casinh
template<typename T>
complex<T> casinh(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (y == T(0)) return complex<T>(x, y);
		if (is_inf(y)) return complex<T>(infinity<T>(), x);         // sign of the real part unspecified
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_nan(y)) {
		if (is_inf(x)) return complex<T>(x, y);
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x)) {
		return complex<T>(x, with_sign(is_inf(y) ? quarter_pi<T>() : T(0), y));
	}
	if (is_inf(y)) {
		return complex<T>(with_sign(infinity<T>(), x), with_sign(half_pi<T>(), y));
	}
	return times_minus_i(kahan_asin(times_i(z)));
}

// asin(z) = -i asinh(iz)
template<typename T>
complex<T> casin(const complex<T>& z) {
	return times_minus_i(casinh(times_i(z)));
}

// acos(z): Kahan's form for finite z, Re = 2 atan(Re s1 / Re s2), Im = asinh(Im(conj(s2) s1));
// the non-finite cases follow from acos(z) = pi/2 - asin(z)
template<typename T>
complex<T> cacos(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (!is_finite(x) || !is_finite(y)) {
		const complex<T> a = casin(z);
		return complex<T>(sub(half_pi<T>(), a.real()), -a.imag());
	}
	const complex<T> s1 = csqrt(complex<T>(T(1) - x, -y));
	const complex<T> s2 = csqrt(complex<T>(T(1) + x, y));
	const T re = mul(T(2), atan2_annexf(s1.real(), s2.real()));
	const T im = odd_asinh(sub(mul(s2.real(), s1.imag()), mul(s2.imag(), s1.real())));
	return complex<T>(re, im);
}

// acosh(z): Kahan's form for finite z, from s1 = sqrt(z - 1) and s2 = sqrt(z + 1):
//   Re = asinh(Re(conj(s1) s2)), Im = 2 atan(Im s1 / Re s2)
template<typename T>
complex<T> cacosh(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (is_inf(y)) return complex<T>(infinity<T>(), x);
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_nan(y)) {
		if (is_inf(x)) return complex<T>(infinity<T>(), y);
		if (x == T(0)) return complex<T>(quiet_nan<T>(), half_pi<T>());   // as acosh(z) = i acos(z) there
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x) || is_inf(y)) return complex<T>(infinity<T>(), atan2_annexf(y, x));
	const complex<T> s1 = csqrt(complex<T>(x - T(1), y));
	const complex<T> s2 = csqrt(complex<T>(x + T(1), y));
	const T re = odd_asinh(add(mul(s1.real(), s2.real()), mul(s1.imag(), s2.imag())));
	const T im = mul(T(2), atan2_annexf(s1.imag(), s2.real()));
	return complex<T>(re, im);
}

// atanh(z) = log((1 + z) / (1 - z)) / 2, as
//   Re = log1p(4x / ((1 - x)^2 + y^2)) / 4, Im = atan2(2y, (1 - x)(1 + x) - y^2) / 2
template<typename T>
complex<T> catanh(const complex<T>& z) {
	const T x = z.real(), y = z.imag();
	if (is_nan(x)) {
		if (is_inf(y)) return complex<T>(T(0), with_sign(half_pi<T>(), y));   // sign of the zero unspecified
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_nan(y)) {
		if (is_inf(x)) return complex<T>(with_sign(T(0), x), y);
		if (x == T(0)) return complex<T>(x, y);
		return complex<T>(quiet_nan<T>(), quiet_nan<T>());
	}
	if (is_inf(x) || is_inf(y)) return complex<T>(with_sign(T(0), x), with_sign(half_pi<T>(), y));
	// atanh is odd, signed zeros included (C99 G.6.2.3): evaluate x < 0 as -atanh(-z). The real part's
	// log1p(4x / h^2) has an argument near -1 when x is near -1, where it cancels and loses about half
	// the digits; for x > 0 the argument is positive and log1p is well conditioned (#2060)
	if (x < T(0)) {
		const complex<T> r = catanh(complex<T>(-x, -y));
		return complex<T>(-r.real(), -r.imag());
	}
	const T ax = magnitude(x), ay = magnitude(y);
	if (y == T(0) && ax == T(1)) return complex<T>(with_sign(infinity<T>(), x), y);
	// (1 - x)^2 + y^2 as hypot(1 - x, y)^2, applied one factor at a time, and the atan2
	// arguments divided by s = max(|x|, |y|): nothing is squared at full size, so the formula
	// holds to the edge of the range (float limbs overflow a square past 1.8e19)
	const T omx = T(1) - x;
	const T h = hypot_annexf(omx, y);
	const T re = div(odd_log1p(mul(T(4), div(div(x, h), h))), T(4));
	const T s = (ax > T(1) || ay > T(1)) ? (ax < ay ? ay : ax) : T(1);
	const T im = div(atan2_annexf(mul(T(2), div(y, s)), (omx / s) * (T(1) + x) - (y / s) * y), T(2));
	return complex<T>(re, im);
}

// atan(z) = -i atanh(iz)
template<typename T>
complex<T> catan(const complex<T>& z) {
	return times_minus_i(catanh(times_i(z)));
}

// z^n by repeated squaring
template<typename T>
complex<T> cpow_int(const complex<T>& z, int n) {
	unsigned long long m = n < 0 ? 0ull - static_cast<unsigned long long>(static_cast<long long>(n)) : static_cast<unsigned long long>(n);
	complex<T> result(T(1), T(0)), base = z;
	while (m != 0) {
		if (m & 1ull) result = result * base;
		m >>= 1;
		if (m != 0) base = base * base;
	}
	if (n < 0) return complex<T>(T(1), T(0)) / result;
	return result;
}

}} // namespace detail::cx

////////////////////////////////////////////////////////////////////
// Exponential functions

/// Complex exponential: exp(a + bi) = exp(a) * (cos(b) + i*sin(b))
template<ComplexCompatible T>
complex<T> exp(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::cexp(c);
	}
	else {
		std::complex<double> result = std::exp(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex natural logarithm: log(z) = log(|z|) + i*arg(z)
template<ComplexCompatible T>
complex<T> log(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::clog(c);
	}
	else {
		std::complex<double> result = std::log(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex base-10 logarithm
template<ComplexCompatible T>
complex<T> log10(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		static const T ln10 = log(T(10));
		const complex<T> l = detail::cx::clog(c);
		return complex<T>(detail::cx::div(l.real(), ln10), detail::cx::div(l.imag(), ln10));
	}
	else {
		std::complex<double> result = std::log10(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

////////////////////////////////////////////////////////////////////
// Power functions

/// Complex square root
template<ComplexCompatible T>
complex<T> sqrt(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::csqrt(c);
	}
	else {
		std::complex<double> result = std::sqrt(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex power: base^exp (both complex)
template<ComplexCompatible T>
complex<T> pow(const complex<T>& base, const complex<T>& exponent) {
	if constexpr (is_high_precision_v<T>) {
		if (base.real() == T(0) && base.imag() == T(0)) return complex<T>(T(0), T(0));
		return detail::cx::cexp(exponent * detail::cx::clog(base));
	}
	else {
		std::complex<double> result = std::pow(detail::to_std_complex(base), detail::to_std_complex(exponent));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex power: base^exp (complex base, real exponent)
template<ComplexCompatible T>
complex<T> pow(const complex<T>& base, const T& exponent) {
	if constexpr (is_high_precision_v<T>) {
		if (base.imag() == T(0) && base.real() > T(0)) return complex<T>(pow(base.real(), exponent), base.imag());
		if (base.real() == T(0) && base.imag() == T(0)) return complex<T>(T(0), T(0));
		const complex<T> l = detail::cx::clog(base);
		return polar(exp(exponent * l.real()), exponent * l.imag());
	}
	else {
		std::complex<double> result = std::pow(detail::to_std_complex(base), static_cast<double>(exponent));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex power: base^exp (real base, complex exponent)
template<ComplexCompatible T>
complex<T> pow(const T& base, const complex<T>& exponent) {
	if constexpr (is_high_precision_v<T>) {
		if (base > T(0)) return polar(pow(base, exponent.real()), exponent.imag() * log(base));
		return pow(complex<T>(base, T(0)), exponent);
	}
	else {
		std::complex<double> result = std::pow(static_cast<double>(base), detail::to_std_complex(exponent));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex power: base^exp (complex base, integer exponent)
template<ComplexCompatible T>
complex<T> pow(const complex<T>& base, int exponent) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::cpow_int(base, exponent);
	}
	else {
		std::complex<double> result = std::pow(detail::to_std_complex(base), exponent);
		return detail::from_std_complex<T>(result);
	}
}

////////////////////////////////////////////////////////////////////
// Trigonometric functions

/// Complex sine
template<ComplexCompatible T>
complex<T> sin(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::times_minus_i(detail::cx::csinh(detail::cx::times_i(c)));
	}
	else {
		std::complex<double> result = std::sin(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex cosine
template<ComplexCompatible T>
complex<T> cos(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::ccosh(detail::cx::times_i(c));
	}
	else {
		std::complex<double> result = std::cos(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex tangent
template<ComplexCompatible T>
complex<T> tan(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::times_minus_i(detail::cx::ctanh(detail::cx::times_i(c)));
	}
	else {
		std::complex<double> result = std::tan(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex arc sine
template<ComplexCompatible T>
complex<T> asin(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::casin(c);
	}
	else {
		std::complex<double> result = std::asin(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex arc cosine
template<ComplexCompatible T>
complex<T> acos(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::cacos(c);
	}
	else {
		std::complex<double> result = std::acos(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex arc tangent
template<ComplexCompatible T>
complex<T> atan(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::catan(c);
	}
	else {
		std::complex<double> result = std::atan(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

////////////////////////////////////////////////////////////////////
// Hyperbolic functions

/// Complex hyperbolic sine
template<ComplexCompatible T>
complex<T> sinh(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::csinh(c);
	}
	else {
		std::complex<double> result = std::sinh(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex hyperbolic cosine
template<ComplexCompatible T>
complex<T> cosh(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::ccosh(c);
	}
	else {
		std::complex<double> result = std::cosh(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex hyperbolic tangent
template<ComplexCompatible T>
complex<T> tanh(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::ctanh(c);
	}
	else {
		std::complex<double> result = std::tanh(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex inverse hyperbolic sine
template<ComplexCompatible T>
complex<T> asinh(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::casinh(c);
	}
	else {
		std::complex<double> result = std::asinh(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex inverse hyperbolic cosine
template<ComplexCompatible T>
complex<T> acosh(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::cacosh(c);
	}
	else {
		std::complex<double> result = std::acosh(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

/// Complex inverse hyperbolic tangent
template<ComplexCompatible T>
complex<T> atanh(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::catanh(c);
	}
	else {
		std::complex<double> result = std::atanh(detail::to_std_complex(c));
		return detail::from_std_complex<T>(result);
	}
}

}} // namespace sw::universal
