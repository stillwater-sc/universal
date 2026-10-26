#pragma once
// numeric_limits.hpp: definition of numeric_limits for efloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <limits>
#include <vector>
// efloat_impl.hpp, not the efloat.hpp umbrella: the umbrella includes core.hpp, which
// includes this header (the cycle areal hit, #1452)
#include <universal/number/efloat/efloat_impl.hpp>

namespace std {

// The limits of efloat<nlimbs> at its widest precision, nlimbs * 32 bits (#1786). A value may
// carry a narrower precision p (set_precision); its own epsilon is then 2^(1 - p). min() and
// max() are the ends of the exponent range, [Efloat::min_scale, Efloat::max_scale]. Past them
// arithmetic overflows and underflows as the rounding mode directs (efloat::enforce_range):
// overflow gives inf, or the largest value at the result's precision where the mode rounds
// toward zero; underflow gives 0, or min() where the mode rounds away from zero, and to
// nearest above min() / 2.
//
// The values that are powers of two carry no precision of their own (#1757): they are exact,
// so x + epsilon() stays at x's precision rather than widening to the type's. max() and
// lowest() need all `digits` bits and carry them.
template<unsigned nlimbs>
class numeric_limits< sw::universal::efloat<nlimbs> > {
	using Efloat = sw::universal::efloat<nlimbs>;
	static Efloat power_of_2(int64_t e) {
		Efloat v(1);   // from an integer: exact, carries no precision
		v.setexponent(e);
		return v;
	}
public:
	static constexpr bool is_specialized = true;
	static Efloat min() {      // smallest positive normal value
		return power_of_2(Efloat::min_scale);
	}
	static Efloat max() {      // largest finite value: digits ones times 2^(max_scale - digits + 1)
		Efloat v(false, Efloat::max_scale, std::vector<uint32_t>(nlimbs, 0xFFFF'FFFFu));
		v.set_precision(static_cast<unsigned>(digits));
		return v;
	}
	static Efloat lowest() {   // most negative finite value
		return -(max)();
	}
	static Efloat epsilon() {  // the distance from 1.0 to the next value at `digits` bits
		return power_of_2(1 - digits);
	}
	static Efloat round_error() {   // under the default round to nearest
		return power_of_2(-1);
	}
	static Efloat denorm_min() {    // denorm_absent: C++ then requires min()
		return (min)();
	}
	static Efloat infinity() {
		Efloat v(0);
		v.setinf(false);
		return v;
	}
	static Efloat quiet_NaN() {
		Efloat v(0);
		v.setnan(true);
		return v;
	}
	static Efloat signaling_NaN() {
		Efloat v(0);
		v.setnan(false);
		return v;
	}

	static constexpr int  digits       = static_cast<int>(nlimbs) * 32;
	// floor((digits - 1) log10 2) and ceil(1 + digits log10 2), as for float and double
	static constexpr int  digits10     = static_cast<int>((int64_t(digits) - 1) * 30103 / 100000);
	static constexpr int  max_digits10 = static_cast<int>(int64_t(digits) * 30103 / 100000) + 2;
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = false;
	static constexpr bool is_exact     = false;
	static constexpr int  radix        = 2;

	// min() is 2^(min_exponent - 1) and max() is below 2^max_exponent. efloat's range, 2^62
	// binades either way, does not fit in an int, so the four exponents saturate: every int
	// power of 2 (and of 10) is inside the range.
	static constexpr int  min_exponent   = -std::numeric_limits<int>::max();
	static constexpr int  min_exponent10 = -std::numeric_limits<int>::max();
	static constexpr int  max_exponent   =  std::numeric_limits<int>::max();
	static constexpr int  max_exponent10 =  std::numeric_limits<int>::max();

	static constexpr bool has_infinity             = true;
	static constexpr bool has_quiet_NaN            = true;
	static constexpr bool has_signaling_NaN        = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss          = false;

	static constexpr bool is_iec559       = false;
	static constexpr bool is_bounded      = true;
	static constexpr bool is_modulo       = false;
	static constexpr bool traps           = false;
	static constexpr bool tinyness_before = false;
	// the mode is efloat_rounding_mode, thread_local and round to nearest unless changed, as
	// the native types report round_to_nearest under fesetround
	static constexpr float_round_style round_style = round_to_nearest;
};

}  // namespace std
