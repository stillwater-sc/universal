#pragma once
// hypot.hpp: hypot support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	namespace ereal_detail {

		// sqrt(sum of a[i]^2) for finite a[i], scaled by a power of two first.  The limbs share
		// the host's exponent range, so x * x overflows for |x| > 2^512 with double limbs:
		// hypot(1e200, 1e200) was inf - inf = NaN (#2006).  Scaling by 2^-e, with e the exponent
		// of the larger operand, is exact, and so is the scaling back.
		template<unsigned maxlimbs, typename FpType, std::size_t n>
		inline ereal<maxlimbs, FpType> scaled_hypot(const ereal<maxlimbs, FpType> (&a)[n]) {
			using Real = ereal<maxlimbs, FpType>;
			int e = 0;
			bool any = false;
			for (const Real& v : a) {
				if (v.iszero()) continue;
				const int ev = ilogb(v);
				if (!any || ev > e) e = ev;
				any = true;
			}
			if (!any) return Real(0.0);   // hypot(+-0, +-0) = +0
			Real sum(0.0);
			for (const Real& v : a) {
				if (v.iszero()) continue;
				const Real s = ldexp(v, -e);
				sum += s * s;
			}
			return ldexp(sqrt(sum), e);
		}

	}  // namespace ereal_detail

	// hypot: sqrt(x^2 + y^2) without overflow or underflow in the squares.
	// C99 F.10.4.3: hypot(+-inf, y) = +inf even when y is NaN; otherwise NaN for a NaN operand.
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> hypot(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (x.isinf() || y.isinf()) return ereal_detail::signed_inf<maxlimbs, FpType>(false);
		if (x.isnan() || y.isnan()) return ereal_detail::quiet_nan<maxlimbs, FpType>();
		const ereal<maxlimbs, FpType> a[2] = { x, y };
		return ereal_detail::scaled_hypot(a);
	}

	// hypot: sqrt(x^2 + y^2 + z^2), with the same scaling and special values (C++17 3-argument form)
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> hypot(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y, const ereal<maxlimbs, FpType>& z) {
		if (x.isinf() || y.isinf() || z.isinf()) return ereal_detail::signed_inf<maxlimbs, FpType>(false);
		if (x.isnan() || y.isnan() || z.isnan()) return ereal_detail::quiet_nan<maxlimbs, FpType>();
		const ereal<maxlimbs, FpType> a[3] = { x, y, z };
		return ereal_detail::scaled_hypot(a);
	}

}} // namespace sw::universal
