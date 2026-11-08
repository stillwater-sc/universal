#pragma once
// fractional.hpp: fractional support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <limits>

namespace sw { namespace universal {

	namespace ereal_detail {

		// fmod(x, 0) and remainder(x, 0): NaN, as in C99, where they used to throw in every
		// build (#2006).  Under EREAL_THROW_ARITHMETIC_EXCEPTION they throw ereal_divide_by_zero,
		// as x / 0 does.
		template<unsigned maxlimbs, typename FpType>
		inline ereal<maxlimbs, FpType> remainder_by_zero() {
#if EREAL_THROW_ARITHMETIC_EXCEPTION
			throw ereal_divide_by_zero();
#else
			return quiet_nan<maxlimbs, FpType>();
#endif
		}

		// The exact remainder of a / b for finite a >= 0 and b > 0: r = a - n b in [0, b), with
		// the parity of n.  The quotient is taken in chunks of `chunk` bits from the leading
		// limbs, so n can be any size: fmod(1e200, 3) is 2, where trunc(x / y) carried only the
		// type's precision and left 1.4e70 (#2006).  Each step subtracts q * b * 2^k with q < 2^(chunk+2),
		// a limb-sized integer, so q * b and the subtraction are exact, and no division is needed.
		template<unsigned maxlimbs, typename FpType>
		inline ereal<maxlimbs, FpType> exact_remainder(const ereal<maxlimbs, FpType>& a, const ereal<maxlimbs, FpType>& b, bool& odd) {
			using Real = ereal<maxlimbs, FpType>;
			constexpr int digits = std::numeric_limits<FpType>::digits;
			constexpr int chunk = (digits > 16) ? digits - 8 : digits / 2;   // q stays well inside a limb
			odd = false;
			Real r = a;
			const int eb = ilogb(b);
			while (!(r < b)) {
				const int d = ilogb(r) - eb;                 // r / b is in [2^(d-1), 2^(d+1))
				const int k = (d > chunk) ? d - chunk : 0;
				const Real bk = ldexp(b, k);                  // exact
				// the leading limbs give the quotient to within a few units; correct it exactly
				FpType q = std::floor(r.limbs()[0] / bk.limbs()[0]);
				if (q < FpType(0)) q = FpType(0);
				Real t = r - Real(q) * bk;
				for (int i = 0; i < 8 && t.isneg(); ++i)  { q -= FpType(1); t += bk; }
				for (int i = 0; i < 8 && !(t < bk); ++i)  { q += FpType(1); t -= bk; }
				if (k == 0 && std::fmod(q, FpType(2)) != FpType(0)) odd = !odd;
				r = t;
			}
			return r;
		}

	}  // namespace ereal_detail

	// fmod: floating-point remainder of x/y
	// fmod(x, y) = x - n*y where n = trunc(x/y), computed exactly (C99 F.10.7.1):
	// the result has the sign of x and |result| < |y|.  fmod(+-0, y) = +-0 for y != 0;
	// fmod(x, +-inf) = x for finite x; NaN for an infinite x, a zero y, or a NaN operand.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> fmod(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		using Real = ereal<maxlimbs, FpType>;
		if (x.isnan() || y.isnan() || x.isinf()) return ereal_detail::quiet_nan<maxlimbs, FpType>();
		if (y.iszero()) return ereal_detail::remainder_by_zero<maxlimbs, FpType>();
		if (y.isinf() || x.iszero()) return x;
		bool odd = false;
		const Real r = ereal_detail::exact_remainder(abs(x), abs(y), odd);
		return x.signbit() ? ereal_detail::zero_with_sign_of(-r, x) : ereal_detail::zero_with_sign_of(r, x);
	}

	// remainder: IEEE remainder of x/y
	// remainder(x, y) = x - n*y where n is x/y rounded to the nearest integer, ties to even,
	// computed exactly (C99 F.10.7.2).  The result is in [-|y|/2, |y|/2]; a zero result has the
	// sign of x.  Special values as for fmod.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> remainder(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		using Real = ereal<maxlimbs, FpType>;
		if (x.isnan() || y.isnan() || x.isinf()) return ereal_detail::quiet_nan<maxlimbs, FpType>();
		if (y.iszero()) return ereal_detail::remainder_by_zero<maxlimbs, FpType>();
		if (y.isinf() || x.iszero()) return x;
		const Real ay = abs(y);
		bool odd = false;
		Real r = ereal_detail::exact_remainder(abs(x), ay, odd);   // |x| - n |y|, in [0, |y|)
		// round n to nearest: past the half, or at the half with an odd n, take n + 1
		const Real twice = ldexp(r, 1);
		if (twice > ay || (twice == ay && odd)) r = r - ay;
		return x.signbit() ? ereal_detail::zero_with_sign_of(-r, x) : ereal_detail::zero_with_sign_of(r, x);
	}

	// modf: split x into integer and fractional parts. Returns the fractional
	// part (same sign as x) and stores the integer part (toward zero) in *iptr.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> modf(const ereal<maxlimbs, FpType>& x, ereal<maxlimbs, FpType>* iptr) {
		if (x.isnan()) {
			if (iptr != nullptr) *iptr = x;
			return x;
		}
		if (x.isinf()) {
			if (iptr != nullptr) *iptr = x;
			return x.isneg() ? -ereal<maxlimbs, FpType>(0.0) : ereal<maxlimbs, FpType>(0.0);
		}
		ereal<maxlimbs, FpType> ipart = trunc(x);
		if (iptr != nullptr) *iptr = ipart;
		// fractional part, with the sign of x even when it is zero: modf(-3) is -0 (C99 F.10.3.12)
		return ereal_detail::zero_with_sign_of(x - ipart, x);
	}

}} // namespace sw::universal
