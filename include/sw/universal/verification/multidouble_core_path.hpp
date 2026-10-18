#pragma once
// multidouble_core_path.hpp: the math library reached without the umbrella header keeps full precision (#1724)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// qd/math/functions/sqrt.hpp defaulted QUADDOUBLE_NATIVE_SQRT to 0 when the umbrella qd.hpp had
// not set it to 1 first, so a translation unit that included core.hpp and mathlib.hpp, the
// layered path of #1334, got a sqrt that went through double: sqrt(qd(2))^2 - 2 = 2.7e-16 with
// zero trailing limbs.  The switches are gone; these checks pin that sqrt, the trigonometric
// functions and their inverses carry the full precision of the type on that path.  The caller
// includes the type's headers without its umbrella before this one.
#include <cmath>
#include <iostream>
#include <string>

namespace sw { namespace universal { namespace multidouble_verification {

	// |v| as a power of two, -inf for 0
	template<typename Real>
	double log2_abs(const Real& v) { return v.iszero() ? -INFINITY : std::log2(std::fabs(double(v))); }

	// bits: the precision of the type (106, 159 or 212); every residual must be within 2^-(bits - 8)
	template<typename Real>
	int VerifyCorePathPrecision(const std::string& tag, int bits, bool reportTestCases) {
		int fails = 0;
		const double limit = -(bits - 8);
		auto check = [&](const Real& residual, const std::string& what) {
			double e = log2_abs(residual);
			if (!(e <= limit)) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL: " << tag << ' ' << what << " residual 2^" << e << " > 2^" << limit << '\n';
			}
		};
		const Real one(1.0), two(2.0), three(3.0);
		Real s = sqrt(two);
		check(s * s - two, "sqrt(2)^2 - 2");
		Real t = sqrt(three);
		check(t * t - three, "sqrt(3)^2 - 3");
		for (double x : { 0.5, 1.0, 2.5, -7.0 }) {
			Real a(x), sn, cs;
			sincos(a, sn, cs);
			check(sqr(sn) + sqr(cs) - one, "sincos(" + std::to_string(x) + ") identity");
			check(sin(a) - sn, "sin(" + std::to_string(x) + ") vs sincos");
			check(cos(a) - cs, "cos(" + std::to_string(x) + ") vs sincos");
			check(tan(a) * cs - sn, "tan(" + std::to_string(x) + ") * cos - sin");
		}
		Real h(0.5);
		check(sin(asin(h)) - h, "sin(asin(0.5)) - 0.5");
		check(cos(acos(h)) - h, "cos(acos(0.5)) - 0.5");
		check(tan(atan(h)) - h, "tan(atan(0.5)) - 0.5");
		return fails;
	}

}}}  // namespace sw::universal::multidouble_verification
