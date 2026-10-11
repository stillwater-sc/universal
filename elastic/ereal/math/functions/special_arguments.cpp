// special_arguments.cpp: ereal math functions at the arguments where their textbook forms fail
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each case below returned a wrong value, not an imprecise one, until #1668:
//
//   isone      true for 1 + 1e-20, which sent log(1 + 1e-20) to 0 and pow(1 + 1e-20, y) to 1
//   log        lost 100 bits at 1 + 1e-30, where e ln 2 cancelled the series
//   sin, cos   diverged for negative arguments beyond 2 pi: sin(-219.35) = -4.4e78
//   tanh       NaN for large arguments, from inf / inf
//   asinh,     0 for tiny arguments: their logarithms of 1 + x cancel completely
//   atanh
//   fmod,      off by |y| at an exact multiple, where the rounded quotient fell just short
//   remainder
//   pow        took y = 3 + 1e-30 for the integer 3
//
// The references are independent of the function under test: Taylor series for the tiny
// arguments, libm in double for the shape, and exact dyadic values for fmod and remainder.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	// bits of agreement between a value and its reference
	template<typename Real>
	double agreement(const Real& v, const Real& ref) {
		const Real d = abs(v - ref);
		if (d.iszero()) return 1.0e6;
		return -std::log2(double(d / abs(ref)));
	}

	// the rational coefficient n / d, exact to the working precision: a double literal such as
	// 1.0 / 6.0 would put a 2^-53 error into the x^2 term
	template<typename Real>
	Real q(double n, double d) { return Real(n) / Real(d); }

	// x (1 + c3 x^2 + c5 x^4 + c7 x^6), enough for |x| <= 1e-20 at ereal<8>'s precision
	template<typename Real>
	Real odd_series(const Real& x, const Real& c3, const Real& c5, const Real& c7) {
		const Real x2 = x * x;
		return x * (Real(1.0) + x2 * (c3 + x2 * (c5 + x2 * c7)));
	}

	// x - x^2/2 + x^3/3 - ... - x^8/8, enough for |x| <= 1e-20
	template<typename Real>
	Real log1p_series(const Real& x) {
		Real r(0.0);
		for (int k = 8; k >= 1; --k) r = q<Real>(1.0, k) - x * r;
		return x * r;
	}

	template<typename Real>
	int expect_bits(const char* label, double arg, const Real& v, const Real& ref, double bits, bool reportTestCases) {
		const double got = agreement(v, ref);
		if (got >= bits) return 0;
		if (reportTestCases) std::cout << "    FAIL " << label << "(" << arg << "): " << got << " bits, expected " << bits << '\n';
		return 1;
	}

	template<typename Real>
	int expect_near(const char* label, double arg, const Real& v, double ref, double tolerance, bool reportTestCases) {
		const double got = double(v);
		if (std::fabs(got - ref) <= tolerance * std::max(1.0, std::fabs(ref))) return 0;
		if (reportTestCases) std::cout << "    FAIL " << label << "(" << arg << ") = " << got << ", expected " << ref << '\n';
		return 1;
	}

	template<typename Real>
	int VerifyIsOne(bool reportTestCases) {
		int fails = 0;
		if (!Real(1.0).isone()) { ++fails; if (reportTestCases) std::cout << "    FAIL isone(1)\n"; }
		const Real near = Real(1.0) + Real(1.0e-20);
		if (near.isone()) { ++fails; if (reportTestCases) std::cout << "    FAIL isone(1 + 1e-20)\n"; }
		if ((Real(1.0) - Real(1.0e-300)).isone()) { ++fails; if (reportTestCases) std::cout << "    FAIL isone(1 - 1e-300)\n"; }
		return fails;
	}

	// log(1 + t) and log1p(t) against t - t^2/2 + t^3/3 - ..., and log(1 - t)
	template<typename Real>
	int VerifyLogNearOne(bool reportTestCases, double bits) {
		int fails = 0;
		for (double t : { 1.0e-20, 1.0e-30, std::ldexp(1.0, -100), 3.0e-50 }) {
			const Real x(t);
			const Real ref = log1p_series(x);
			fails += expect_bits("log(1 + t)", t, log(Real(1.0) + x), ref, bits, reportTestCases);
			fails += expect_bits("log1p", t, log1p(x), ref, bits, reportTestCases);
			fails += expect_bits("log(1 - t)", t, log(Real(1.0) - x), log1p_series(Real(-x)), bits, reportTestCases);
		}
		return fails;
	}

	// sinh, tanh, asinh and atanh at tiny arguments, against their Taylor series
	template<typename Real>
	int VerifyHyperbolicNearZero(bool reportTestCases, double bits) {
		int fails = 0;
		for (double t : { 1.0e-20, -1.0e-30, 3.1752805378007457e-50, -1.0e-100 }) {
			const Real x(t);
			fails += expect_bits("sinh",  t, sinh(x),  odd_series(x, q<Real>(1, 6), q<Real>(1, 120), q<Real>(1, 5040)), bits, reportTestCases);
			fails += expect_bits("tanh",  t, tanh(x),  odd_series(x, q<Real>(-1, 3), q<Real>(2, 15), q<Real>(-17, 315)), bits, reportTestCases);
			fails += expect_bits("asinh", t, asinh(x), odd_series(x, q<Real>(-1, 6), q<Real>(3, 40), q<Real>(-15, 336)), bits, reportTestCases);
			fails += expect_bits("atanh", t, atanh(x), odd_series(x, q<Real>(1, 3), q<Real>(1, 5), q<Real>(1, 7)), bits, reportTestCases);
		}
		// acosh(1 + t) = sqrt(2t) (1 - t/12 + 3t^2/160 - 5t^3/896), next term t^4
		const double t = std::ldexp(1.0, -100);
		const Real tt(t);
		const Real ref = sqrt(Real(2.0) * tt) * (Real(1.0) - tt * (q<Real>(1, 12) - tt * (q<Real>(3, 160) - tt * q<Real>(5, 896))));
		fails += expect_bits("acosh(1 + t)", t, acosh(Real(1.0) + tt), ref, bits, reportTestCases);
		return fails;
	}

	// the shape across the range, against libm: large tanh, huge asinh and acosh, and the
	// moderate arguments the new branches meet at 1/2
	template<typename Real>
	int VerifyHyperbolicRange(bool reportTestCases) {
		int fails = 0;
		for (double v : { 0.25, 0.4999, 0.5, 0.75, 3.0, 20.0, 3576.4793954088896, 1.0e300 }) {
			fails += expect_near("tanh", v, tanh(Real(v)), std::tanh(v), 1.0e-15, reportTestCases);
			fails += expect_near("tanh", -v, tanh(Real(-v)), std::tanh(-v), 1.0e-15, reportTestCases);
			if (v < 700.0) {
				fails += expect_near("sinh", v, sinh(Real(v)), std::sinh(v), 1.0e-15, reportTestCases);
				fails += expect_near("sinh", -v, sinh(Real(-v)), std::sinh(-v), 1.0e-15, reportTestCases);
			}
			fails += expect_near("asinh", v, asinh(Real(v)), std::asinh(v), 1.0e-15, reportTestCases);
			fails += expect_near("asinh", -v, asinh(Real(-v)), std::asinh(-v), 1.0e-15, reportTestCases);
			if (v < 1.0) fails += expect_near("atanh", -v, atanh(Real(-v)), std::atanh(-v), 1.0e-15, reportTestCases);
			else         fails += expect_near("acosh", v, acosh(Real(v)), std::acosh(v), 1.0e-15, reportTestCases);
		}
		return fails;
	}

	// sin, cos and tan at negative arguments of several magnitudes: the odd and even symmetry
	// exactly, and libm's value in double
	template<typename Real>
	int VerifyTrigNegative(bool reportTestCases) {
		int fails = 0;
		for (double v : { 1.0, 7.0, 219.34981640059584, 5549.5471854265415, 123456.789, 1.0e9 }) {
			const Real x(v), mx(-v);
			if (!(sin(mx) == -sin(x))) { ++fails; if (reportTestCases) std::cout << "    FAIL sin(-x) != -sin(x) at " << v << '\n'; }
			if (!(cos(mx) == cos(x)))  { ++fails; if (reportTestCases) std::cout << "    FAIL cos(-x) != cos(x) at " << v << '\n'; }
			fails += expect_near("sin", -v, sin(mx), std::sin(-v), 1.0e-15, reportTestCases);
			fails += expect_near("cos", -v, cos(mx), std::cos(-v), 1.0e-15, reportTestCases);
			fails += expect_near("tan", -v, tan(mx), std::tan(-v), 1.0e-14, reportTestCases);
		}
		// huge arguments, beyond what the stored pi resolves: the reduction must still finish,
		// in a bounded number of passes, with a value in [-1, 1]
		for (double v : { 1.0e300, -1.0e300, std::numeric_limits<double>::max() }) {
			const double s = double(sin(Real(v))), c = double(cos(Real(v)));
			if (!(std::fabs(s) <= 1.0) || !(std::fabs(c) <= 1.0)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL sin/cos(" << v << ") = " << s << ", " << c << '\n';
			}
		}
		return fails;
	}

	// fmod and remainder at exact multiples and exact halves of y, in all four sign combinations
	template<typename Real>
	int VerifyFmodRemainderExact(bool reportTestCases) {
		int fails = 0;
		auto fail = [&](const char* what, double x, double y, const Real& got, double expected) {
			++fails;
			if (reportTestCases) std::cout << "    FAIL " << what << "(" << x << ", " << y << ") = " << double(got) << ", expected " << expected << '\n';
		};
		// y with a full 53-bit significand, so that k y is rarely a short binary fraction
		for (double y0 : { 5.1200389862060547e-05, 0.1, 3.0, 1.0 / 3.0 }) {
			for (int k = 1; k <= 2048; k = 2 * k + 1) {
				for (int sx = 0; sx < 2; ++sx) for (int sy = 0; sy < 2; ++sy) {
					const Real y(sy ? -y0 : y0);
					const Real x = Real(sx ? -double(k) : double(k)) * Real(y0);           // exact
					const double xd = double(x), yd = double(y);
					const Real f = fmod(x, y), r = remainder(x, y);
					if (!f.iszero()) fail("fmod", xd, yd, f, 0.0);
					if (!r.iszero()) fail("remainder", xd, yd, r, 0.0);
					// x + y/2: fmod is |y|/2 with the sign of x, remainder the half that makes n even
					const Real half = Real(y0) * Real(0.5);
					const Real xh = sx ? x - half : x + half;
					const Real fh = fmod(xh, y);
					const Real expect_f = sx ? -half : half;
					if (!(fh == expect_f)) fail("fmod", double(xh), yd, fh, double(expect_f));
					// |xh / y| = k + 1/2 with k odd: n = k + 1, and the remainder is opposite to x
					const Real rh = remainder(xh, y);
					const Real expect_r = sx ? half : -half;
					if (!(rh == expect_r)) fail("remainder", double(xh), yd, rh, double(expect_r));
				}
			}
		}
		// the case that exposed it
		const Real f = fmod(Real(0.41943359375), Real(5.1200389862060547e-05));
		if (!f.iszero()) fail("fmod", 0.41943359375, 5.1200389862060547e-05, f, 0.0);
		return fails;
	}

	// pow with an exponent whose leading limb is an integer, and a base whose leading limb is 1
	template<typename Real>
	int VerifyPowNearInteger(bool reportTestCases) {
		int fails = 0;
		// 2^(3 + 1e-30) - 8 = 8 (2^(1e-30) - 1) ~= 8 ln2 1e-30
		const Real y = Real(3.0) + Real(1.0e-30);
		const Real excess = pow(Real(2.0), y) - Real(8.0);
		fails += expect_near("pow(2, 3 + 1e-30) - 8, scaled", 3.0, excess * Real(1.0e30), 8.0 * 0.69314718055994531, 1.0e-14, reportTestCases);
		// (1 + 1e-20)^(1e20) = e (1 - 1e-20 / 2 + ...)
		const Real base = Real(1.0) + Real(1.0e-20);
		fails += expect_near("pow(1 + 1e-20, 1e20)", 1.0e20, pow(base, Real(1.0e20)), 2.718281828459045, 1.0e-15, reportTestCases);
		return fails;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib at special arguments";
	std::string test_tag    = "special arguments";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Real = ereal<8>;   // about 424 bits; the near-zero references hold 380 of them

#if MANUAL_TESTING

	std::cout << "sin(-219.34981640059584) = " << double(sin(Real(-219.34981640059584))) << " (libm " << std::sin(-219.34981640059584) << ")\n";
	std::cout << "tanh(3576.4793954088896) = " << double(tanh(Real(3576.4793954088896))) << '\n';
	std::cout << "asinh(1e-50)             = " << double(asinh(Real(1.0e-50))) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIsOne<Real>(reportTestCases), "isone(ereal<8>)", "isone");
	nrOfFailedTestCases += ReportTestResult(VerifyLogNearOne<Real>(reportTestCases, 380.0), "log near 1", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyHyperbolicNearZero<Real>(reportTestCases, 380.0), "hyperbolic near 0", "hyperbolic");
	nrOfFailedTestCases += ReportTestResult(VerifyHyperbolicRange<Real>(reportTestCases), "hyperbolic vs libm", "hyperbolic");
	nrOfFailedTestCases += ReportTestResult(VerifyTrigNegative<Real>(reportTestCases), "trig at negative x", "trigonometry");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainderExact<Real>(reportTestCases), "fmod/remainder exact", "fractional");
	nrOfFailedTestCases += ReportTestResult(VerifyPowNearInteger<Real>(reportTestCases), "pow near integer", "pow");
#endif

#if REGRESSION_LEVEL_2
	// the default configuration, with its own precision
	nrOfFailedTestCases += ReportTestResult(VerifyIsOne<ereal<>>(reportTestCases), "isone(ereal<>)", "isone");
	nrOfFailedTestCases += ReportTestResult(VerifyTrigNegative<ereal<>>(reportTestCases), "trig at negative x, ereal<>", "trigonometry");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainderExact<ereal<>>(reportTestCases), "fmod/remainder exact, ereal<>", "fractional");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
