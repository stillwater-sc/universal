// pow.cpp: tests for elreal pow (Phase E.3)
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

	std::string test_suite = "elreal Phase E.3 pow";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Exact integer powers ------------------------------------------
	{
		if (double(pow(elreal(2.0), elreal(10.0))) != 1024.0) {
			std::cerr << "FAIL: pow(2, 10) != 1024\n"; ++nrOfFailedTestCases;
		}
		if (double(pow(elreal(3.0), elreal(4.0))) != 81.0) {
			std::cerr << "FAIL: pow(3, 4) != 81\n"; ++nrOfFailedTestCases;
		}
		if (double(pow(elreal(5.0), elreal(0.0))) != 1.0) {
			std::cerr << "FAIL: pow(5, 0) != 1\n"; ++nrOfFailedTestCases;
		}
		if (double(pow(elreal(2.0), elreal(-3.0))) != 0.125) {
			std::cerr << "FAIL: pow(2, -3) != 0.125\n"; ++nrOfFailedTestCases;
		}
	}

	// --- Identity cases ------------------------------------------------
	{
		// pow(a, 1) == a
		nrOfFailedTestCases += check_close("pow(7, 1) == 7",
			double(pow(elreal(7.0), elreal(1.0))), 7.0);
		// pow(1, b) == 1 for any b
		nrOfFailedTestCases += check_close("pow(1, 3.14) == 1",
			double(pow(elreal(1.0), elreal(3.14))), 1.0);
		// pow(0, b) == 0 for b > 0
		nrOfFailedTestCases += check_close("pow(0, 3) == 0",
			double(pow(elreal(0.0), elreal(3.0))), 0.0);
		// pow(0, 0) == 1 per C convention
		nrOfFailedTestCases += check_close("pow(0, 0) == 1 (C convention)",
			double(pow(elreal(0.0), elreal(0.0))), 1.0);
	}

	// --- Cross-validation against std::pow at depth 0 ----------------
	{
		using P = std::pair<double, double>;
		for (P p : {P{2.0, 0.5}, P{3.0, 1.5}, P{0.5, 4.0}, P{10.0, -2.0},
		            P{1.5, 2.5}, P{0.25, 0.5}}) {
			double got = double(pow(elreal(p.first), elreal(p.second)));
			double exp_v = std::pow(p.first, p.second);
			if (got != exp_v) {
				std::cerr << "FAIL: pow(" << p.first << ", " << p.second << ") = " << got
					<< " (std::pow = " << exp_v << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- pow consistency with exp(b * log(a)) for a > 0 --------------
	{
		for (auto p : { std::pair{3.14, 2.71}, std::pair{0.5, 7.0}, std::pair{10.0, 0.3} }) {
			elreal a(p.first), b(p.second);
			elreal direct = pow(a, b);
			elreal via_explog = exp(b * log(a));
			nrOfFailedTestCases += check_close(
				(std::string("pow(") + std::to_string(p.first) + "," + std::to_string(p.second)
					+ ") == exp(b*log(a))").c_str(),
				double(direct), double(via_explog), 1e-13);
		}
	}

	// --- Edge cases: negative base with fractional exponent gives NaN --
	{
		// std::pow(-1, 0.5) = NaN
		elreal r = pow(elreal(-1.0), elreal(0.5));
		if (!r.isnan()) {
			std::cerr << "FAIL: pow(-1, 0.5) != NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Inf/NaN propagation ------------------------------------------
	{
		elreal nan_v(SpecificValue::qnan), x(2.0);
		if (!pow(nan_v, x).isnan()) {
			std::cerr << "FAIL: pow(NaN, 2) != NaN\n"; ++nrOfFailedTestCases;
		}
		if (!pow(x, nan_v).isnan()) {
			std::cerr << "FAIL: pow(2, NaN) != NaN\n"; ++nrOfFailedTestCases;
		}
	}

	// --- Depth-1 propagation: pow on rational base has non-zero at(1) -
	{
		elreal third(1LL, 3LL);
		elreal two(2.0);
		elreal r = pow(third, two);     // (1/3)^2 = 1/9
		if (r.at(1) == 0.0) {
			std::cerr << "FAIL: pow(1/3, 2).at(1) == 0 "
				<< "(depth-1 generator not propagating operand a.at(1))\n";
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
