#pragma once
// complex_literals.hpp: user-defined literals for complex numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Note: This file must be included after complex_impl.hpp so that complex<double> is defined.
// The main complex.hpp header handles the correct include order.

namespace sw { namespace universal {

/// Namespace for complex number user-defined literals
///
/// Usage:
///   using namespace sw::universal::complex_literals;
///   auto z = 3.0 + 4.0_ui;  // Creates complex<double>(3.0, 4.0)
///
/// Note: We use _ui (universal imaginary) instead of _i to avoid conflicts
/// with existing code that may define _i for std::complex types.
///
namespace complex_literals {

/// Imaginary literal for long double values
/// Example: 4.0_ui creates an imaginary number with value 4.0i
inline complex<double> operator""_ui(long double val) {
	return complex<double>(0.0, static_cast<double>(val));
}

/// Imaginary literal for integer values
/// Example: 4_ui creates an imaginary number with value 4i
inline complex<double> operator""_ui(unsigned long long val) {
	return complex<double>(0.0, static_cast<double>(val));
}

} // namespace complex_literals

}} // namespace sw::universal
