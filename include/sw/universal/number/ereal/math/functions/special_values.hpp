#pragma once
// special_values.hpp: helpers for the C99 Annex F special values of the ereal math functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <limits>

namespace sw { namespace universal { namespace ereal_detail {

	// The sign of an ereal is the sign of its leading limb, so -0 is a leading limb of -0.0.
	// isneg() is false for -0; signbit() is the test that sees it.

	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> signed_zero(bool negative) {
		const ereal<maxlimbs, FpType> zero(0.0);
		return negative ? -zero : zero;
	}

	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> signed_inf(bool negative) {
		ereal<maxlimbs, FpType> v;
		v.setinf(negative);
		return v;
	}

	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> quiet_nan() {
		return std::numeric_limits<ereal<maxlimbs, FpType>>::quiet_NaN();
	}

	// A zero result of floor, ceil, trunc, round, rint, fmod, remainder or modf has the sign of
	// the argument (C99 F.10.6, F.10.7): ceil(-0.5), rint(-0.3) and fmod(-2, 1) are -0.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> zero_with_sign_of(const ereal<maxlimbs, FpType>& r, const ereal<maxlimbs, FpType>& x) {
		return r.iszero() ? signed_zero<maxlimbs, FpType>(x.signbit()) : r;
	}

	// 2^(max_exponent / 4): a value above it may square past the limb type's range in an
	// intermediate, 2^256 for double limbs and 2^32 for float limbs
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> square_safe_limit() {
		return ereal<maxlimbs, FpType>(std::ldexp(FpType(1), std::numeric_limits<FpType>::max_exponent / 4));
	}

	// x is a finite integer.  floor is exact on an expansion.
	template<unsigned maxlimbs, typename FpType>
	inline bool is_integer(const ereal<maxlimbs, FpType>& x) {
		if (x.isnan() || x.isinf()) return false;
		return floor(x) == x;
	}

	// x is a finite odd integer: x / 2 is exact, and is not an integer.
	template<unsigned maxlimbs, typename FpType>
	inline bool is_odd_integer(const ereal<maxlimbs, FpType>& x) {
		if (!is_integer(x)) return false;
		return !is_integer(ldexp(x, -1));
	}

}}}  // namespace sw::universal::ereal_detail
