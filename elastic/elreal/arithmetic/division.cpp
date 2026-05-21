// division.cpp: tests for elreal binary / and /= (Phase C)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase C scope reminder: division ships only the depth-0 leading double
// result. Newton-Raphson refinement on the reciprocal stream is Phase E/F.
// Tests here exercise correctness at double precision plus the special
// cases (divide-by-zero, signs, identities).

#include <universal/utility/directives.hpp>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite = "elreal Phase C division";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Basic correctness ----------------------------------------------
	{
		elreal a(6.0), b(2.0), c = a / b;
		nrOfFailedTestCases += check_close("6 / 2", double(c), 3.0);
	}
	{
		elreal a(1.0), b(4.0), c = a / b;
		nrOfFailedTestCases += check_close("1 / 4", double(c), 0.25);
	}
	{
		elreal a(-6.0), b(2.0), c = a / b;
		nrOfFailedTestCases += check_close("-6 / 2", double(c), -3.0);
	}

	// --- Identity: a / a == 1 --------------------------------------------
	{
		elreal a(3.14);
		nrOfFailedTestCases += check_close("a / a == 1", double(a / a), 1.0);
	}
	{
		elreal a(1.0/3.0);
		// Even for "inexact" leading components, the leading-double divide
		// at runtime is exact for a/a since a/a always rounds to 1.0.
		nrOfFailedTestCases += check_close("(1/3) / (1/3) == 1", double(a / a), 1.0);
	}

	// --- 1 / a is the reciprocal ----------------------------------------
	{
		elreal a(4.0);
		elreal recip = elreal(1.0) / a;
		nrOfFailedTestCases += check_close("1 / 4 == 0.25", double(recip), 0.25);
	}

	// --- Double reciprocal: 1 / (1 / a) ~= a (at double precision) ------
	{
		elreal a(3.14);
		elreal recip = elreal(1.0) / a;
		elreal back  = elreal(1.0) / recip;
		nrOfFailedTestCases += check_close("1 / (1/a) ~= a", double(back), 3.14);
	}

	// --- Compound assignment --------------------------------------------
	{
		elreal x(6.0), y(2.0), z = x / y;
		x /= y;
		if (double(x) != double(z)) {
			std::cerr << "FAIL: a /= b not equivalent to a = a / b\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Division by zero, default mode (no exception) -------------------
	// ELREAL_THROW_ARITHMETIC_EXCEPTION is 0 by default; div-by-zero
	// should propagate IEEE-754 +/-inf or NaN through the leading double.
	{
		elreal a(1.0), zero;
		elreal r = a / zero;
		if (!r.isinf()) {
			std::cerr << "FAIL: 1 / 0 with throw disabled did not produce inf\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal zero1, zero2;
		elreal r = zero1 / zero2;
		if (!r.isnan()) {
			std::cerr << "FAIL: 0 / 0 with throw disabled did not produce NaN\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal neg(-1.0), zero;
		elreal r = neg / zero;
		if (!r.isinf() || double(r) > 0.0) {
			std::cerr << "FAIL: -1 / 0 with throw disabled did not produce -inf\n";
			++nrOfFailedTestCases;
		}
	}

	// --- NaN propagation ------------------------------------------------
	{
		elreal nan_v(SpecificValue::qnan), a(2.0);
		elreal r = nan_v / a;
		if (!r.isnan()) {
			std::cerr << "FAIL: NaN / 2 != NaN\n";
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
