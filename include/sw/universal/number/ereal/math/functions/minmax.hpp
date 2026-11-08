#pragma once
// minmax.hpp: minmax support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// min and max follow C99 fmin and fmax (F.10.9.2, F.10.9.3): a NaN operand is ignored,
	// and the result is NaN only when both are.  They used to depend on the argument order:
	// max(1, NaN) was NaN and max(NaN, 1) was 1 (#2005).
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> min(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (x.isnan()) return y;
		if (y.isnan()) return x;
		return (y < x) ? y : x;
	}

	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> max(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (x.isnan()) return y;
		if (y.isnan()) return x;
		return (x < y) ? y : x;
	}

	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> fmin(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		return min(x, y);
	}

	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> fmax(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		return max(x, y);
	}

	// fdim: positive difference -- x - y if x > y, otherwise +0 (NaN if either is NaN)
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> fdim(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (x.isnan() || y.isnan()) return ereal<maxlimbs, FpType>(std::numeric_limits<double>::quiet_NaN());
		return (x > y) ? (x - y) : ereal<maxlimbs, FpType>(0.0);
	}

}} // namespace sw::universal
