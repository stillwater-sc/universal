#pragma once
// classify.hpp: classification functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	//  Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
	inline int fpclassify(const qd& a) {
		return (std::fpclassify(a[0]));
	}
	
	//  Determines if the given floating point number arg is a cfloative or negative infinity.
	// specialized for quad-double (qd)
	inline bool isinf(const qd& a) {
		return (std::fpclassify(a[0]) == FP_INFINITE);
	}

	//  Determines if the given floating point number arg is a not-a-number (NaN) value.
	// specialized for quad-double (qd)
	inline bool isnan(const qd& a) {
		return (std::fpclassify(a[0]) == FP_NAN);
	}

	//  Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
	// specialized for quad-double (qd)
	inline bool isfinite(const qd& a) {
		return (std::fpclassify(a[0]) != FP_INFINITE) && (std::fpclassify(a[0]) != FP_NAN);
	}

	//  Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
	// specialized for quad-double (qd)
	inline bool isnormal(const qd& a) {
		return (std::fpclassify(a[0]) == FP_NORMAL);
	}

	//  Determines if the given floating point number arg is denormal, i.e. is neither zero, normal, infinite, nor NaN.
	// specialized for quad-double (qd)
	inline bool isdenorm(const qd& a) {
		return (std::fpclassify(a[0]) == FP_SUBNORMAL);
	}

	inline bool iszero(const qd& a) {
		return (std::fpclassify(a[0]) == FP_ZERO);
	}

	inline bool signbit(qd const& a) {
		auto signA = std::copysign(1.0, a[0]);
		return signA < 0.0;
	}

}} // namespace sw::universal
