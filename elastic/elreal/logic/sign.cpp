// sign.cpp: tests for elreal sign() refinement walk (Phase D)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase D sign";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Simple positive / negative / zero ------------------------------
	{
		elreal a(0.0);
		if (sign(a) != 0) {
			std::cerr << "FAIL: sign(0.0) != 0\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal a(3.14);
		if (sign(a) != +1) {
			std::cerr << "FAIL: sign(3.14) != +1\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal a(-2.5);
		if (sign(a) != -1) {
			std::cerr << "FAIL: sign(-2.5) != -1\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Inf and NaN -----------------------------------------------------
	{
		elreal pinf(SpecificValue::infpos);
		if (sign(pinf) != +1) { std::cerr << "FAIL: sign(+inf) != +1\n"; ++nrOfFailedTestCases; }
		elreal ninf(SpecificValue::infneg);
		if (sign(ninf) != -1) { std::cerr << "FAIL: sign(-inf) != -1\n"; ++nrOfFailedTestCases; }
		elreal nan(SpecificValue::qnan);
		if (sign(nan) != 0)   { std::cerr << "FAIL: sign(NaN) != 0 (unordered)\n"; ++nrOfFailedTestCases; }
	}

	// --- Refinement: sign of a value whose leading component is zero ----
	{
		// Construct a value whose at(0) is 0 but at(1) is non-zero by
		// subtracting two elreals whose leading doubles are equal.
		// double(1.0/3.0) == double(1.0/3.0) trivially -- but the rational
		// elreal carries an extra correction. So (rational - rounded) has
		// at(0) == 0 and at(1) != 0.
		elreal rat(1LL, 3LL);
		elreal rnd(1.0/3.0);
		elreal diff = rat - rnd;
		// The leading double of diff is 0; the depth-1 correction must be
		// the rational residual (positive, since 1/3 > double(1/3)).
		if (diff.at(0) != 0.0) {
			std::cerr << "FAIL: (1/3 rational) - (1/3 double) at(0) != 0\n";
			++nrOfFailedTestCases;
		}
		if (sign(diff) != +1) {
			std::cerr << "FAIL: sign((1/3 rational) - (1/3 double)) != +1 "
				<< "(refinement past depth 0 not walking the stream)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Budget exhaustion behavior -------------------------------------
	{
		// a - a should be all-zero at any depth; sign returns 0 regardless
		// of budget.
		elreal a(3.14);
		elreal r = a - a;
		for (std::size_t budget : {1U, 4U, 16U, 100U}) {
			if (sign(r, budget) != 0) {
				std::cerr << "FAIL: sign(a - a) != 0 at budget=" << budget << "\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Signed zero: sign(-0.0) is 0, not -1 ---------------------------
	{
		elreal nz(-0.0);
		// elreal stores -0.0 in _components[0]; sign() does `at(0) < 0.0`
		// which is FALSE for -0.0 (the IEEE-754 comparison treats -0 == +0).
		// So sign(-0.0) returns 0 -- matches std::signbit-less convention.
		if (sign(nz) != 0) {
			std::cerr << "FAIL: sign(-0.0) != 0 (got " << sign(nz) << ")\n";
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
