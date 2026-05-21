// addition.cpp: tests for elreal binary + and += (Phase C)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
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

	std::string test_suite = "elreal Phase C addition";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Basic correctness ----------------------------------------------
	{
		elreal a(1.5), b(2.25), c = a + b;
		nrOfFailedTestCases += check_close("1.5 + 2.25", double(c), 3.75);
	}
	{
		elreal a(0.0), b(7.0), c = a + b;
		nrOfFailedTestCases += check_close("0 + 7", double(c), 7.0);
	}
	{
		elreal a(-3.0), b(3.0), c = a + b;
		nrOfFailedTestCases += check_close("-3 + 3", double(c), 0.0);
	}

	// --- Commutativity (a + b) == (b + a) on leading double -------------
	{
		elreal a(1.0/7.0), b(22.0/7.0);
		elreal ab = a + b;
		elreal ba = b + a;
		if (double(ab) != double(ba)) {
			std::cerr << "FAIL: commutativity (leading): " << double(ab)
				<< " != " << double(ba) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Algebraic identity: (a + b) - b ~= a ---------------------------
	{
		elreal a(1.0/3.0), b(22.0/7.0);
		elreal back = (a + b) - b;
		nrOfFailedTestCases += check_close("(a+b)-b ~= a", double(back), double(a));
	}

	// --- Compound assignment matches binary -----------------------------
	{
		elreal x(1.5), y(0.25), z = x + y;
		x += y;
		if (double(x) != double(z)) {
			std::cerr << "FAIL: a += b not equivalent to a = a + b\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Depth-1 refinement: 1/3 + 1/3 should have a non-zero correction
	// (each operand has its own correction; the sum's correction is the
	// sum of theirs plus the EFT residual on the leading doubles).
	{
		elreal third(1LL, 3LL);
		elreal sum = third + third;
		// at(1) should not equal 0: the rational operand has at(1) != 0,
		// and the operator+ generator propagates that to the result.
		if (sum.at(1) == 0.0) {
			std::cerr << "FAIL: (1/3) + (1/3) has zero depth-1 correction "
				<< "(generator not propagating operand refinement)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- NaN / inf propagation ------------------------------------------
	{
		elreal nan_v(SpecificValue::qnan), a(1.0);
		elreal r = nan_v + a;
		if (!r.isnan()) {
			std::cerr << "FAIL: NaN + 1 != NaN\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal pinf(SpecificValue::infpos), a(1.0);
		elreal r = pinf + a;
		if (!r.isinf()) {
			std::cerr << "FAIL: +inf + 1 != +inf\n";
			++nrOfFailedTestCases;
		}
	}
	{
		// +inf + -inf = NaN
		elreal pinf(SpecificValue::infpos), ninf(SpecificValue::infneg);
		elreal r = pinf + ninf;
		if (!r.isnan()) {
			std::cerr << "FAIL: +inf + -inf != NaN\n";
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
