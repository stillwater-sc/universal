#pragma once
// complex_native_real.hpp: real kernels for complex<T> computed in T's own arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The complex library computes abs, arg and the transcendentals in T itself when
// is_high_precision<T> is true (ereal, efloat, dd, qd), and through double otherwise
// (#2026). These are the real building blocks of the T path. They take the non-finite
// cases (C99 Annex F) themselves and hand T's own functions finite arguments only, so the
// special values of the complex results do not depend on how a type's real mathlib treats
// infinities and NaNs.

#include <limits>
#include <universal/math/complex/complex_traits.hpp>

namespace sw { namespace universal { namespace detail { namespace cx {

template<typename T> T infinity() { return std::numeric_limits<T>::infinity(); }
template<typename T> T quiet_nan() { return std::numeric_limits<T>::quiet_NaN(); }

template<typename T> bool is_nan(const T& v) { return isnan(v); }
template<typename T> bool is_inf(const T& v) { return isinf(v); }
template<typename T> bool is_finite(const T& v) { return !isnan(v) && !isinf(v); }
template<typename T> bool sign_bit(const T& v) { return signbit(v); }

template<typename T> T magnitude(const T& v) { return sign_bit(v) ? -v : v; }

// v with the sign of s; v and s may be zero, infinite or NaN
template<typename T> T with_sign(const T& v, const T& s) {
	T m = magnitude(v);
	return sign_bit(s) ? -m : m;
}

// Signed zeros decide which side of a branch cut a value lies on, and not every element type
// keeps them through its arithmetic: dd and qd gave 0 * -1 = +0 and sin(-0) = +0 before #2032.
// The kernels therefore form the products and odd functions that can meet a zero through these,
// which give the IEEE sign of a zero result whatever T's arithmetic does with it.
template<typename T> T mul(const T& a, const T& b) {
	if ((a == T(0) && is_finite(b)) || (b == T(0) && is_finite(a))) {
		return (sign_bit(a) != sign_bit(b)) ? -T(0) : T(0);
	}
	return a * b;
}
template<typename T> T odd_sin(const T& v)   { return v == T(0) ? v : sin(v); }
template<typename T> T odd_tan(const T& v)   { return v == T(0) ? v : tan(v); }
template<typename T> T odd_sinh(const T& v)  { return v == T(0) ? v : sinh(v); }
template<typename T> T odd_asinh(const T& v) { return v == T(0) ? v : asinh(v); }
template<typename T> T odd_log1p(const T& v) { return v == T(0) ? v : log1p(v); }
// a + b and a - b of two zeros, by the IEEE rule
template<typename T> T add(const T& a, const T& b) {
	if (a == T(0) && b == T(0)) return (sign_bit(a) && sign_bit(b)) ? -T(0) : T(0);
	return a + b;
}
template<typename T> T sub(const T& a, const T& b) {
	if (a == T(0) && b == T(0)) return (sign_bit(a) && !sign_bit(b)) ? -T(0) : T(0);
	return a - b;
}
// a / b for a finite, nonzero b
template<typename T> T div(const T& a, const T& b) {
	if (a == T(0)) return (sign_bit(a) != sign_bit(b)) ? -T(0) : T(0);
	return a / b;
}

// pi and its fractions in T's precision, computed once per type
template<typename T> const T& pi() {
	static const T v = T(4) * atan(T(1));
	return v;
}
template<typename T> const T& half_pi() {
	static const T v = pi<T>() / T(2);
	return v;
}
template<typename T> const T& quarter_pi() {
	static const T v = pi<T>() / T(4);
	return v;
}
template<typename T> const T& three_quarter_pi() {
	static const T v = T(3) * pi<T>() / T(4);
	return v;
}

// hypot with the Annex F special values: an infinite argument gives +inf even when the
// other one is NaN. The larger magnitude is factored out, so x*x cannot overflow or underflow.
template<typename T>
T hypot_annexf(const T& x, const T& y) {
	if (is_inf(x) || is_inf(y)) return infinity<T>();
	if (is_nan(x) || is_nan(y)) return quiet_nan<T>();
	T a = magnitude(x), b = magnitude(y);
	if (a < b) { T t = a; a = b; b = t; }
	if (a == T(0)) return T(0);
	if (b == T(0)) return a;
	const T r = b / a;
	return a * sqrt(T(1) + r * r);
}

// atan2 with the Annex F special values for zeros, infinities and NaNs
template<typename T>
T atan2_annexf(const T& y, const T& x) {
	if (is_nan(x) || is_nan(y)) return quiet_nan<T>();
	if (y == T(0)) {
		if (sign_bit(x)) return with_sign(pi<T>(), y);   // x < 0 or x == -0
		return y;                                          // +-0 keeps its sign
	}
	if (is_inf(y)) {
		if (is_inf(x)) return with_sign(sign_bit(x) ? three_quarter_pi<T>() : quarter_pi<T>(), y);
		return with_sign(half_pi<T>(), y);
	}
	if (is_inf(x)) {
		if (sign_bit(x)) return with_sign(pi<T>(), y);
		return with_sign(T(0), y);
	}
	if (x == T(0)) return with_sign(half_pi<T>(), y);
	// atan2 is scale invariant; bring extreme arguments near 1 first, since a type's atan2 may
	// square them (dd's and qd's did before #2032: atan2(2e100, -1e200) overflowed)
	const T m = magnitude(x) < magnitude(y) ? magnitude(y) : magnitude(x);
	if (m > T(1e100) || m < T(1e-100)) return atan2(y / m, x / m);
	return atan2(y, x);
}

}}}} // namespace sw::universal::detail::cx
