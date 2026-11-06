#pragma once
// constants.hpp: the constants of faithful's elementary functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each constant is four doubles, peeled off the exact value one at a time (#1955): ln 2 matches the
// shared multi-double table, ln 10 was generated the same way, with Python's decimal module at
// 120 digits. split<Real>() peels the doubles again into FloatingPointType parts, so a float host
// gets ln 2 to about 100 bits, and pair<Real>() rounds the constant to a faithful.
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/functions/fractional.hpp>   // faithful_detail::exact_expansion

namespace sw { namespace universal { namespace faithful_detail {

	namespace constants {
		inline constexpr double ln2[4]  = { 0x1.62e42fefa39efp-1, 0x1.abc9e3b39803fp-56, 0x1.7b57a079a1934p-111, -0x1.ace93a4ebe5d1p-165 };
		inline constexpr double ln10[4] = { 0x1.26bb1bbb55516p+1, -0x1.f48ad494ea3e9p-53, -0x1.9ebae3ae0260cp-107, -0x1.2d10378be1cf1p-161 };
	}

	// the first N FloatingPointType parts of a four-double constant: nonoverlapping, the largest
	// first, each the FloatingPointType nearest what the earlier ones leave. The doubles are held
	// in an exact expansion, and each part is subtracted from it exactly.
	template<typename Real, int N>
	struct split_constant {
		Real part[N];
	};
	template<typename Real, int N>
	inline split_constant<Real, N> split(const double (&c)[4]) {
		split_constant<Real, N> s{};
		exact_expansion<double> rest;
		for (int j = 3; j >= 0; --j) rest.add(c[j]);
		for (int i = 0; i < N; ++i) {
			const Real p = static_cast<Real>(rest.approx());
			s.part[i] = p;
			rest.add(-static_cast<double>(p));
		}
		return s;
	}

	// the constant rounded to a faithful
	template<typename Real>
	inline faithful<Real> pair(const double (&c)[4]) {
		const split_constant<Real, 3> s = split<Real, 3>(c);
		return faithful<Real>(s.part[0], s.part[1]) + faithful<Real>(s.part[2], Real(0));
	}

}}} // namespace sw::universal::faithful_detail
