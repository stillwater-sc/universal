#pragma once
// trigonometry.hpp: trigonometry function support for quad-double (qd) floating-point
// 
// algorithms and constants courtesy of Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numbers>
namespace sw { namespace universal {

	// value representing an angle expressed in radians
	// One radian is equivalent to 180/PI degrees

	// sine of an angle of x radians
	
	inline qd sin(const qd& x) {
		return qd(std::sin(double(x)));
	}

	// cosine of an angle of x radians	
	inline qd cos(const qd& x) {
		return qd(std::cos(double(x)));
	}

	// tangent of an angle of x radians
	inline qd tan(const qd& x) {
		return qd(std::tan(double(x)));
	}

	// cotangent of an angle of x radians
	inline qd atan(const qd& x) {
		return qd(std::atan(double(x)));
	}

	// Arc tangent with two parameters
	inline qd atan2(qd y, const qd& x) {
		return qd(std::atan2(double(y), double(x)));
	}

	// cosecant of an angle of x radians
	inline qd acos(const qd& x) {
		return qd(std::acos(double(x)));
	}

	// secant of an angle of x radians
	inline qd asin(const qd& x) {
		return qd(std::asin(double(x)));
	}

	// cotangent an angle of x radians
	inline qd cot(const qd& x) {
		return qd(std::tan(std::numbers::pi*2 - double(x)));
	}

	// secant of an angle of x radians
	inline qd sec(const qd& x) {
		return qd(1.0 / std::cos(double(x)));
	}

	// cosecant of an angle of x radians
	inline qd csc(const qd& x) {
		return qd(1.0 / std::sin(double(x)));
	}


}} // namespace sw::universal
