// hypot.cpp: tests for elreal hypot (Phase E.2)
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

	std::string test_suite = "elreal Phase E.2 hypot";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Pythagorean triples (exact for these inputs) -------------------
	{
		nrOfFailedTestCases += check_close("hypot(3, 4) == 5",
			double(hypot(elreal(3.0), elreal(4.0))), 5.0);
		nrOfFailedTestCases += check_close("hypot(5, 12) == 13",
			double(hypot(elreal(5.0), elreal(12.0))), 13.0);
		nrOfFailedTestCases += check_close("hypot(8, 15) == 17",
			double(hypot(elreal(8.0), elreal(15.0))), 17.0);
	}

	// --- Zero arguments ------------------------------------------------
	{
		if (double(hypot(elreal(0.0), elreal(0.0))) != 0.0) {
			std::cerr << "FAIL: hypot(0, 0) != 0\n";
			++nrOfFailedTestCases;
		}
		if (double(hypot(elreal(0.0), elreal(5.0))) != 5.0) {
			std::cerr << "FAIL: hypot(0, 5) != 5\n";
			++nrOfFailedTestCases;
		}
		if (double(hypot(elreal(7.0), elreal(0.0))) != 7.0) {
			std::cerr << "FAIL: hypot(7, 0) != 7\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Symmetry: hypot(a, b) == hypot(b, a) ---------------------------
	{
		for (auto [a, b] : { std::pair{2.0, 7.0}, {1.5, 0.5}, {1e10, 1.0} }) {
			double ab = double(hypot(elreal(a), elreal(b)));
			double ba = double(hypot(elreal(b), elreal(a)));
			if (ab != ba) {
				std::cerr << "FAIL: hypot(" << a << ", " << b << ") != "
					<< "hypot(" << b << ", " << a << ") (" << ab << " vs " << ba << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Overflow safety: x*x would overflow but std::hypot handles it -
	{
		// 1e200^2 = 1e400 which overflows double.  std::hypot(1e200, 1e200)
		// = sqrt(2) * 1e200, well within double's range.
		double huge = 1e200;
		double result = double(hypot(elreal(huge), elreal(huge)));
		double expected = huge * std::sqrt(2.0);
		nrOfFailedTestCases += check_close("hypot(1e200, 1e200) no overflow",
			result, expected, 1e-13);
	}

	// --- Negative arguments treated like their absolute values ---------
	{
		double r = double(hypot(elreal(-3.0), elreal(-4.0)));
		nrOfFailedTestCases += check_close("hypot(-3, -4) == 5", r, 5.0);
	}

	// --- Cross-validation against std::hypot --------------------------
	{
		for (auto [a, b] : { std::pair{1.5, 2.5}, {0.1, 0.2}, {1e-10, 2e-10},
		                     {3.14, 2.71}, {1.0, 1e-15} }) {
			double got = double(hypot(elreal(a), elreal(b)));
			double exp = std::hypot(a, b);
			if (got != exp) {
				std::cerr << "FAIL: hypot(" << a << ", " << b
					<< ") = " << got << " (std::hypot = " << exp << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Special values ------------------------------------------------
	{
		elreal inf(SpecificValue::infpos), x(1.0);
		if (!hypot(inf, x).isinf()) {
			std::cerr << "FAIL: hypot(+inf, 1) != +inf\n";
			++nrOfFailedTestCases;
		}
		// std::hypot convention: hypot(NaN, finite) = NaN, but hypot(NaN, inf) = inf.
		elreal nan(SpecificValue::qnan);
		if (!hypot(nan, x).isnan()) {
			std::cerr << "FAIL: hypot(NaN, 1) != NaN\n";
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
