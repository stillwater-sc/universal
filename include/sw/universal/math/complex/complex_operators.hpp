#pragma once
// complex_operators.hpp: arithmetic operators and free functions for complex<T>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <sstream>
#include <universal/math/complex/complex_impl.hpp>
#include <universal/math/complex/complex_native_real.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// Unary operators

/// Unary plus: returns a copy of the complex number
template<ComplexCompatible T>
constexpr complex<T> operator+(const complex<T>& c) noexcept {
	return c;
}

/// Unary minus: returns the negation of the complex number
template<ComplexCompatible T>
constexpr complex<T> operator-(const complex<T>& c) noexcept {
	return complex<T>(-c.real(), -c.imag());
}

////////////////////////////////////////////////////////////////////
// Binary arithmetic operators: complex-complex

/// Addition of two complex numbers
template<ComplexCompatible T>
constexpr complex<T> operator+(const complex<T>& lhs, const complex<T>& rhs) noexcept {
	return complex<T>(lhs.real() + rhs.real(), lhs.imag() + rhs.imag());
}

/// Subtraction of two complex numbers
template<ComplexCompatible T>
constexpr complex<T> operator-(const complex<T>& lhs, const complex<T>& rhs) noexcept {
	return complex<T>(lhs.real() - rhs.real(), lhs.imag() - rhs.imag());
}

/// Multiplication of two complex numbers
/// (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
template<ComplexCompatible T>
constexpr complex<T> operator*(const complex<T>& lhs, const complex<T>& rhs) noexcept {
	return complex<T>(
		lhs.real() * rhs.real() - lhs.imag() * rhs.imag(),
		lhs.real() * rhs.imag() + lhs.imag() * rhs.real()
	);
}

/// Division of two complex numbers
/// (a + bi) / (c + di) = ((ac + bd) + (bc - ad)i) / (c^2 + d^2)
template<ComplexCompatible T>
constexpr complex<T> operator/(const complex<T>& lhs, const complex<T>& rhs) noexcept {
	T denom = rhs.real() * rhs.real() + rhs.imag() * rhs.imag();
	return complex<T>(
		(lhs.real() * rhs.real() + lhs.imag() * rhs.imag()) / denom,
		(lhs.imag() * rhs.real() - lhs.real() * rhs.imag()) / denom
	);
}

////////////////////////////////////////////////////////////////////
// Binary arithmetic operators: complex-scalar

/// Addition: complex + scalar
template<ComplexCompatible T>
constexpr complex<T> operator+(const complex<T>& lhs, const T& rhs) noexcept {
	return complex<T>(lhs.real() + rhs, lhs.imag());
}

/// Addition: scalar + complex
template<ComplexCompatible T>
constexpr complex<T> operator+(const T& lhs, const complex<T>& rhs) noexcept {
	return complex<T>(lhs + rhs.real(), rhs.imag());
}

/// Subtraction: complex - scalar
template<ComplexCompatible T>
constexpr complex<T> operator-(const complex<T>& lhs, const T& rhs) noexcept {
	return complex<T>(lhs.real() - rhs, lhs.imag());
}

/// Subtraction: scalar - complex
template<ComplexCompatible T>
constexpr complex<T> operator-(const T& lhs, const complex<T>& rhs) noexcept {
	return complex<T>(lhs - rhs.real(), -rhs.imag());
}

/// Multiplication: complex * scalar
template<ComplexCompatible T>
constexpr complex<T> operator*(const complex<T>& lhs, const T& rhs) noexcept {
	return complex<T>(lhs.real() * rhs, lhs.imag() * rhs);
}

/// Multiplication: scalar * complex
template<ComplexCompatible T>
constexpr complex<T> operator*(const T& lhs, const complex<T>& rhs) noexcept {
	return complex<T>(lhs * rhs.real(), lhs * rhs.imag());
}

/// Division: complex / scalar
template<ComplexCompatible T>
constexpr complex<T> operator/(const complex<T>& lhs, const T& rhs) noexcept {
	return complex<T>(lhs.real() / rhs, lhs.imag() / rhs);
}

/// Division: scalar / complex
template<ComplexCompatible T>
constexpr complex<T> operator/(const T& lhs, const complex<T>& rhs) noexcept {
	T denom = rhs.real() * rhs.real() + rhs.imag() * rhs.imag();
	return complex<T>(
		(lhs * rhs.real()) / denom,
		(-lhs * rhs.imag()) / denom
	);
}

////////////////////////////////////////////////////////////////////
// Comparison operators

/// Equality comparison
template<ComplexCompatible T>
constexpr bool operator==(const complex<T>& lhs, const complex<T>& rhs) noexcept {
	return lhs.real() == rhs.real() && lhs.imag() == rhs.imag();
}

/// Inequality comparison
template<ComplexCompatible T>
constexpr bool operator!=(const complex<T>& lhs, const complex<T>& rhs) noexcept {
	return !(lhs == rhs);
}

/// Equality with scalar (imaginary part must be zero)
template<ComplexCompatible T>
constexpr bool operator==(const complex<T>& lhs, const T& rhs) noexcept {
	return lhs.real() == rhs && lhs.imag() == T{};
}

/// Equality with scalar (imaginary part must be zero)
template<ComplexCompatible T>
constexpr bool operator==(const T& lhs, const complex<T>& rhs) noexcept {
	return rhs == lhs;
}

/// Inequality with scalar
template<ComplexCompatible T>
constexpr bool operator!=(const complex<T>& lhs, const T& rhs) noexcept {
	return !(lhs == rhs);
}

/// Inequality with scalar
template<ComplexCompatible T>
constexpr bool operator!=(const T& lhs, const complex<T>& rhs) noexcept {
	return !(lhs == rhs);
}

////////////////////////////////////////////////////////////////////
// Essential free functions

/// Extract the real component
template<ComplexCompatible T>
constexpr T real(const complex<T>& c) noexcept {
	return c.real();
}

/// Extract the imaginary component
template<ComplexCompatible T>
constexpr T imag(const complex<T>& c) noexcept {
	return c.imag();
}

/// Complex conjugate: conj(a + bi) = a - bi
template<ComplexCompatible T>
constexpr complex<T> conj(const complex<T>& c) noexcept {
	return complex<T>(c.real(), -c.imag());
}

/// Squared magnitude (norm): |z|^2 = a^2 + b^2
template<ComplexCompatible T>
constexpr T norm(const complex<T>& c) noexcept {
	return c.real() * c.real() + c.imag() * c.imag();
}

/// Absolute value (magnitude): |z| = sqrt(a^2 + b^2)
/// A high-precision T (is_high_precision) computes it in T, scaled like hypot so that
/// a^2 cannot overflow or underflow; any other T goes through double.
template<ComplexCompatible T>
T abs(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::hypot_annexf(c.real(), c.imag());
	}
	else {
		double re = static_cast<double>(c.real());
		double im = static_cast<double>(c.imag());
		return T(std::hypot(re, im));
	}
}

/// Phase angle (argument): arg(z) = atan2(b, a), in T for a high-precision T
template<ComplexCompatible T>
T arg(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::atan2_annexf(c.imag(), c.real());
	}
	else {
		return T(std::atan2(static_cast<double>(c.imag()), static_cast<double>(c.real())));
	}
}

/// Construct complex from polar coordinates: polar(rho, theta) = rho * (cos(theta) + i*sin(theta))
template<ComplexCompatible T>
complex<T> polar(const T& rho, const T& theta = T{}) {
	if constexpr (is_high_precision_v<T>) {
		return complex<T>(rho * cos(theta), rho * sin(theta));
	}
	else {
		double th = static_cast<double>(theta);
		return complex<T>(rho * T(std::cos(th)), rho * T(std::sin(th)));
	}
}

/// Project onto the Riemann sphere
template<ComplexCompatible T>
complex<T> proj(const complex<T>& c) {
	// If c is infinite, return (inf, copysign(0, imag(c)))
	double re = static_cast<double>(c.real());
	double im = static_cast<double>(c.imag());
	if (std::isinf(re) || std::isinf(im)) {
		return complex<T>(T(std::numeric_limits<double>::infinity()), T(std::copysign(0.0, im)));
	}
	return c;
}

////////////////////////////////////////////////////////////////////
// Classification functions

/// Check if any component is NaN
/// A high-precision T is classified by its own isnan/isinf: its range can exceed double's.
template<ComplexCompatible T>
bool isnan(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::is_nan(c.real()) || detail::cx::is_nan(c.imag());
	}
	else {
		double re = static_cast<double>(c.real());
		double im = static_cast<double>(c.imag());
		return std::isnan(re) || std::isnan(im);
	}
}

/// Check if any component is infinite
template<ComplexCompatible T>
bool isinf(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::is_inf(c.real()) || detail::cx::is_inf(c.imag());
	}
	else {
		double re = static_cast<double>(c.real());
		double im = static_cast<double>(c.imag());
		return std::isinf(re) || std::isinf(im);
	}
}

/// Check if both components are finite
template<ComplexCompatible T>
bool isfinite(const complex<T>& c) {
	if constexpr (is_high_precision_v<T>) {
		return detail::cx::is_finite(c.real()) && detail::cx::is_finite(c.imag());
	}
	else {
		double re = static_cast<double>(c.real());
		double im = static_cast<double>(c.imag());
		return std::isfinite(re) && std::isfinite(im);
	}
}

/// Check if both components are normal (not zero, subnormal, infinite, or NaN)
template<ComplexCompatible T>
bool isnormal(const complex<T>& c) {
	using std::isnormal;
	double re = static_cast<double>(c.real());
	double im = static_cast<double>(c.imag());
	// A complex number is normal if both parts are normal, or one is zero and the other is normal
	bool re_ok = (re == 0.0) || isnormal(re);
	bool im_ok = (im == 0.0) || isnormal(im);
	return re_ok && im_ok && !(re == 0.0 && im == 0.0);
}

////////////////////////////////////////////////////////////////////
// Stream I/O

/// Output stream operator: formats as (real,imag)
template<ComplexCompatible T>
std::ostream& operator<<(std::ostream& os, const complex<T>& c) {
	os << '(' << c.real() << ',' << c.imag() << ')';
	return os;
}

/// Input stream operator: expects format (real,imag)
template<ComplexCompatible T>
std::istream& operator>>(std::istream& is, complex<T>& c) {
	T re{}, im{};
	char ch;

	is >> ch;  // '('
	if (ch != '(') {
		is.setstate(std::ios_base::failbit);
		return is;
	}

	is >> re >> ch;  // real, ','
	if (ch != ',') {
		is.setstate(std::ios_base::failbit);
		return is;
	}

	is >> im >> ch;  // imag, ')'
	if (ch != ')') {
		is.setstate(std::ios_base::failbit);
		return is;
	}

	c = complex<T>(re, im);
	return is;
}

}} // namespace sw::universal
