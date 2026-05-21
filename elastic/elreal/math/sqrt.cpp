// sqrt.cpp: tests for elreal sqrt (Phase E.2)
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

	std::string test_suite = "elreal Phase E.2 sqrt";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Exact perfect squares ------------------------------------------
	{
		if (double(sqrt(elreal(0.0))) != 0.0)  { std::cerr << "FAIL: sqrt(0)\n";  ++nrOfFailedTestCases; }
		if (double(sqrt(elreal(1.0))) != 1.0)  { std::cerr << "FAIL: sqrt(1)\n";  ++nrOfFailedTestCases; }
		if (double(sqrt(elreal(4.0))) != 2.0)  { std::cerr << "FAIL: sqrt(4)\n";  ++nrOfFailedTestCases; }
		if (double(sqrt(elreal(9.0))) != 3.0)  { std::cerr << "FAIL: sqrt(9)\n";  ++nrOfFailedTestCases; }
		if (double(sqrt(elreal(16.0))) != 4.0) { std::cerr << "FAIL: sqrt(16)\n"; ++nrOfFailedTestCases; }
		if (double(sqrt(elreal(1.0e100))) != 1.0e50) { std::cerr << "FAIL: sqrt(1e100)\n"; ++nrOfFailedTestCases; }
	}

	// --- Non-square arguments cross-validated against std::sqrt ---------
	{
		for (double v : {2.0, 3.0, 5.0, 7.0, 10.0, 3.14, 1.0/3.0, 1.5e10}) {
			elreal a(v);
			double got = double(sqrt(a));
			double exp = std::sqrt(v);
			if (got != exp) {
				std::cerr << "FAIL: sqrt(" << v << ") = " << got
					<< " (std::sqrt = " << exp << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Round-trip identity: sqrt(a) * sqrt(a) ~= a --------------------
	{
		for (double v : {2.0, 7.0, 100.0, 0.5, 1.0e-10}) {
			elreal a(v);
			elreal r = sqrt(a);
			elreal back = r * r;
			nrOfFailedTestCases += check_close(
				(std::string("sqrt(") + std::to_string(v) + ")^2 ~= input").c_str(),
				double(back), v);
		}
	}

	// --- sqrt(2) matches the elreal_sqrt2 constant (cross-check vs E.1) -
	{
		elreal r = sqrt(elreal(2.0));
		double leading = r.at(0);
		double expected = std::numbers::sqrt2_v<double>;
		if (leading != expected) {
			std::cerr << "FAIL: sqrt(2).at(0) (" << leading
				<< ") != std::numbers::sqrt2 (" << expected << ")\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Depth-1 refinement: sqrt(2) on a single-double input does not
	// produce a useful depth-1 (since the input has only a depth-0 term and
	// the EFT residual on a square is small). But sqrt of an elreal that
	// already has a depth-1 correction must propagate it.
	{
		// Use a rational input: elreal(2,3) = 2/3, an inexact double.
		// sqrt(2/3) ~= 0.8164965... The depth-1 correction in the result
		// must reflect both the EFT residual of c0^2 and the operand's
		// depth-1 contribution.
		elreal two_thirds(2LL, 3LL);
		elreal r = sqrt(two_thirds);
		// at(1) need not be exact; only that it is non-zero (the generator
		// fired and propagated something useful).
		if (r.at(1) == 0.0) {
			std::cerr << "FAIL: sqrt(2/3).at(1) == 0 "
				<< "(depth-1 generator not propagating residual)\n";
			++nrOfFailedTestCases;
		}
		// Round-trip at depth 1 should be closer to 2/3 than at depth 0.
		// elreal arithmetic at depth 1 routes through the operator* generator
		// which uses two_prod -- so sqrt(2/3)^2 should recover 2/3 within
		// double precision easily.
		elreal back = r * r;
		nrOfFailedTestCases += check_close("sqrt(2/3)^2 ~= 2/3",
			double(back), 2.0/3.0);
	}

	// --- Negative argument in default mode returns NaN (no throw) -------
	{
		elreal neg(-4.0);
		elreal r = sqrt(neg);
		if (!r.isnan()) {
			std::cerr << "FAIL: sqrt(-4) with throw disabled did not produce NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Special values --------------------------------------------------
	{
		elreal inf(SpecificValue::infpos);
		if (!sqrt(inf).isinf()) {
			std::cerr << "FAIL: sqrt(+inf) != +inf\n";
			++nrOfFailedTestCases;
		}
		elreal nan(SpecificValue::qnan);
		if (!sqrt(nan).isnan()) {
			std::cerr << "FAIL: sqrt(NaN) != NaN\n";
			++nrOfFailedTestCases;
		}
		elreal zero;
		if (!sqrt(zero).iszero()) {
			std::cerr << "FAIL: sqrt(0) != 0\n";
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
