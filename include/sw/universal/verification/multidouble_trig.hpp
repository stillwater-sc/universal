#pragma once
// multidouble_trig.hpp: sin, cos, sincos and tan of the multi-double types against mpmath (#1723)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The vectors of multidouble_trig_vectors.hpp (tools/generators/multidouble_trig_gen.py) reach
// DBL_MAX and come within a few ulps of multiples of pi/2, where a reduction by a stored 2pi
// lost every digit.  Each result is checked against the reference with a relative error bound
// of 2^-(bits - slack), bits being the precision of the type.  A vector whose argument has more
// limbs than the type holds is skipped: the type cannot represent that argument.
#include <cmath>
#include <iostream>
#include <string>
#include <universal/verification/multidouble_trig_vectors.hpp>

namespace sw { namespace universal { namespace multidouble_verification {

	template<typename Real>
	Real from_limbs(const double (&l)[4]) {
		return ((Real(l[3]) + Real(l[2])) + Real(l[1])) + Real(l[0]);
	}

	// log2 of the relative error of v against ref (-inf when exact)
	template<typename Real>
	double rel_error_log2(const Real& v, const Real& ref) {
		const Real d = v - ref;
		if (d.iszero()) return -INFINITY;
		return std::log2(std::fabs(double(d))) - std::log2(std::fabs(double(ref)));
	}

	// limbs: how many doubles the type holds; bits: its precision; slack: bits allowed lost
	template<typename Real>
	int VerifyTrigVectors(const std::string& tag, int limbs, int bits, int slack, bool reportTestCases, double* worst = nullptr) {
		int fails = 0;
		double maxerr = -INFINITY;
		const double limit = -(bits - slack);
		for (const auto& v : trig_vectors) {
			if (v.n > limbs) continue;
			const Real x = from_limbs<Real>(v.x);
			Real s, c;
			sincos(x, s, c);
			const struct { const char* name; Real got; const double (&ref)[4]; } cases[] = {
				{ "sin", sin(x), v.sin }, { "cos", cos(x), v.cos },
				{ "sincos.sin", s, v.sin }, { "sincos.cos", c, v.cos }, { "tan", tan(x), v.tan },
			};
			for (const auto& k : cases) {
				const double e = rel_error_log2(k.got, from_limbs<Real>(k.ref));
				if (e > maxerr) maxerr = e;
				if (!(e <= limit)) {
					++fails;
					if (reportTestCases) {
						std::cerr << "FAIL: " << tag << ' ' << k.name << "(x = " << v.x[0] << " + " << v.x[1]
						          << ", " << v.n << " limbs) relative error 2^" << e << " > 2^" << limit << '\n';
					}
				}
			}
		}
		if (worst) *worst = maxerr;
		return fails;
	}

}}}  // namespace sw::universal::multidouble_verification
