#pragma once
// multidouble_special_division.hpp: division with an infinite or NaN operand for the multi-double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dd, qd, dd_cascade, td_cascade and qd_cascade follow IEEE-754 for a division with an infinite
// operand: a finite value divided by an infinity is a zero whose sign is the exclusive or of the
// operand signs, an infinity divided by a finite value is an infinity with that sign, and an
// infinity divided by an infinity is NaN. Before #1327 a finite value divided by an infinity came
// out NaN. The fix (PR #1337) came with a test under tests/ that no build compiled; these checks
// replace it, called from each type's arithmetic suite (#1942).
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace sw { namespace universal { namespace multidouble_verification {

	// the leading limb, which carries the sign of a zero or an infinity
	template<typename Real>
	double leading_limb(const Real& v) {
		if constexpr (requires { v.high(); }) return v.high();
		else return v[0];
	}

	template<typename Real>
	int VerifyDivisionByInfinity(const std::string& tag, bool reportTestCases) {
		int fails = 0;
		auto check = [&](bool ok, const std::string& what) {
			if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << tag << ' ' << what << '\n'; }
		};
		const Real pinf = std::numeric_limits<Real>::infinity();
		const Real ninf = -pinf;
		const Real nan  = std::numeric_limits<Real>::quiet_NaN();

		// a zero with the expected sign
		auto signedZero = [&](const Real& r, bool negative, const std::string& what) {
			check(r.iszero() && std::signbit(leading_limb(r)) == negative, what);
		};
		// an infinity with the expected sign
		auto signedInf = [&](const Real& r, bool negative, const std::string& what) {
			check(r.isinf() && std::signbit(leading_limb(r)) == negative, what);
		};

		// finite / +-inf = a zero, signed by the exclusive or of the operand signs
		for (double v : { 2.0, 1.0e-300, 1.0e300, 0.75 }) {
			const Real p(v), n(-v);
			signedZero(p / pinf, false, std::to_string(v) + " / +inf = +0");
			signedZero(n / pinf, true,  std::to_string(-v) + " / +inf = -0");
			signedZero(p / ninf, true,  std::to_string(v) + " / -inf = -0");
			signedZero(n / ninf, false, std::to_string(-v) + " / -inf = +0");
			// the compound form takes the same path
			Real q = p;
			q /= ninf;
			signedZero(q, true, std::to_string(v) + " /= -inf = -0");
		}
		// a value with every limb in use, not only the leading one
		{
			const Real third = Real(1.0) / Real(3.0);
			signedZero(third / pinf, false, "1/3 / +inf = +0");
			signedZero(-third / pinf, true, "-1/3 / +inf = -0");
		}
		// +-0 / +-inf = a zero, signed the same way
		{
			const Real pz(0.0), nz(-0.0);
			signedZero(pz / pinf, false, "+0 / +inf = +0");
			signedZero(pz / ninf, true,  "+0 / -inf = -0");
			signedZero(nz / pinf, true,  "-0 / +inf = -0");
			signedZero(nz / ninf, false, "-0 / -inf = +0");
		}
		// +-inf / finite = an infinity, signed the same way
		{
			const Real p(2.0), n(-2.0);
			signedInf(pinf / p, false, "+inf / 2 = +inf");
			signedInf(pinf / n, true,  "+inf / -2 = -inf");
			signedInf(ninf / p, true,  "-inf / 2 = -inf");
			signedInf(ninf / n, false, "-inf / -2 = +inf");
		}
		// +-inf / +-inf = NaN
		check((pinf / pinf).isnan(), "+inf / +inf = NaN");
		check((pinf / ninf).isnan(), "+inf / -inf = NaN");
		check((ninf / pinf).isnan(), "-inf / +inf = NaN");
		check((ninf / ninf).isnan(), "-inf / -inf = NaN");
		// a NaN operand stays NaN
		check((nan / pinf).isnan(), "NaN / +inf = NaN");
		check((pinf / nan).isnan(), "+inf / NaN = NaN");
		check((Real(2.0) / nan).isnan(), "2 / NaN = NaN");
		return fails;
	}

}}} // namespace sw::universal::multidouble_verification
