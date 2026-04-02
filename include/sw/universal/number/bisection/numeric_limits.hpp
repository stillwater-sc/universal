#pragma once
// numeric_limits.hpp: std::numeric_limits specialization for bisection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

namespace std {

template<typename Generator, typename Refinement, unsigned nbits, typename bt>
class numeric_limits<sw::universal::bisection<Generator, Refinement, nbits, bt>> {
public:
	using Bisection = sw::universal::bisection<Generator, Refinement, nbits, bt>;

	static constexpr bool is_specialized = true;

	static constexpr Bisection min() {
		Bisection v; v.minpos(); return v;
	}
	static constexpr Bisection max() {
		Bisection v; v.maxpos(); return v;
	}
	static constexpr Bisection lowest() {
		Bisection v; v.maxneg(); return v;
	}
	static constexpr Bisection epsilon() {
		Bisection one(1.0), incr(1.0);
		++incr;
		return incr - one;
	}
	static constexpr Bisection round_error() { return Bisection(0.5); }
	static constexpr Bisection denorm_min() { return min(); }
	static constexpr Bisection infinity() {
		Bisection v; v.maxpos(); return v;
	}
	static constexpr Bisection quiet_NaN() {
		Bisection v; v.setnan(); return v;
	}
	static constexpr Bisection signaling_NaN() { return quiet_NaN(); }

	static constexpr int digits = nbits - 1;
	static constexpr int digits10 = static_cast<int>(digits * 0.301029995663981);
	static constexpr int max_digits10 = digits10 + 1;
	static constexpr bool is_signed = true;
	static constexpr bool is_integer = false;
	static constexpr bool is_exact = false;
	static constexpr int radix = 2;

	static constexpr bool has_infinity = false;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
