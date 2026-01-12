#pragma once
// complex.hpp: standalone complex number implementation for Universal types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header provides a complete complex number implementation that works with
// user-defined floating-point types, bypassing the ISO C++ 26.2/2 restriction
// that std::complex<T> is only valid for float, double, and long double.
//
// Usage:
//   #include <universal/number/posit/posit.hpp>
//   #include <universal/math/complex.hpp>
//
//   using Real = sw::universal::posit<32, 2>;
//   using Complex = sw::universal::complex<Real>;
//
//   Complex z1(Real(1.0), Real(2.0));  // 1 + 2i
//   Complex z2 = exp(z1);              // exponential
//   Real magnitude = abs(z1);          // |z1|
//
// For high-precision types (dd, qd), native implementations are used to
// preserve full precision. For other types, transcendental functions
// delegate to std::complex<double>.

#include <cmath>
#include <complex>    // for std::complex<double> interop
#include <iostream>
#include <sstream>
#include <type_traits>

// Core implementation
#include <universal/math/complex/complex_traits.hpp>
#include <universal/math/complex/complex_impl.hpp>
#include <universal/math/complex/complex_operators.hpp>
#include <universal/math/complex/complex_functions.hpp>
#include <universal/math/complex/complex_literals.hpp>

// Note: Native implementations for dd and qd are included conditionally
// when those types are used. Include them explicitly if needed:
//   #include <universal/math/complex/complex_functions_dd.hpp>
//   #include <universal/math/complex/complex_functions_qd.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// Utility functions for complex types

/// Convert complex number to binary string representation
template<ComplexCompatible T>
std::string to_binary(const complex<T>& c, bool nibbleMarker = false) {
	std::stringstream ss;
	ss << '(' << to_binary(c.real(), nibbleMarker)
	   << ", " << to_binary(c.imag(), nibbleMarker) << ')';
	return ss.str();
}

/// Convert complex number to string representation
template<ComplexCompatible T>
std::string to_string(const complex<T>& c) {
	std::stringstream ss;
	ss << '(' << c.real() << ',' << c.imag() << ')';
	return ss.str();
}

/// Convert complex number to triple representation (sign, scale, fraction)
template<ComplexCompatible T>
std::string to_triple(const complex<T>& c) {
	std::stringstream ss;
	ss << "real: " << to_triple(c.real()) << " imag: " << to_triple(c.imag());
	return ss.str();
}

////////////////////////////////////////////////////////////////////
// Color functions for visualization (returns hue based on argument)

/// Map complex number to hue value [0, 360) based on argument
template<ComplexCompatible T>
double complex_to_hue(const complex<T>& c) {
	double phase = static_cast<double>(arg(c));  // [-pi, pi]
	double hue = (phase + 3.14159265358979323846) * 180.0 / 3.14159265358979323846;  // [0, 360)
	return hue;
}

}} // namespace sw::universal
