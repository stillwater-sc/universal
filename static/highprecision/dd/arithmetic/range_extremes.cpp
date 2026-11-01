// range_extremes.cpp: multiplication and division at the top of the double range (#1881)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Dekker's two_prod splits each factor into 26-bit halves. For a factor within 2^-27 of 2^1024
// the high half rounds up to 2^1024 = inf, and for a product in the top binade a_hi * b_hi
// can pass DBL_MAX. Either way the residual came back NaN for a finite product, and every
// multi-double type that forms its residuals through the split returned NaN from * and /.
// This suite checks the error-free transformation itself against an fma oracle, at run time
// and in constant evaluation, and the multi-double types that consume it.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <random>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

namespace sw { namespace universal {

	// The residual's top-of-range values are fixed by exact arithmetic, so the constant-evaluated
	// branch of two_prod and two_sqr can be checked at compile time.
	constexpr double ConstexprResidual(double a, double b) { double r = 1.0; two_prod(a, b, r); return r; }
	constexpr double ConstexprSquareResidual(double a) { double r = 1.0; two_sqr(a, r); return r; }

	// DBL_MAX * 0.5 is exact. DBL_MAX * (1 - 2^-53) = (2^1024 - 2^971)(1 - 2^-53) = 2^1024 - 2^972 + 2^918,
	// which rounds to 2^1024 - 2^972 (one ulp below DBL_MAX) with residual 2^918.
	static_assert(ConstexprResidual(DBL_MAX, 0.5) == 0.0, "two_prod(DBL_MAX, 0.5) residual");
	static_assert(ConstexprResidual(DBL_MAX, 0x1.fffffffffffffp-1) == 0x1p918, "two_prod(DBL_MAX, 1 - 2^-53) residual");
	static_assert(ConstexprResidual(0x1p-512, DBL_MAX) == 0.0, "two_prod(2^-512, DBL_MAX) residual");
	// (2^512 - 2^459)^2 = 2^1024 - 2^972 + 2^918: the high part is exact, the residual 2^918
	static_assert(ConstexprSquareResidual(0x1.fffffffffffffp+511) == 0x1p918, "two_sqr(2^512 - 2^459) residual");

	inline bool SameDouble(double x, double y) {
		return (x == y) || (x != x && y != y);
	}

	// The residual r = a * b - p is exact, so it must match fma(a, b, -p) wherever fma rounds once.
	// Without a trusted fma it is still finite, and at most half an ulp of p.
	inline int CheckResidual(bool reportTestCases, double a, double b) {
		double p = a * b;
		if (!std::isfinite(p)) return 0;
		int nrOfFailures = 0;
		for (int order = 0; order < 2; ++order) {
			double r = 0.0;
			double pp = (order == 0) ? two_prod(a, b, r) : two_prod(b, a, r);
#if UNIVERSAL_TRUSTED_FMA
			double ref = std::fma(a, b, -p);
			bool ok = (pp == p) && SameDouble(r, ref);
#else
			double ref = std::ldexp(std::nextafter(std::abs(p), INFINITY) - std::abs(p), -1);
			bool ok = (pp == p) && std::isfinite(r) && std::abs(r) <= ref;
#endif
			if (!ok) {
				++nrOfFailures;
				if (reportTestCases) std::cerr << "FAIL: two_prod(" << to_hex(a) << ", " << to_hex(b) << ") residual " << r << " expected " << ref << '\n';
			}
		}
		if (a == b) {
			double r = 0.0;
			double pp = two_sqr(a, r);
#if UNIVERSAL_TRUSTED_FMA
			bool ok = (pp == p) && SameDouble(r, std::fma(a, a, -p));
#else
			bool ok = (pp == p) && std::isfinite(r);
#endif
			if (!ok) {
				++nrOfFailures;
				if (reportTestCases) std::cerr << "FAIL: two_sqr(" << to_hex(a) << ") residual " << r << '\n';
			}
		}
		return nrOfFailures;
	}

	// Random factor pairs whose product is finite, with one factor in the top 124 binades,
	// plus top-binade factors whose high 27 bits round up in the split, plus squares near 2^512.
	inline int VerifyTopOfRangeResiduals(bool reportTestCases, unsigned nrOfRandoms) {
		std::mt19937_64 rng(1881);
		std::uniform_real_distribution<double> significand(1.0, 2.0);
		std::uniform_int_distribution<int> big_exponent(900, 1023);
		std::uniform_int_distribution<int> small_exponent(-1074, 200);
		std::uniform_int_distribution<int> square_exponent(400, 511);
		int nrOfFailures = 0;
		for (unsigned i = 0; i < nrOfRandoms; ++i) {
			int ea = big_exponent(rng);
			double a = std::ldexp(significand(rng), ea);
			if (rng() & 1) a = -a;
			int eb = small_exponent(rng);
			if (eb > 1023 - ea) eb = 1023 - ea;
			double b = std::ldexp(significand(rng), eb);
			if (rng() & 1) b = -b;
			nrOfFailures += CheckResidual(reportTestCases, a, b);

			double t = std::ldexp(2.0 - std::ldexp(static_cast<double>(rng() % 4096), -52), ea - 1);
			nrOfFailures += CheckResidual(reportTestCases, t, std::ldexp(significand(rng), 1023 - ea - static_cast<int>(rng() % 3)));

			double s = std::ldexp(significand(rng), square_exponent(rng));
			nrOfFailures += CheckResidual(reportTestCases, s, s);
		}
		for (double b : { 0.5, 1.0, -0.75, 0x1p-52, 0x1p-512, 0x1p-1074, 0x1.fffffffffffffp-1 }) {
			nrOfFailures += CheckResidual(reportTestCases, DBL_MAX, b);
		}
		// Without a trusted fma CheckResidual only bounds |r|, so pin the exact run-time residuals
		// the static_asserts check in constant evaluation (volatile keeps the call from folding).
		{
			volatile double top = DBL_MAX, below_one = 0x1.fffffffffffffp-1, root = 0x1.fffffffffffffp+511;
			double r = 0.0;
			double p = two_prod(top, below_one, r);
			if (!(p == 0x1.ffffffffffffep+1023 && r == 0x1p918)) {
				++nrOfFailures;
				if (reportTestCases) std::cerr << "FAIL: two_prod(DBL_MAX, 1 - 2^-53) = " << to_hex(p) << " residual " << r << " expected 2^918\n";
			}
			r = 0.0;
			p = two_sqr(root, r);
			if (!(p == 0x1.ffffffffffffep+1023 && r == 0x1p918)) {
				++nrOfFailures;
				if (reportTestCases) std::cerr << "FAIL: two_sqr(2^512 - 2^459) = " << to_hex(p) << " residual " << r << " expected 2^918\n";
			}
		}
		return nrOfFailures;
	}

	// A multi-double value at the top of the range scaled by a power of two is exact in every type
	template<typename Real>
	int VerifyTopOfRangeArithmetic(bool reportTestCases, const std::string& type) {
		int nrOfFailures = 0;
		auto check = [&](const Real& result, const Real& expected, const char* op) {
			if (!(result == expected)) {
				++nrOfFailures;
				if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << op << " = " << to_binary(result) << " expected " << to_binary(expected) << '\n';
			}
		};
		const Real top(DBL_MAX);
		check(top * Real(0.5),        Real(0x1.fffffffffffffp+1022), "DBL_MAX * 0.5");
		check(Real(0.5) * top,        Real(0x1.fffffffffffffp+1022), "0.5 * DBL_MAX");
		check(top * Real(1.0),        top,                           "DBL_MAX * 1");
		check(top * Real(0x1p-512),   Real(0x1.fffffffffffffp+511),  "DBL_MAX * 2^-512");
		check(top / Real(2.0),        Real(0x1.fffffffffffffp+1022), "DBL_MAX / 2");
		check(top / Real(0x1p512),    Real(0x1.fffffffffffffp+511),  "DBL_MAX / 2^512");
		check(top / Real(1.0),        top,                           "DBL_MAX / 1");
		check(Real(0x1.8p1023) / Real(3.0), Real(0x1p1022),          "1.5 * 2^1023 / 3");
		Real square_root_top(0x1.fffffffffffffp+511);
		Real expected(0x1.ffffffffffffep+1023);
		expected += Real(0x1p918);
		check(square_root_top * square_root_top, expected,           "(2^512 - 2^459)^2");
		return nrOfFailures;
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite         = "multi-double arithmetic at the top of the double range";
	std::string test_tag           = "range extremes";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeResiduals(reportTestCases, 1000), "two_prod/two_sqr", "top of range residual");
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeArithmetic<dd>(reportTestCases, "dd"), "dd", "top of range");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeResiduals(reportTestCases, 10000), "two_prod/two_sqr", "top of range residual");
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeArithmetic<dd>(reportTestCases, "dd"), "dd", "top of range");
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeArithmetic<qd>(reportTestCases, "qd"), "qd", "top of range");
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeArithmetic<dd_cascade>(reportTestCases, "dd_cascade"), "dd_cascade", "top of range");
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeArithmetic<td_cascade>(reportTestCases, "td_cascade"), "td_cascade", "top of range");
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeArithmetic<qd_cascade>(reportTestCases, "qd_cascade"), "qd_cascade", "top of range");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeResiduals(reportTestCases, 100000), "two_prod/two_sqr", "top of range residual");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeResiduals(reportTestCases, 1000000), "two_prod/two_sqr", "top of range residual");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyTopOfRangeResiduals(reportTestCases, 4000000), "two_prod/two_sqr", "top of range residual");
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
