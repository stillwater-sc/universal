#pragma once
// numeric_limits.hpp: definition of numeric_limits for unum Type I
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

template<unsigned esizesize, unsigned fsizesize, typename bt>
class numeric_limits< sw::universal::unum<esizesize, fsizesize, bt> > {
public:
	using UNUM = sw::universal::unum<esizesize, fsizesize, bt>;
	static constexpr bool is_specialized = true;
	static constexpr UNUM min() {
		UNUM u;
		return sw::universal::minpos<esizesize, fsizesize, bt>(u);
	}
	static constexpr UNUM max() {
		UNUM u;
		return sw::universal::maxpos<esizesize, fsizesize, bt>(u);
	}
	static constexpr UNUM lowest() {
		UNUM u;
		return sw::universal::maxneg<esizesize, fsizesize, bt>(u);
	}
	static constexpr UNUM epsilon() {
		return UNUM{};  // stub: Phase 2
	}
	static constexpr UNUM round_error() {
		return UNUM{};  // stub
	}
	static constexpr UNUM denorm_min() {
		return UNUM{};  // stub
	}
	static constexpr UNUM infinity() {
		return UNUM{};  // unum Type I has no infinity
	}
	static constexpr UNUM quiet_NaN() {
		UNUM u;
		return sw::universal::qnan<esizesize, fsizesize, bt>(u);
	}
	static constexpr UNUM signaling_NaN() {
		UNUM u;
		return sw::universal::snan<esizesize, fsizesize, bt>(u);
	}

	// max fraction bits + 1 hidden bit
	static constexpr int digits       = static_cast<int>((1u << fsizesize));
	static constexpr int digits10     = static_cast<int>(digits / 3.3);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -(1 << ((1 << esizesize) - 1));
	static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3);
	static constexpr int max_exponent   = (1 << ((1 << esizesize) - 1));
	static constexpr int max_exponent10 = static_cast<int>(max_exponent / 3.3);
	static constexpr bool has_infinity  = false;  // unum Type I has no infinity
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

} // namespace std
