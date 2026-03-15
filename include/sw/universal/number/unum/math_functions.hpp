#pragma once
// math_functions.hpp: math library functions for unum Type I
//
// Initial implementation delegates to native math via double conversion.
// The ubit is automatically set by operator=(double) when the result
// cannot be represented exactly.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <algorithm>

namespace sw { namespace universal {

// classification
template<unsigned esizesize, unsigned fsizesize, typename bt>
bool isnan(const unum<esizesize, fsizesize, bt>& v) { return v.isnan(); }

template<unsigned esizesize, unsigned fsizesize, typename bt>
bool iszero(const unum<esizesize, fsizesize, bt>& v) { return v.iszero(); }

template<unsigned esizesize, unsigned fsizesize, typename bt>
bool isinf(const unum<esizesize, fsizesize, bt>& v) { return v.isinf(); }

// absolute value
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> abs(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> a(v);
	if (!a.iszero() && !a.isnan()) {
		unsigned sign_pos = a.nbits_used() - 1u;
		a.setbit(sign_pos, false);
	}
	return a;
}

// square root
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> sqrt(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan() || v.isneg()) { result.setnan(); return result; }
	result = std::sqrt(v.to_double());
	return result;
}

// exponential
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> exp(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::exp(v.to_double());
	return result;
}

// natural logarithm
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> log(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan() || v.isneg() || v.iszero()) { result.setnan(); return result; }
	result = std::log(v.to_double());
	return result;
}

// base-2 logarithm
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> log2(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan() || v.isneg() || v.iszero()) { result.setnan(); return result; }
	result = std::log2(v.to_double());
	return result;
}

// base-10 logarithm
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> log10(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan() || v.isneg() || v.iszero()) { result.setnan(); return result; }
	result = std::log10(v.to_double());
	return result;
}

// power
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> pow(const unum<esizesize, fsizesize, bt>& base, const unum<esizesize, fsizesize, bt>& exponent) {
	unum<esizesize, fsizesize, bt> result;
	if (base.isnan() || exponent.isnan()) { result.setnan(); return result; }
	result = std::pow(base.to_double(), exponent.to_double());
	return result;
}

// trigonometric functions
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> sin(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::sin(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> cos(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::cos(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> tan(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::tan(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> atan(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::atan(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> asin(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	double d = v.to_double();
	if (d < -1.0 || d > 1.0) { result.setnan(); return result; }
	result = std::asin(d);
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> acos(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	double d = v.to_double();
	if (d < -1.0 || d > 1.0) { result.setnan(); return result; }
	result = std::acos(d);
	return result;
}

// hyperbolic functions
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> sinh(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::sinh(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> cosh(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::cosh(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> tanh(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	if (v.isnan()) { result.setnan(); return result; }
	result = std::tanh(v.to_double());
	return result;
}

// min/max
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> min(const unum<esizesize, fsizesize, bt>& a, const unum<esizesize, fsizesize, bt>& b) {
	return (a < b) ? a : b;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> max(const unum<esizesize, fsizesize, bt>& a, const unum<esizesize, fsizesize, bt>& b) {
	return (a > b) ? a : b;
}

// floor, ceil, truncate
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> floor(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	result = std::floor(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> ceil(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	result = std::ceil(v.to_double());
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> trunc(const unum<esizesize, fsizesize, bt>& v) {
	unum<esizesize, fsizesize, bt> result;
	result = std::trunc(v.to_double());
	return result;
}

}} // namespace sw::universal
