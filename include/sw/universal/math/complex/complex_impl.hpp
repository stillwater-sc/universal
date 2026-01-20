#pragma once
// complex_impl.hpp: core complex<T> class template implementation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This is a standalone complex number implementation that works with user-defined
// floating-point types, bypassing the ISO C++ 26.2/2 restriction that std::complex<T>
// is only valid for float, double, and long double.

#include <complex>  // for std::complex<double> interoperability
#include <universal/math/complex/complex_traits.hpp>

namespace sw { namespace universal {

/// complex<T>: A complex number class template for Universal number types
///
/// This class provides a drop-in replacement for std::complex<T> that works
/// with any type satisfying the ComplexCompatible concept (supports basic
/// arithmetic and conversion to/from double).
///
/// @tparam T The underlying real number type (e.g., posit<32,2>, cfloat<32,8>, dd, qd)
template<ComplexCompatible T>
class complex {
public:
	using value_type = T;

	// Default constructor: initializes to (0, 0)
	constexpr complex() noexcept : _re{}, _im{} {}

	// Constructor from real part only: initializes to (re, 0)
	constexpr complex(const T& re) noexcept : _re{re}, _im{} {}

	// Constructor from real and imaginary parts
	constexpr complex(const T& re, const T& im) noexcept : _re{re}, _im{im} {}

	// Copy constructor
	constexpr complex(const complex&) noexcept = default;

	// Move constructor
	constexpr complex(complex&&) noexcept = default;

	// Converting constructor from different complex types
	template<ComplexCompatible U>
	constexpr complex(const complex<U>& other) noexcept
		: _re{static_cast<T>(other.real())}, _im{static_cast<T>(other.imag())} {}

	// Interoperability: construct from std::complex<double>
	constexpr complex(const std::complex<double>& c) noexcept
		: _re{T(c.real())}, _im{T(c.imag())} {}

	// Interoperability: construct from std::complex<float>
	constexpr complex(const std::complex<float>& c) noexcept
		: _re{T(static_cast<double>(c.real()))}, _im{T(static_cast<double>(c.imag()))} {}

	// Interoperability: construct from std::complex<long double>
	constexpr complex(const std::complex<long double>& c) noexcept
		: _re{T(static_cast<double>(c.real()))}, _im{T(static_cast<double>(c.imag()))} {}

	// Accessors
	constexpr T real() const noexcept { return _re; }
	constexpr T imag() const noexcept { return _im; }

	// Setters
	constexpr void real(const T& re) noexcept { _re = re; }
	constexpr void imag(const T& im) noexcept { _im = im; }

	// Copy assignment
	constexpr complex& operator=(const complex&) noexcept = default;

	// Move assignment
	constexpr complex& operator=(complex&&) noexcept = default;

	// Assignment from real value
	constexpr complex& operator=(const T& re) noexcept {
		_re = re;
		_im = T{};
		return *this;
	}

	// Compound assignment: addition
	constexpr complex& operator+=(const complex& rhs) noexcept {
		_re += rhs._re;
		_im += rhs._im;
		return *this;
	}

	// Compound assignment: addition with scalar
	constexpr complex& operator+=(const T& rhs) noexcept {
		_re += rhs;
		return *this;
	}

	// Compound assignment: subtraction
	constexpr complex& operator-=(const complex& rhs) noexcept {
		_re -= rhs._re;
		_im -= rhs._im;
		return *this;
	}

	// Compound assignment: subtraction with scalar
	constexpr complex& operator-=(const T& rhs) noexcept {
		_re -= rhs;
		return *this;
	}

	// Compound assignment: multiplication
	// (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
	constexpr complex& operator*=(const complex& rhs) noexcept {
		T temp_re = _re * rhs._re - _im * rhs._im;
		_im = _re * rhs._im + _im * rhs._re;
		_re = temp_re;
		return *this;
	}

	// Compound assignment: multiplication with scalar
	constexpr complex& operator*=(const T& rhs) noexcept {
		_re *= rhs;
		_im *= rhs;
		return *this;
	}

	// Compound assignment: division
	// (a + bi) / (c + di) = ((ac + bd) + (bc - ad)i) / (c^2 + d^2)
	constexpr complex& operator/=(const complex& rhs) noexcept {
		T denom = rhs._re * rhs._re + rhs._im * rhs._im;
		T temp_re = (_re * rhs._re + _im * rhs._im) / denom;
		_im = (_im * rhs._re - _re * rhs._im) / denom;
		_re = temp_re;
		return *this;
	}

	// Compound assignment: division with scalar
	constexpr complex& operator/=(const T& rhs) noexcept {
		_re /= rhs;
		_im /= rhs;
		return *this;
	}

	// Conversion to std::complex<double> for interfacing with standard library
	explicit constexpr operator std::complex<double>() const noexcept {
		return std::complex<double>(static_cast<double>(_re), static_cast<double>(_im));
	}

	// Conversion to std::complex<float>
	explicit constexpr operator std::complex<float>() const noexcept {
		return std::complex<float>(static_cast<float>(static_cast<double>(_re)),
		                          static_cast<float>(static_cast<double>(_im)));
	}

private:
	T _re;  // Real part
	T _im;  // Imaginary part
};

////////////////////////////////////////////////////////////////////
// Trait specialization now that complex is defined

/// Specialization for sw::universal::complex
template<ComplexCompatible T>
struct is_sw_complex<complex<T>> : std::true_type {};

}} // namespace sw::universal
