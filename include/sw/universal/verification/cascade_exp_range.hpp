#pragma once
// cascade_exp_range.hpp: exp of a cascade type at the ends of the double range (#1709)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// exp of dd_cascade, td_cascade and qd_cascade returned +inf from x = 709.0 and 0 from
// x = -709.0, although e^x is finite up to ln(DBL_MAX) = 709.78 and nonzero down to
// ln(2^-1075) = -745.13.  This check, shared by the three types' exp_range tests, compares
// exp against the mpmath vectors of tools/generators/cascade_exp_range_gen.py:
//
//   past DBL_MAX     +inf
//   below DBL_MIN    exactly 0 where e^x rounds to 0; otherwise a positive subnormal within
//                    one subnormal ulp of the correctly rounded one, since scaling a
//                    multi-limb value into the subnormal range rounds the leading limb alone
//   in between       the relative error is below rel_tol, the type's own precision; near the
//                    bottom of the range the trailing limbs are themselves subnormal, so the
//                    error there is also allowed a few units of 2^-1074
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/verification/cascade_exp_range_vectors.hpp>

namespace sw { namespace universal {

template<typename Cascade>
int VerifyExpRange(const std::string& tag, double rel_tol, bool reportTestCases) {
	namespace cv = sw::universal::cascade_exp_verification;
	const double denorm_min = std::numeric_limits<double>::denorm_min();
	int fails = 0;
	double worst = 0.0;
	for (const auto& v : cv::exp_range) {
		const Cascade r = exp(Cascade(v.x));
		const double hi = double(r);
		bool ok = true;
		if (std::isinf(v.rounded)) {
			ok = std::isinf(hi) && hi > 0.0;
		}
		else if (v.limbs[0] == 0.0) {
			// zero exactly where e^x rounds to zero, positive where it does not: the underflow
			// boundary is the point of the check, so the ulp allowance never spans it
			if (v.rounded == 0.0) ok = (hi == 0.0);
			else ok = (hi > 0.0) && std::fabs(hi - v.rounded) <= denorm_min;
		}
		else {
			Cascade exact = Cascade(v.limbs[0]) + Cascade(v.limbs[1]) + Cascade(v.limbs[2]) + Cascade(v.limbs[3]);
			Cascade err = r - exact;
			const double abs_err = std::fabs(double(err));
			const double rel = abs_err / v.limbs[0];
			ok = std::isfinite(hi) && (rel <= rel_tol || abs_err <= 4.0 * denorm_min);
			if (abs_err > 4.0 * denorm_min && rel > worst) worst = rel;
		}
		if (!ok) {
			++fails;
			if (reportTestCases) {
				std::cerr << "FAIL: " << tag << " exp(" << std::setprecision(17) << v.x << ") = " << hi
				          << ", expected " << v.rounded << '\n';
			}
		}
	}
	if (reportTestCases) std::cout << tag << ": worst relative error in the normal range " << std::setprecision(3) << worst << '\n';
	return fails;
}

}} // namespace sw::universal
