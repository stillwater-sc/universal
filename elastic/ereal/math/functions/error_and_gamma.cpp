// error_and_gamma.cpp: regression tests for the error and gamma functions of
// ereal adaptive-precision.
//
// Standardized as part of #950 (math/functions refactor).
//
// erf, erfc, tgamma and lgamma are double-precision shims: each evaluates the std:: function
// on double(x) (error_and_gamma.hpp), so a result carries 53 bits whatever the configuration.
// Measured exactly against the 340-digit references and against ereal<19> (#2008):
//   tgamma(1/2)^2 vs pi, 2 / tgamma(1/2) and 1 / tgamma(3/2) vs 2/sqrt(pi): 53 bits
//   erf(x) + erfc(x) vs 1, x in {0.3, 1, 2, 2.5, -1.7, 3}: 52-55 bits (absolute)
//   tgamma(x + 1) vs x tgamma(x), x in {1.5, 2.7, 4.2}: 53-55 bits
//   lgamma(x) vs log(tgamma(x)), x in {0.5, 1.5, 3, 5, 10}: 50-54 bits
//   ereal<8> and ereal<19> return the identical value: the extra limbs are never used
// The old checks compared through double at 1e-12..1e-14 and passed this.  The precision
// checks are left out rather than loosened to 53 bits: they come back, at
// ereal_bits_of<Real> less a measured margin, when these functions are native (#2017).
// What stays is exact: the fixed points, parity, the factorials, the special values.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <random>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>

namespace {
	using namespace sw::universal;

	// exact: the dyadic values are equal, whatever the limb structure
	template<typename Real>
	bool exactly(const Real& a, const Real& b) { return agreed_bits_absolute(a, b) == kExactAgreement; }

	// erf/erfc: fixed point, parity, and the special values.
	template<typename Real>
	int VerifyErrorFunction(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// erf(0) == 0, erfc(0) == 1
		if (!erf(Real(0.0)).iszero()) {
			if (reportTestCases) std::cout << "    FAIL erf(0) != 0\n";
			++nrOfFailedTestCases;
		}
		if (!exactly(erfc(Real(0.0)), Real(1.0))) {
			if (reportTestCases) std::cout << "    FAIL erfc(0) != 1\n";
			++nrOfFailedTestCases;
		}
		// parity: erf(-x) == -erf(x), exactly
		for (double v : {0.3, 1.0, 2.5}) {
			Real x(v);
			if (!exactly(erf(-x), -erf(x))) {
				if (reportTestCases) std::cout << "    FAIL erf parity at " << v << "\n";
				++nrOfFailedTestCases;
			}
		}
		// erf(x) + erfc(x) == 1 and erf against a reference: left out, erf/erfc are double
		// shims (53 bits, see the top of the file)

		// special values: erf(+/-Inf) = +/-1, erfc(+/-Inf) = 0/2, NaN propagates
		double pinf = std::numeric_limits<double>::infinity();
		double qnan = std::numeric_limits<double>::quiet_NaN();
		if (!exactly(erf(Real(pinf)), Real(1.0)) || !exactly(erf(Real(-pinf)), Real(-1.0))) {
			if (reportTestCases) std::cout << "    FAIL erf(+/-Inf) != +/-1\n";
			++nrOfFailedTestCases;
		}
		if (!erfc(Real(pinf)).iszero() || !exactly(erfc(Real(-pinf)), Real(2.0))) {
			if (reportTestCases) std::cout << "    FAIL erfc(+/-Inf) != 0/2\n";
			++nrOfFailedTestCases;
		}
		if (!erf(Real(qnan)).isnan() || !erfc(Real(qnan)).isnan()) {
			if (reportTestCases) std::cout << "    FAIL erf/erfc(NaN) not NaN\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// tgamma: factorial fixed points and the special values.
	template<typename Real>
	int VerifyTgamma(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// tgamma(n) == (n-1)!, exactly
		double fact = 1.0;  // (n-1)! ; starts at 0!=1 for n=1
		for (int n = 1; n <= 7; ++n) {
			if (n > 1) fact *= (n - 1);
			if (!exactly(tgamma(Real(double(n))), Real(fact))) {
				if (reportTestCases) std::cout << "    FAIL tgamma(" << n << ") != " << fact << "\n";
				++nrOfFailedTestCases;
			}
		}
		// tgamma(1/2)^2 == pi and the recurrence tgamma(x+1) == x*tgamma(x): left out,
		// tgamma is a double shim (53 bits, see the top of the file)

		// special values: tgamma(+Inf) = +Inf, tgamma(NaN) = NaN
		{
			double pinf = std::numeric_limits<double>::infinity();
			Real rinf = tgamma(Real(pinf));
			if (!rinf.isinf() || rinf.signbit()) {
				if (reportTestCases) std::cout << "    FAIL tgamma(+Inf) != +Inf\n";
				++nrOfFailedTestCases;
			}
			if (!tgamma(Real(std::numeric_limits<double>::quiet_NaN())).isnan()) {
				if (reportTestCases) std::cout << "    FAIL tgamma(NaN) != NaN\n";
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// lgamma: fixed points and the special value.
	template<typename Real>
	int VerifyLgamma(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// lgamma(1) == 0 and lgamma(2) == 0
		if (!lgamma(Real(1.0)).iszero() || !lgamma(Real(2.0)).iszero()) {
			if (reportTestCases) std::cout << "    FAIL lgamma(1) or lgamma(2) != 0\n";
			++nrOfFailedTestCases;
		}
		// lgamma(x) == log(tgamma(x)) and agreement with a reference: left out, lgamma and
		// tgamma are double shims (53 bits, see the top of the file)

		// special value: lgamma(+Inf) = +Inf
		Real linf = lgamma(Real(std::numeric_limits<double>::infinity()));
		if (!linf.isinf() || linf.signbit()) {
			if (reportTestCases) std::cout << "    FAIL lgamma(+Inf) != +Inf\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// Property fuzzer: erf parity and monotonicity over random inputs.  The complement
	// erf + erfc == 1 and the gamma recurrence are left out with the other precision checks.
	template<typename Real>
	int VerifyErrorGammaFuzz(bool reportTestCases, unsigned nrIterations) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(0xC1A55'1FFEULL);
		std::uniform_real_distribution<double> edist(-3.0, 3.0);
		for (unsigned i = 0; i < nrIterations; ++i) {
			double e = edist(rng);
			Real x(e);
			if (!exactly(erf(-x), -erf(x))) {
				if (reportTestCases) std::cout << "    FAIL erf parity at " << e << "\n";
				++nrOfFailedTestCases;
			}
			if (!(erf(x) < erf(x + Real(0.5)))) {   // erf strictly increasing
				if (reportTestCases) std::cout << "    FAIL erf monotonic at " << e << "\n";
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

}  // anonymous namespace

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib error and gamma function validation";
	std::string test_tag    = "erf/erfc/tgamma/lgamma";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<>>(reportTestCases), "erf/erfc(ereal)", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#	if REGRESSION_LEVEL_1
	test_tag = "erf/erfc";
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<>>(reportTestCases), "erf/erfc(ereal)", test_tag);

	test_tag = "tgamma";
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma<ereal<>>(reportTestCases), "tgamma(ereal)", test_tag);

	test_tag = "lgamma";
	nrOfFailedTestCases += ReportTestResult(VerifyLgamma<ereal<>>(reportTestCases), "lgamma(ereal)", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
	test_tag = "error/gamma fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyErrorGammaFuzz<ereal<>>(reportTestCases, 50), "error/gamma property fuzz", test_tag);
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
