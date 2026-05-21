// exponent.cpp: tests for elreal exp / exp2 / expm1 (Phase E.3)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

static int check_close(const char* label, double got, double expected, double tol = 1e-14) {
	double diff = std::abs(got - expected);
	double mag  = std::max(std::abs(expected), 1.0);
	if (diff / mag > tol) {
		std::cerr << "FAIL: " << label << ": got " << got
			<< " expected " << expected << " (rel err " << diff / mag << ")\n";
		return 1;
	}
	return 0;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase E.3 exponent";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- exp identities -------------------------------------------------
	{
		if (double(exp(elreal(0.0))) != 1.0) {
			std::cerr << "FAIL: exp(0) != 1\n"; ++nrOfFailedTestCases;
		}
		nrOfFailedTestCases += check_close("exp(1) ~= e",
			double(exp(elreal(1.0))), std::numbers::e_v<double>);
	}

	// --- exp cross-validation against std::exp -------------------------
	{
		for (double v : {-2.0, -1.0, -0.5, 0.1, 0.5, 1.5, 2.0, 5.0, 10.0}) {
			double got = double(exp(elreal(v)));
			double exp_v = std::exp(v);
			if (got != exp_v) {
				std::cerr << "FAIL: exp(" << v << ") = " << got
					<< " (std::exp = " << exp_v << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- exp(inf) = inf, exp(-inf) = 0, exp(NaN) = NaN -----------------
	{
		if (!exp(elreal(SpecificValue::infpos)).isinf()) {
			std::cerr << "FAIL: exp(+inf) != +inf\n"; ++nrOfFailedTestCases;
		}
		if (double(exp(elreal(SpecificValue::infneg))) != 0.0) {
			std::cerr << "FAIL: exp(-inf) != 0\n"; ++nrOfFailedTestCases;
		}
		if (!exp(elreal(SpecificValue::qnan)).isnan()) {
			std::cerr << "FAIL: exp(NaN) != NaN\n"; ++nrOfFailedTestCases;
		}
	}

	// --- exp2 identities ------------------------------------------------
	{
		nrOfFailedTestCases += check_close("exp2(0) == 1",  double(exp2(elreal(0.0))),  1.0);
		nrOfFailedTestCases += check_close("exp2(1) == 2",  double(exp2(elreal(1.0))),  2.0);
		nrOfFailedTestCases += check_close("exp2(10) == 1024", double(exp2(elreal(10.0))), 1024.0);
		nrOfFailedTestCases += check_close("exp2(-1) == 0.5", double(exp2(elreal(-1.0))), 0.5);
	}

	// --- expm1 identities ----------------------------------------------
	{
		if (double(expm1(elreal(0.0))) != 0.0) {
			std::cerr << "FAIL: expm1(0) != 0\n"; ++nrOfFailedTestCases;
		}
		// For small x, expm1(x) is much more accurate than exp(x) - 1.
		// Cross-validate against std::expm1.
		for (double v : {1.0e-15, 1.0e-10, 0.01, 1.0, 5.0}) {
			double got = double(expm1(elreal(v)));
			double exp_v = std::expm1(v);
			if (got != exp_v) {
				std::cerr << "FAIL: expm1(" << v << ") = " << got
					<< " (std::expm1 = " << exp_v << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Depth-1 propagation: exp on rational input has non-zero at(1) -
	// The derivative-based correction is c0 * a.at(1); rational at(1) is
	// non-zero, so the result's at(1) must be non-zero too.
	{
		elreal third(1LL, 3LL);     // 1/3, has rational residual at depth 1
		elreal e_third = exp(third);
		if (e_third.at(1) == 0.0) {
			std::cerr << "FAIL: exp(1/3) has zero depth-1 (generator not propagating)\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
