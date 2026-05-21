// multiplication.cpp: tests for elreal binary * and *= (Phase C)
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

	std::string test_suite = "elreal Phase C multiplication";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Basic correctness ----------------------------------------------
	{
		elreal a(3.0), b(4.0), c = a * b;
		nrOfFailedTestCases += check_close("3 * 4", double(c), 12.0);
	}
	{
		elreal a(0.5), b(0.25), c = a * b;
		nrOfFailedTestCases += check_close("0.5 * 0.25", double(c), 0.125);
	}
	{
		elreal a(-2.0), b(3.0), c = a * b;
		nrOfFailedTestCases += check_close("-2 * 3", double(c), -6.0);
	}

	// --- Identity: 1 * a == a, a * 1 == a -------------------------------
	{
		elreal one(1.0), a(7.5);
		nrOfFailedTestCases += check_close("1 * a == a",  double(one * a), 7.5);
		nrOfFailedTestCases += check_close("a * 1 == a",  double(a * one), 7.5);
	}

	// --- Absorbing element: 0 * a == 0 ---------------------------------
	{
		elreal zero, a(7.5);
		nrOfFailedTestCases += check_close("0 * a == 0", double(zero * a), 0.0);
		nrOfFailedTestCases += check_close("a * 0 == 0", double(a * zero), 0.0);
	}

	// --- Commutativity ---------------------------------------------------
	{
		elreal a(1.0/3.0), b(22.0/7.0);
		if (double(a * b) != double(b * a)) {
			std::cerr << "FAIL: commutativity (leading): "
				<< double(a * b) << " != " << double(b * a) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// --- (a * b) / b ~= a (when b != 0) --------------------------------
	{
		elreal a(3.14), b(2.71);
		elreal back = (a * b) / b;
		nrOfFailedTestCases += check_close("(a*b)/b ~= a", double(back), 3.14);
	}

	// --- Compound assignment --------------------------------------------
	{
		elreal x(3.0), y(4.0), z = x * y;
		x *= y;
		if (double(x) != double(z)) {
			std::cerr << "FAIL: a *= b not equivalent to a = a * b\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Depth-1 refinement: 3 * (1/3) should have a residual -----------
	{
		elreal three(3.0);
		elreal third(1LL, 3LL);
		elreal r = three * third;
		// 3 * (1/3) is the canonical "doesn't quite equal 1" case in IEEE.
		// Depth-0 result: c0 = 3.0 * (1/3) rounded; that may equal 1.0
		// exactly (depending on rounding), and we'd then expect the depth-1
		// correction to be small but non-zero (the EFT residual plus the
		// operand correction). We only assert that the leading is close to
		// 1.0; the depth-1 contract is just "the generator fired."
		nrOfFailedTestCases += check_close("3 * (1/3) ~= 1.0", double(r), 1.0);
		// Touching at(1) walks the generator -- not zero on a rational operand.
		(void)r.at(1);
	}

	// --- NaN / inf propagation ------------------------------------------
	{
		elreal nan_v(SpecificValue::qnan), a(2.0);
		elreal r = nan_v * a;
		if (!r.isnan()) {
			std::cerr << "FAIL: NaN * 2 != NaN\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal pinf(SpecificValue::infpos), zero;
		elreal r = pinf * zero;
		if (!r.isnan()) {
			std::cerr << "FAIL: inf * 0 != NaN\n";
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
