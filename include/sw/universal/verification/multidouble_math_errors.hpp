#pragma once
// multidouble_math_errors.hpp: domain errors and special cases of the multi-double math libraries (#1717)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dd, qd, dd_cascade, td_cascade and qd_cascade report an invalid argument the way the std::
// functions do: errno = EDOM and a quiet NaN, with no message.  The cases with a defined value
// follow std as well: atan2(+-0, +-0) and npwr(0, 0) = 1.  A finite argument too large for the
// trigonometric argument reduction returns a NaN with errno = ERANGE, or throws under the type's
// *_THROW_ARITHMETIC_EXCEPTION.  These checks hold for every one of the five types, which share
// the function names; they need the non-throwing configuration, except VerifyTrigReductionThrows.
#include <cerrno>
#include <cmath>
#include <iostream>
#include <string>

namespace sw { namespace universal { namespace multidouble_verification {

	// the leading limb, which carries the sign of a zero
	template<typename Real>
	double lead(const Real& v) {
		if constexpr (requires { v.high(); }) return v.high();
		else return v[0];
	}

	// a quiet NaN, with errno as expected afterwards (0: untouched)
	template<typename Real>
	bool isquiet(const Real& v) { return v.isnan(NAN_TYPE_QUIET); }

	template<typename Real>
	int VerifyMathErrors(const std::string& tag, bool reportTestCases) {
		int fails = 0;
		auto check = [&](bool ok, const std::string& what) {
			if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << tag << ' ' << what << '\n'; }
		};
		// evaluate f with errno cleared, and check a quiet NaN and the errno it leaves
		auto nan_with = [&](auto f, int want_errno, const std::string& what) {
			errno = 0;
			Real r = f();
			int e = errno;
			check(isquiet(r) && e == want_errno, what + (isquiet(r) ? "" : " is not a quiet NaN") +
			      (e == want_errno ? "" : " errno " + std::to_string(e) + " != " + std::to_string(want_errno)));
		};

		const Real one(1.0), two(2.0), pzero(0.0), nzero(-0.0), inf(SpecificValue::infpos), nan(SpecificValue::qnan);
		const Real big(1.0e300);

		// domain errors: EDOM and a quiet NaN
		nan_with([&] { return log(-one); },          EDOM, "log(-1)");
		nan_with([&] { return log2(-one); },         EDOM, "log2(-1)");
		nan_with([&] { return log10(-one); },        EDOM, "log10(-1)");
		nan_with([&] { return log1p(-two); },        EDOM, "log1p(-2)");
		nan_with([&] { return sqrt(-one); },         EDOM, "sqrt(-1)");
		nan_with([&] { return nroot(-one, 2); },     EDOM, "nroot(-1, 2)");
		nan_with([&] { return nroot(two, 0); },      EDOM, "nroot(2, 0)");
		nan_with([&] { return nroot(two, -3); },     EDOM, "nroot(2, -3)");
		nan_with([&] { return asin(two); },          EDOM, "asin(2)");
		nan_with([&] { return acos(-two); },         EDOM, "acos(-2)");
		nan_with([&] { return sin(inf); },           EDOM, "sin(inf)");
		nan_with([&] { return cos(-inf); },          EDOM, "cos(-inf)");
		nan_with([&] { return tan(inf); },           EDOM, "tan(inf)");
		nan_with([&] { Real s, c; sincos(inf, s, c); check(isquiet(c), "sincos(inf) cos"); return s; }, EDOM, "sincos(inf)");

		// a NaN argument is not an error: a quiet NaN, errno untouched
		nan_with([&] { return sin(nan); }, 0, "sin(nan)");
		nan_with([&] { return cos(nan); }, 0, "cos(nan)");
		nan_with([&] { Real s, c; sincos(nan, s, c); check(isquiet(c), "sincos(nan) cos"); return s; }, 0, "sincos(nan)");

		// a finite argument too large to reduce: ERANGE and a quiet NaN.  sincos (and tan
		// through it) indexed its table with std::abs(INT_MIN) here and crashed.
		nan_with([&] { return sin(big); },  ERANGE, "sin(1e300)");
		nan_with([&] { return cos(-big); }, ERANGE, "cos(-1e300)");
		nan_with([&] { return tan(big); },  ERANGE, "tan(1e300)");
		nan_with([&] { Real s, c; sincos(big, s, c); check(isquiet(c), "sincos(1e300) cos"); return s; }, ERANGE, "sincos(1e300)");

		// the cases std defines: atan2 of a zero is signed as y, and pi for a negative x
		auto signed_value = [&](const Real& r, double want, const std::string& what) {
			bool ok = (double(r) == want) && (std::signbit(lead(r)) == std::signbit(want));
			check(ok, what + " = " + std::to_string(double(r)) + (std::signbit(lead(r)) ? " (negative)" : ""));
		};
		const double pi = 3.141592653589793;
		errno = 0;
		signed_value(atan2(pzero, pzero), 0.0,  "atan2(+0, +0)");
		signed_value(atan2(nzero, pzero), -0.0, "atan2(-0, +0)");
		signed_value(atan2(pzero, nzero), pi,   "atan2(+0, -0)");
		signed_value(atan2(nzero, nzero), -pi,  "atan2(-0, -0)");
		signed_value(atan2(pzero, one),   0.0,  "atan2(+0, 1)");
		signed_value(atan2(nzero, one),   -0.0, "atan2(-0, 1)");
		signed_value(atan2(pzero, -one),  pi,   "atan2(+0, -1)");
		signed_value(atan2(nzero, -one),  -pi,  "atan2(-0, -1)");
		signed_value(npwr(pzero, 0),      1.0,  "npwr(0, 0)");
		signed_value(npwr(two, 0),        1.0,  "npwr(2, 0)");
		check(errno == 0, "atan2/npwr of zeros set errno " + std::to_string(errno));

		// and the ordinary values are untouched
		check(std::fabs(double(sin(Real(1.0))) - std::sin(1.0)) < 1.0e-15, "sin(1)");
		check(std::fabs(double(sqrt(two)) - std::sqrt(2.0)) < 1.0e-15, "sqrt(2)");
		check(std::fabs(double(nroot(Real(-8.0), 3)) + 2.0) < 1.0e-15, "nroot(-8, 3)");
		return fails;
	}

	// under the type's *_THROW_ARITHMETIC_EXCEPTION a finite argument too large to reduce throws
	// TooLarge, and a negative sqrt argument throws NegativeSqrt; inf is still a NaN, as std
	template<typename Real, typename TooLarge, typename NegativeSqrt>
	int VerifyTrigReductionThrows(const std::string& tag, bool reportTestCases) {
		int fails = 0;
		auto expect_throw = [&](auto f, const std::string& what) {
			bool thrown = false;
			try { f(); } catch (const TooLarge&) { thrown = true; }
			if (!thrown) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << tag << ' ' << what << " did not throw\n"; }
		};
		const Real big(1.0e300), inf(SpecificValue::infpos);
		expect_throw([&] { return sin(big); }, "sin(1e300)");
		expect_throw([&] { return cos(big); }, "cos(1e300)");
		expect_throw([&] { Real s, c; sincos(big, s, c); }, "sincos(1e300)");
		expect_throw([&] { return tan(big); }, "tan(1e300)");
		bool thrown = false;
		try { (void)sqrt(Real(-1.0)); } catch (const NegativeSqrt&) { thrown = true; }
		if (!thrown) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << tag << " sqrt(-1) did not throw\n"; }
		errno = 0;
		if (!(sin(inf).isnan() && errno == EDOM)) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << tag << " sin(inf) is not NaN with EDOM\n"; }
		return fails;
	}

}}}  // namespace sw::universal::multidouble_verification
