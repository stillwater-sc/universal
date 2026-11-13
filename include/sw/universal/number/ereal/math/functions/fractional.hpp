#pragma once
// fractional.hpp: fractional support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <limits>
#include <vector>
#include <universal/internal/expansion/expansion_ops.hpp>

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
		// a limb-sized integer, and no division is needed.
		//
		// The steps run on the raw expansions, not on ereal: q * b of a multi-limb b has twice
		// b's limbs, and ereal's arithmetic cuts every result to the limb budget, so the old
		// Real(q) * bk lost the low limbs of the product and the remainder was off by them
		// (ereal<4> fmod(1e30/7, 1/3) was ~1e-101 relative to y, #2054).  The exact remainder is
		// below b, so it spans about b's precision: it is returned exactly whenever it fits the
		// limb budget, and otherwise as its leading limb_budget limbs, like any other result.
		template<unsigned maxlimbs, typename FpType>
		inline std::vector<FpType> exact_remainder_expansion(const ereal<maxlimbs, FpType>& a, const ereal<maxlimbs, FpType>& b, bool& odd) {
			using Expansion = std::vector<FpType>;
			constexpr int digits = std::numeric_limits<FpType>::digits;
			constexpr int chunk = (digits > 16) ? digits - 8 : digits / 2;   // q stays well inside a limb
			auto negated = [](Expansion e) { for (auto& v : e) v = -v; return e; };
			auto sign = [](const Expansion& e) { return expansion_ops::sign_adaptive(e); };
			odd = false;
			Expansion r = a.limbs();
			const Expansion& B = b.limbs();
			const int eb = std::ilogb(B[0]);
			while (expansion_ops::compare_adaptive(r, B) >= 0) {
				const int d = std::ilogb(r[0]) - eb;           // r / b is in [2^(d-1), 2^(d+1))
				const int k = (d > chunk) ? d - chunk : 0;
				Expansion bk(B);
				for (auto& v : bk) v = std::ldexp(v, k);       // exact
				// the leading limbs give the quotient to within a few units; correct it exactly
				FpType q = std::floor(r[0] / bk[0]);
				if (q < FpType(0)) q = FpType(0);
				// near the top of the range q * bk can round past it (fmod(DBL_MAX, 2 - 2^-44)
				// rounds q up to 2^45 + 1), and an inf product would leave NaN limbs in the raw
				// subtraction: back q off until the product is finite; the correction below adds
				// back what that undershoots
				while (q > FpType(0) && !std::isfinite(q * bk[0])) q -= FpType(1);
				Expansion t = expansion_ops::expansion_sum_normalized(r, negated(expansion_ops::expansion_product(bk, Expansion{ q })));
				for (int i = 0; i < 8 && sign(t) < 0; ++i) {
					q -= FpType(1);
					t = expansion_ops::expansion_sum_normalized(t, bk);
				}
				for (int i = 0; i < 8 && expansion_ops::compare_adaptive(t, bk) >= 0; ++i) {
					q += FpType(1);
					t = expansion_ops::expansion_sum_normalized(t, negated(bk));
				}
				if (k == 0 && std::fmod(q, FpType(2)) != FpType(0)) odd = !odd;
				r = t;
				if (r.empty()) break;   // an exact multiple
			}
			return r;
		}

		// an expansion as an ereal, cut to the limb budget TOWARD ZERO: a plain cut drops the tail,
		// and a tail of the opposite sign leaves a result larger in magnitude than the exact
		// one, so a remainder just below y came back as y itself, breaking |fmod(x, y)| < |y|.
		// The last kept limb is then stepped one unit toward zero, which, the tail being below
		// half its unit, puts the result between the exact value and zero.
		template<unsigned maxlimbs, typename FpType>
		inline ereal<maxlimbs, FpType> from_expansion(std::vector<FpType> e) {
			using Real = ereal<maxlimbs, FpType>;
			constexpr std::size_t budget = Real::limb_budget;
			if (e.size() > budget && e[budget] != FpType(0) && (std::signbit(e[budget]) != std::signbit(e[0]))) {
				e[budget - 1] = std::nextafter(e[budget - 1], std::signbit(e[0]) ? std::numeric_limits<FpType>::infinity() : -std::numeric_limits<FpType>::infinity());
			}
			if (e.size() > budget) e.resize(budget);
			Real result(0.0);
			for (FpType limb : e) result += Real(limb);
			return result;
		}

		template<unsigned maxlimbs, typename FpType>
		inline ereal<maxlimbs, FpType> exact_remainder(const ereal<maxlimbs, FpType>& a, const ereal<maxlimbs, FpType>& b, bool& odd) {
			return from_expansion<maxlimbs, FpType>(exact_remainder_expansion(a, b, odd));
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
		// |x| - n |y| in [0, |y|), on the raw expansion: it may need a limb more than the budget,
		// and r - |y| below is then exact and small, where the budgeted r was not (#2054)
		std::vector<FpType> r = ereal_detail::exact_remainder_expansion(abs(x), ay, odd);
		// round n to nearest: past the half, or at the half with an odd n, take n + 1
		std::vector<FpType> twice(r);
		for (auto& v : twice) v = std::ldexp(v, 1);
		const int half = expansion_ops::compare_adaptive(twice, ay.limbs());
		if (half > 0 || (half == 0 && odd)) {
			std::vector<FpType> negy(ay.limbs());
			for (auto& v : negy) v = -v;
			r = expansion_ops::expansion_sum_normalized(r, negy);
		}
		const Real rr = ereal_detail::from_expansion<maxlimbs, FpType>(r);
		return x.signbit() ? ereal_detail::zero_with_sign_of(-rr, x) : ereal_detail::zero_with_sign_of(rr, x);
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
