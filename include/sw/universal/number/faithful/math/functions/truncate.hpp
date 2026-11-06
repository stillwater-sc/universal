#pragma once
// truncate.hpp: floor, ceil, trunc and round for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The integer-valued functions, exact on the two parts (#1954). Rounding to the FloatingPointType
// first would be wrong, not just imprecise: above 2^digits(Real) the value part is an integer, and
// a fraction carried in the error part decides the result, as for dd before #1667.
//
// For a canonical pair (v, e), with |e| at most half an ulp of v:
//   - when v is not an integer, floor(v + e) == floor(v). The fraction of v is a multiple of
//     ulp(v), at least 2|e|, so e cannot carry the sum across an integer;
//   - when v is an integer, floor(v + e) == v + floor(e), and twoSum adds the two exactly.
// ceil, trunc and round follow from floor. round adds 0.5 and 1 to the floor, which is exact:
// a pair that has a fraction has its fraction in a part below 2^(digits - 1), where 0.5 and 1
// are representable.
//
// Only a native FloatingPointType: the parts are rounded with std::floor.
#include <cmath>         // std::floor
#include <type_traits>   // std::is_floating_point_v
#include <universal/number/faithful/core.hpp>
#include <universal/numerics/twosum.hpp>

namespace sw { namespace universal {

namespace faithful_detail {
	// the canonical form of a pair: the value the rounded sum, the error what remains
	template<typename Real>
	inline faithful<Real> canonical(const faithful<Real>& x) {
		Real v(x.high()), e(x.low()), s, r;
		twoSum(v, e, s, r);
		return faithful<Real>(s, r);
	}
}

// the largest integer not greater than x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> floor(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.isinf()) return x;
	if (x.iszero()) return F(x.high() == Real(0) ? x.high() : Real(0), Real(0));   // floor(+-0) = +-0
	const F c = faithful_detail::canonical(x);
	const Real fv = std::floor(c.high());
	if (fv != c.high()) return F(fv, Real(0));
	Real v(c.high()), fe(std::floor(c.low())), s, r;
	twoSum(v, fe, s, r);
	return F(s, r);
}

// the smallest integer not less than x
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> ceil(const faithful<FloatingPointType>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return floor(x);
	return -floor(-x);   // ceil(-0.5) = -0, as -floor(0.5) is
}

// the integer nearest x that is not larger in magnitude
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> trunc(const faithful<FloatingPointType>& x) {
	return x.sign() ? ceil(x) : floor(x);
}

// the integer nearest x, halfway cases rounded away from zero; a zero result keeps x's sign
template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> round(const faithful<FloatingPointType>& x) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	if (x.isnan() || x.isinf() || x.iszero()) return floor(x);
	const bool negative = x.sign();
	const F a = negative ? -x : x;
	const F f = floor(a);
	if (f == a) return x;                            // already an integer
	const F r = (a < f + F(Real(0.5))) ? f : f + F(Real(1));
	return negative ? -r : r;                        // round(-0.3) = -0
}

}} // namespace sw::universal
