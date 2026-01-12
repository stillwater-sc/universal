#pragma once
// complex_functions.hpp: transcendental functions for complex<T>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Default implementations delegate to std::complex<double> for transcendental functions.
// This maintains the existing library pattern and provides correct results for most types.
// For high-precision types (dd, qd), native implementations are provided in separate headers.

#include <complex>
#include <cmath>
#include <universal/math/complex/complex_impl.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// Helper: convert to std::complex<double> and back
// This is the default strategy for transcendental functions

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
// Exponential functions

/// Complex exponential: exp(a + bi) = exp(a) * (cos(b) + i*sin(b))
template<ComplexCompatible T>
complex<T> exp(const complex<T>& c) {
	std::complex<double> result = std::exp(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex natural logarithm: log(z) = log(|z|) + i*arg(z)
template<ComplexCompatible T>
complex<T> log(const complex<T>& c) {
	std::complex<double> result = std::log(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex base-10 logarithm
template<ComplexCompatible T>
complex<T> log10(const complex<T>& c) {
	std::complex<double> result = std::log10(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

////////////////////////////////////////////////////////////////////
// Power functions

/// Complex square root
template<ComplexCompatible T>
complex<T> sqrt(const complex<T>& c) {
	std::complex<double> result = std::sqrt(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex power: base^exp (both complex)
template<ComplexCompatible T>
complex<T> pow(const complex<T>& base, const complex<T>& exponent) {
	std::complex<double> result = std::pow(detail::to_std_complex(base), detail::to_std_complex(exponent));
	return detail::from_std_complex<T>(result);
}

/// Complex power: base^exp (complex base, real exponent)
template<ComplexCompatible T>
complex<T> pow(const complex<T>& base, const T& exponent) {
	std::complex<double> result = std::pow(detail::to_std_complex(base), static_cast<double>(exponent));
	return detail::from_std_complex<T>(result);
}

/// Complex power: base^exp (real base, complex exponent)
template<ComplexCompatible T>
complex<T> pow(const T& base, const complex<T>& exponent) {
	std::complex<double> result = std::pow(static_cast<double>(base), detail::to_std_complex(exponent));
	return detail::from_std_complex<T>(result);
}

/// Complex power: base^exp (complex base, integer exponent)
template<ComplexCompatible T>
complex<T> pow(const complex<T>& base, int exponent) {
	std::complex<double> result = std::pow(detail::to_std_complex(base), exponent);
	return detail::from_std_complex<T>(result);
}

////////////////////////////////////////////////////////////////////
// Trigonometric functions

/// Complex sine
template<ComplexCompatible T>
complex<T> sin(const complex<T>& c) {
	std::complex<double> result = std::sin(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex cosine
template<ComplexCompatible T>
complex<T> cos(const complex<T>& c) {
	std::complex<double> result = std::cos(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex tangent
template<ComplexCompatible T>
complex<T> tan(const complex<T>& c) {
	std::complex<double> result = std::tan(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex arc sine
template<ComplexCompatible T>
complex<T> asin(const complex<T>& c) {
	std::complex<double> result = std::asin(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex arc cosine
template<ComplexCompatible T>
complex<T> acos(const complex<T>& c) {
	std::complex<double> result = std::acos(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex arc tangent
template<ComplexCompatible T>
complex<T> atan(const complex<T>& c) {
	std::complex<double> result = std::atan(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

////////////////////////////////////////////////////////////////////
// Hyperbolic functions

/// Complex hyperbolic sine
template<ComplexCompatible T>
complex<T> sinh(const complex<T>& c) {
	std::complex<double> result = std::sinh(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex hyperbolic cosine
template<ComplexCompatible T>
complex<T> cosh(const complex<T>& c) {
	std::complex<double> result = std::cosh(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex hyperbolic tangent
template<ComplexCompatible T>
complex<T> tanh(const complex<T>& c) {
	std::complex<double> result = std::tanh(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex inverse hyperbolic sine
template<ComplexCompatible T>
complex<T> asinh(const complex<T>& c) {
	std::complex<double> result = std::asinh(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex inverse hyperbolic cosine
template<ComplexCompatible T>
complex<T> acosh(const complex<T>& c) {
	std::complex<double> result = std::acosh(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

/// Complex inverse hyperbolic tangent
template<ComplexCompatible T>
complex<T> atanh(const complex<T>& c) {
	std::complex<double> result = std::atanh(detail::to_std_complex(c));
	return detail::from_std_complex<T>(result);
}

}} // namespace sw::universal
