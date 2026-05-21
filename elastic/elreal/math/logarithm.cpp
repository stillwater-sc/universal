// logarithm.cpp: tests for elreal log / log2 / log10 / log1p (Phase E.3)
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

	std::string test_suite = "elreal Phase E.3 logarithm";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- log identities -------------------------------------------------
	{
		if (double(log(elreal(1.0))) != 0.0) {
			std::cerr << "FAIL: log(1) != 0\n"; ++nrOfFailedTestCases;
		}
		nrOfFailedTestCases += check_close("log(e) ~= 1",
			double(log(elreal(std::numbers::e_v<double>))), 1.0, 1e-15);
	}

	// --- log cross-validation against std::log -------------------------
	{
		for (double v : {0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 100.0, 1e10, 1e-10}) {
			double got = double(log(elreal(v)));
			double exp_v = std::log(v);
			if (got != exp_v) {
				std::cerr << "FAIL: log(" << v << ") = " << got
					<< " (std::log = " << exp_v << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Edge cases: log(0), log(negative), log(+inf), log(NaN) --------
	{
		if (!log(elreal(0.0)).isinf()) {
			std::cerr << "FAIL: log(0) not infinite (expected -inf)\n";
			++nrOfFailedTestCases;
		}
		if (!log(elreal(-1.0)).isnan()) {
			std::cerr << "FAIL: log(-1) != NaN\n"; ++nrOfFailedTestCases;
		}
		if (!log(elreal(SpecificValue::infpos)).isinf()) {
			std::cerr << "FAIL: log(+inf) != +inf\n"; ++nrOfFailedTestCases;
		}
		if (!log(elreal(SpecificValue::qnan)).isnan()) {
			std::cerr << "FAIL: log(NaN) != NaN\n"; ++nrOfFailedTestCases;
		}
	}

	// --- log2 identities ------------------------------------------------
	{
		nrOfFailedTestCases += check_close("log2(1) == 0",   double(log2(elreal(1.0))), 0.0);
		nrOfFailedTestCases += check_close("log2(2) == 1",   double(log2(elreal(2.0))), 1.0);
		nrOfFailedTestCases += check_close("log2(1024)==10", double(log2(elreal(1024.0))), 10.0);
		nrOfFailedTestCases += check_close("log2(0.5)==-1",  double(log2(elreal(0.5))), -1.0);
	}

	// --- log10 identities ----------------------------------------------
	{
		nrOfFailedTestCases += check_close("log10(1)==0",    double(log10(elreal(1.0))), 0.0);
		nrOfFailedTestCases += check_close("log10(10)==1",   double(log10(elreal(10.0))), 1.0);
		nrOfFailedTestCases += check_close("log10(100)==2",  double(log10(elreal(100.0))), 2.0);
		nrOfFailedTestCases += check_close("log10(0.01)==-2",double(log10(elreal(0.01))), -2.0);
	}

	// --- log1p identities -----------------------------------------------
	{
		nrOfFailedTestCases += check_close("log1p(0) == 0", double(log1p(elreal(0.0))), 0.0);
		// log1p(x) is much more accurate than log(1+x) for small x.
		// Cross-validate against std::log1p.
		for (double v : {1.0e-15, 1.0e-10, 0.01, 1.0, 5.0}) {
			double got = double(log1p(elreal(v)));
			double exp_v = std::log1p(v);
			if (got != exp_v) {
				std::cerr << "FAIL: log1p(" << v << ") = " << got
					<< " (std::log1p = " << exp_v << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Round-trip: log(exp(x)) ~= x ---------------------------------
	{
		for (double v : {-3.0, -1.0, 0.5, 1.0, 3.14, 10.0}) {
			elreal x(v);
			elreal back = log(exp(x));
			nrOfFailedTestCases += check_close(
				(std::string("log(exp(") + std::to_string(v) + ")) ~= x").c_str(),
				double(back), v, 1e-13);
		}
	}

	// --- Round-trip: exp(log(x)) ~= x ---------------------------------
	{
		for (double v : {0.1, 0.5, 1.0, 2.0, 10.0, 1e10}) {
			elreal x(v);
			elreal back = exp(log(x));
			nrOfFailedTestCases += check_close(
				(std::string("exp(log(") + std::to_string(v) + ")) ~= x").c_str(),
				double(back), v, 1e-13);
		}
	}

	// --- Depth-1 propagation: log on a rational has a non-zero at(1) --
	{
		elreal third(1LL, 3LL);
		elreal l = log(third);
		if (l.at(1) == 0.0) {
			std::cerr << "FAIL: log(1/3) has zero depth-1 correction\n";
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
