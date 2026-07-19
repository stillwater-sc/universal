// fractional.hpp: fractional, remainder, and sign-copy functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <limits>
#include <universal/number/efloat/math/truncate.hpp>
#include <universal/number/efloat/math/classify.hpp>

namespace sw { namespace universal {

// fmod: floating-point remainder x - n * y where n = trunc(x / y)
template<unsigned nlimbs>
constexpr efloat<nlimbs> fmod(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (y.iszero()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.iszero()) return x;
	if (x.isinf() || y.iszero()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}

	return x - trunc(x / y) * y;
}

// remainder / drem: IEEE-754 remainder x - n * y where n = rint(x / y) (round to nearest-even)
template<unsigned nlimbs>
constexpr efloat<nlimbs> remainder(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (y.iszero() || x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return nan;
	}
	if (x.iszero()) return x;

	return x - rint(x / y) * y;
}

template<unsigned nlimbs>
constexpr efloat<nlimbs> drem(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	return remainder(x, y);
}

// frac: return the fractional part of x (x - trunc(x))
template<unsigned nlimbs>
constexpr efloat<nlimbs> frac(const efloat<nlimbs>& x) {
	if (x.isnan() || x.isinf() || x.iszero()) return x;
	return x - trunc(x);
}

// modf: decomposes x into integer and fractional parts
template<unsigned nlimbs>
constexpr efloat<nlimbs> modf(const efloat<nlimbs>& x, efloat<nlimbs>* iptr) {
	if (x.isnan()) {
		if (iptr) iptr->setnan();
		return x;
	}
	if (x.isinf()) {
		if (iptr) *iptr = x;
		efloat<nlimbs> zero(0.0);
		zero.setsign(x.sign() == -1);
		return zero;
	}
	efloat<nlimbs> integer_part = trunc(x);
	if (iptr) {
		*iptr = integer_part;
	}
	efloat<nlimbs> frac_part = x - integer_part;
	frac_part.setsign(x.sign() == -1);
	return frac_part;
}

// copysign: returns x with its sign set to match y
template<unsigned nlimbs>
constexpr efloat<nlimbs> copysign(const efloat<nlimbs>& x, const efloat<nlimbs>& y) noexcept {
	efloat<nlimbs> res(x);
	res.setsign(y.sign() == -1);
	return res;
}

}} // namespace sw::universal
