#pragma once
// constexpr_ldexp.hpp: x * 2^e in a native floating-point type, usable in constant expressions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <array>
#include <cmath>        // std::ldexp
#include <limits>
#include <type_traits>  // std::is_constant_evaluated

namespace sw { namespace universal {

	// x * 2^e, correctly rounded, as std::ldexp gives it. std::ldexp is not constexpr before C++23,
	// and clang rejects it in a constant expression (gcc accepts it as a builtin), so a constant
	// evaluation multiplies by exact powers of two instead: 2^(2^i), largest first, a logarithmic
	// number of steps that stays inside MSVC's constexpr step limit. Every step is exact while the
	// running value stays normal; only the last can take it below the normal range, so it rounds
	// once. Overflow gives inf, and an infinite or nan x stays one. Run-time calls go to std::ldexp.
	// POWER's IBM double-double long double (106 digits) is not a binary format: gcc evaluates it in
	// a constant expression only where every step is exact, so a result that has to round, below
	// its normal range, is not a constant expression there. Run time is unaffected.
	template<typename Real>
	constexpr Real constexpr_ldexp(Real x, int e) {
		static_assert(std::numeric_limits<Real>::radix == 2, "constexpr_ldexp scales by powers of two");
		if (!std::is_constant_evaluated()) {
			return std::ldexp(x, e);
		}
		using limits   = std::numeric_limits<Real>;
		auto magnitude = [](Real v) { return v < Real(0) ? -v : v; };
		if (x == Real(0) || x != x || magnitude(x) > limits::max()) return x;  // zero keeps its sign; nan; inf
		constexpr Real minNormal    = limits::min();
		constexpr int  digits       = limits::digits;
		// K powers 2^(2^i) with 2^(2^(K-1)) < 2^(max_exponent - 1): finite, and their reciprocals normal
		constexpr int K = [] {
			int k = 0;
			while ((1 << (k + 1)) < limits::max_exponent - 1) ++k;
			return k + 1;
		}();
		struct Powers {
			std::array<Real, K> up{}, down{}, threshold{};  // 2^(2^i), 2^-(2^i), minNormal * 2^(2^i)
		};
		constexpr Powers p = [] {
			Powers r{};
			r.up[0]   = Real(2);
			r.down[0] = Real(0.5);
			for (int i = 1; i < K; ++i) {
				r.up[i]   = r.up[i - 1] * r.up[i - 1];
				r.down[i] = r.down[i - 1] * r.down[i - 1];
			}
			for (int i = 0; i < K; ++i) r.threshold[i] = minNormal * r.up[i];
			return r;
		}();
		// scaling up: exact while the result stays finite. An overflow is detected before the multiply,
		// by the exact max / 2^(2^i): gcc does not accept a constant expression that overflows to inf
		for (int i = K - 1; i >= 0 && e > 0; --i) {
			for (; e >= (1 << i); e -= (1 << i)) {
				if (magnitude(x) > limits::max() / p.up[i]) return x < Real(0) ? -limits::infinity() : limits::infinity();
				x *= p.up[i];
			}
		}
		// scaling down: exact while the result stays normal
		for (int i = K - 1; i >= 0 && e < 0; --i) {
			for (; e <= -(1 << i) && magnitude(x) >= p.threshold[i]; e += (1 << i)) x *= p.down[i];
		}
		if (e == 0) return x;
		// |x| < 2 * minNormal and the result is below the normal range: at most 2 * minNormal * 2^e.
		// Past digits + 1 halvings that is under half the smallest subnormal, and rounds to zero.
		if (e < -(digits + 1)) return x * Real(0);
		Real scale = Real(1);  // 2^e, at least 2^-(digits + 1): a normal number, built exactly
		for (int i = K - 1; i >= 0; --i) {
			if ((-e) & (1 << i)) scale *= p.down[i];
		}
		return x * scale;  // the one rounding step
	}

}} // namespace sw::universal
