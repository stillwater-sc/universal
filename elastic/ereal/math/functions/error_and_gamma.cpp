// error_and_gamma.cpp: regression tests for the error and gamma functions of
// ereal adaptive-precision.
//
// Standardized as part of #950 (math/functions refactor). The previous file was
// a Phase-0 MANUAL_TESTING smoke stub; erf/erfc/tgamma/lgamma are in fact
// implemented and accurate, so this exercises them with hand-curated identities
// and an oracle-free property fuzzer (the precision-lifting self-oracle remains
// scoped to #954).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	template<typename Real>
	bool close_rel(const Real& x, const Real& y, double relTol, double absTol = 1.0e-15) {
		double a = double(x), b = double(y);
		double diff = std::abs(a - b);
		if (diff == 0.0) return true;
		double scale = std::max(std::abs(a), std::abs(b));
		return diff <= std::max(absTol, relTol * scale);
	}

	// erf/erfc: fixed point, parity, and the erf(x)+erfc(x)==1 complement.
	template<typename Real>
	int VerifyErrorFunction(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// erf(0) == 0, erfc(0) == 1
		if (!erf(Real(0.0)).iszero()) {
			if (reportTestCases) std::cout << "    FAIL erf(0) != 0\n"; ++nrOfFailedTestCases;
		}
		if (!close_rel(erfc(Real(0.0)), Real(1.0), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL erfc(0) != 1\n"; ++nrOfFailedTestCases;
		}
		// erf matches the std::erf reference for representative points
		for (double v : {0.5, 1.0, 2.0}) {
			if (!close_rel(erf(Real(v)), Real(std::erf(v)), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL erf(" << v << ") mismatch\n"; ++nrOfFailedTestCases;
			}
		}
		// parity: erf(-x) == -erf(x); complement: erf(x)+erfc(x) == 1
		for (double v : {0.3, 1.0, 2.5}) {
			Real x(v);
			if (!close_rel(erf(-x), -erf(x), 1.0e-14)) {
				if (reportTestCases) std::cout << "    FAIL erf parity at " << v << "\n"; ++nrOfFailedTestCases;
			}
			if (!close_rel(erf(x) + erfc(x), Real(1.0), 1.0e-14)) {
				if (reportTestCases) std::cout << "    FAIL erf+erfc != 1 at " << v << "\n"; ++nrOfFailedTestCases;
			}
		}
		// special values: erf(+/-Inf) = +/-1, erfc(+/-Inf) = 0/2, NaN propagates
		double pinf = std::numeric_limits<double>::infinity();
		double qnan = std::numeric_limits<double>::quiet_NaN();
		if (!close_rel(erf(Real(pinf)), Real(1.0), 1.0e-14) || !close_rel(erf(Real(-pinf)), Real(-1.0), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL erf(+/-Inf) != +/-1\n"; ++nrOfFailedTestCases;
		}
		if (!erfc(Real(pinf)).iszero() || !close_rel(erfc(Real(-pinf)), Real(2.0), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL erfc(+/-Inf) != 0/2\n"; ++nrOfFailedTestCases;
		}
		if (!std::isnan(double(erf(Real(qnan)))) || !std::isnan(double(erfc(Real(qnan))))) {
			if (reportTestCases) std::cout << "    FAIL erf/erfc(NaN) not NaN\n"; ++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// tgamma: factorial fixed points, the reflection-free recurrence
	// tgamma(x+1)==x*tgamma(x), and tgamma(1/2)^2 == pi.
	template<typename Real>
	int VerifyTgamma(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// tgamma(n) == (n-1)!
		double fact = 1.0;  // (n-1)! ; starts at 0!=1 for n=1
		for (int n = 1; n <= 7; ++n) {
			if (n > 1) fact *= (n - 1);
			if (!close_rel(tgamma(Real(double(n))), Real(fact), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL tgamma(" << n << ") != " << fact << "\n"; ++nrOfFailedTestCases;
			}
		}
		// tgamma(1/2)^2 == pi
		Real g = tgamma(Real(0.5));
		if (!close_rel(g * g, Real(std::acos(-1.0)), 1.0e-13)) {
			if (reportTestCases) std::cout << "    FAIL tgamma(1/2)^2 != pi\n"; ++nrOfFailedTestCases;
		}
		// recurrence: tgamma(x+1) == x*tgamma(x)
		for (double v : {1.5, 2.7, 4.2}) {
			Real x(v);
			if (!close_rel(tgamma(x + Real(1.0)), x * tgamma(x), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL gamma recurrence at " << v << "\n"; ++nrOfFailedTestCases;
			}
		}
		// special values: tgamma(+Inf) = +Inf, tgamma(NaN) = NaN
		{
			double pinf = std::numeric_limits<double>::infinity();
			double rinf = double(tgamma(Real(pinf)));
			if (!std::isinf(rinf) || rinf < 0) {
				if (reportTestCases) std::cout << "    FAIL tgamma(+Inf) != +Inf\n"; ++nrOfFailedTestCases;
			}
			if (!std::isnan(double(tgamma(Real(std::numeric_limits<double>::quiet_NaN()))))) {
				if (reportTestCases) std::cout << "    FAIL tgamma(NaN) != NaN\n"; ++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// lgamma: fixed points and agreement with log(tgamma) / std::lgamma.
	template<typename Real>
	int VerifyLgamma(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// lgamma(1) == 0 and lgamma(2) == 0
		if (!lgamma(Real(1.0)).iszero() || !lgamma(Real(2.0)).iszero()) {
			if (reportTestCases) std::cout << "    FAIL lgamma(1) or lgamma(2) != 0\n"; ++nrOfFailedTestCases;
		}
		// lgamma(x) == log(tgamma(x)) for x where tgamma is moderate
		for (double v : {1.5, 3.0, 5.0}) {
			Real x(v);
			if (!close_rel(lgamma(x), log(tgamma(x)), 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL lgamma != log(tgamma) at " << v << "\n"; ++nrOfFailedTestCases;
			}
			// agreement with the std::lgamma reference
			if (!close_rel(lgamma(x), Real(std::lgamma(v)), 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL lgamma(" << v << ") vs std\n"; ++nrOfFailedTestCases;
			}
		}
		// special value: lgamma(+Inf) = +Inf
		double linf = double(lgamma(Real(std::numeric_limits<double>::infinity())));
		if (!std::isinf(linf) || linf < 0) {
			if (reportTestCases) std::cout << "    FAIL lgamma(+Inf) != +Inf\n"; ++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// Property fuzzer: erf parity / complement / monotonicity, and the gamma
	// recurrence, over random inputs.
	template<typename Real>
	int VerifyErrorGammaFuzz(bool reportTestCases, unsigned nrIterations) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(0xC1A55'1FFEULL);
		std::uniform_real_distribution<double> edist(-3.0, 3.0);
		std::uniform_real_distribution<double> gdist(0.5, 6.0);
		for (unsigned i = 0; i < nrIterations; ++i) {
			double e = edist(rng);
			Real x(e);
			if (!close_rel(erf(-x), -erf(x), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL erf parity at " << e << "\n"; ++nrOfFailedTestCases;
			}
			if (!close_rel(erf(x) + erfc(x), Real(1.0), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL erf+erfc != 1 at " << e << "\n"; ++nrOfFailedTestCases;
			}
			if (!(erf(x) < erf(x + Real(0.5)))) {   // erf strictly increasing
				if (reportTestCases) std::cout << "    FAIL erf monotonic at " << e << "\n"; ++nrOfFailedTestCases;
			}
			double g = gdist(rng);
			Real y(g);
			if (!close_rel(tgamma(y + Real(1.0)), y * tgamma(y), 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL gamma recurrence at " << g << "\n"; ++nrOfFailedTestCases;
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
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
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
	nrOfFailedTestCases += ReportTestResult(VerifyErrorGammaFuzz<ereal<>>(reportTestCases, 1000), "error/gamma property fuzz", test_tag);
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
