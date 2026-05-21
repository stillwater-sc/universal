// subtraction.cpp: tests for elreal binary - and -=, plus unary - and abs (Phase C)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

#include <algorithm>   // std::max
#include <cmath>       // std::abs

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

	std::string test_suite = "elreal Phase C subtraction";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Basic correctness ----------------------------------------------
	{
		elreal a(5.0), b(3.0), c = a - b;
		nrOfFailedTestCases += check_close("5 - 3", double(c), 2.0);
	}
	{
		elreal a(0.0), b(7.0), c = a - b;
		nrOfFailedTestCases += check_close("0 - 7", double(c), -7.0);
	}

	// --- Identity: a - a == 0 -------------------------------------------
	{
		elreal a(1.0/3.0);
		elreal r = a - a;
		nrOfFailedTestCases += check_close("a - a == 0", double(r), 0.0);
	}

	// --- (a - b) + b ~= a -----------------------------------------------
	{
		elreal a(22.0/7.0), b(1.0/3.0);
		elreal back = (a - b) + b;
		nrOfFailedTestCases += check_close("(a-b)+b ~= a", double(back), double(a));
	}

	// --- Compound assignment --------------------------------------------
	{
		elreal x(5.0), y(2.0), z = x - y;
		x -= y;
		if (double(x) != double(z)) {
			std::cerr << "FAIL: a -= b not equivalent to a = a - b\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Unary minus: negate every component, including the correction --
	{
		elreal third(1LL, 3LL);
		elreal neg_third = -third;
		// Leading component
		if (neg_third.at(0) != -third.at(0)) {
			std::cerr << "FAIL: unary -(1/3) did not negate leading\n";
			++nrOfFailedTestCases;
		}
		// Correction component (from the generator)
		if (neg_third.at(1) != -third.at(1)) {
			std::cerr << "FAIL: unary -(1/3) did not negate depth-1 correction\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Double negation is identity -------------------------------------
	{
		elreal a(1.0/3.0);
		elreal back = -(-a);
		if (back.at(0) != a.at(0) || back.at(1) != a.at(1)) {
			std::cerr << "FAIL: -(-a) != a at depth 0 or 1\n";
			++nrOfFailedTestCases;
		}
	}

	// --- abs(): negative becomes positive, positive unchanged ----------
	{
		elreal a(-3.14);
		elreal b = abs(a);
		nrOfFailedTestCases += check_close("abs(-3.14) == 3.14", double(b), 3.14);
	}
	{
		elreal a(7.0);
		elreal b = abs(a);
		nrOfFailedTestCases += check_close("abs(7) == 7", double(b), 7.0);
	}
	// fabs alias
	{
		elreal a(-2.5);
		elreal b = fabs(a);
		nrOfFailedTestCases += check_close("fabs(-2.5) == 2.5", double(b), 2.5);
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
