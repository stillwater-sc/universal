#pragma once
// numeric_limits.hpp: definition of numeric_limits for positional integer types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

using namespace sw::universal;

template<unsigned _nd, unsigned _rx>
class numeric_limits< sw::universal::positional<_nd, _rx> > {
	using PositionalType = sw::universal::positional<_nd, _rx>;
public:
	static constexpr bool is_specialized = true;
	static constexpr PositionalType min() { return PositionalType().minpos(); }
	static constexpr PositionalType max() { return PositionalType().maxpos(); }
	static constexpr PositionalType lowest() { return PositionalType().maxneg(); }
	static constexpr PositionalType epsilon() { return PositionalType(1); }
	static constexpr PositionalType round_error() { return PositionalType(0); }
	static constexpr PositionalType denorm_min() { return PositionalType().minpos(); }
	static constexpr PositionalType infinity() { return PositionalType().maxpos(); }
	static constexpr PositionalType quiet_NaN() { return PositionalType(0); }
	static constexpr PositionalType signaling_NaN() { return PositionalType(0); }

	static constexpr int  digits       = _nd;
	static constexpr int  digits10     = _nd;
	static constexpr int  max_digits10 = _nd;
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = true;
	static constexpr bool is_exact     = true;
	static constexpr int  radix        = _rx;

	static constexpr int                min_exponent      = 0;
	static constexpr int                min_exponent10    = 0;
	static constexpr int                max_exponent      = 0;
	static constexpr int                max_exponent10    = 0;
	static constexpr bool               has_infinity      = false;
	static constexpr bool               has_quiet_NaN     = false;
	static constexpr bool               has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm        = denorm_absent;
	static constexpr bool               has_denorm_loss   = false;

	static constexpr bool              is_iec559       = false;
	static constexpr bool              is_bounded      = true;
	static constexpr bool              is_modulo       = true;
	static constexpr bool              traps           = false;
	static constexpr bool              tinyness_before = false;
	static constexpr float_round_style round_style     = round_toward_zero;
};

} // namespace std
