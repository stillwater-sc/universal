// compensated_arithmetic.cpp: test suite runner for faithful's negation and compensated arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// faithful<Real> carries a value and the error of that value. Unary minus did not compile, and
// three of the four compensated operators lost an operand's error: += dropped the right-hand
// error, -= added it instead of subtracting it, and /= ignored the divisor's error (#1930).
//
// The reference is exact: a faithful<float> is a float head plus a float error a few binades
// below it, and their sum fits a double exactly. The compensated result must match the double
// reference to well below float precision; each of those defects was off by a whole operand
// error, a relative 2^-24.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>
#include <cstdint>
#include <random>

namespace {

using Faithful = sw::universal::faithful<float>;

// the exact value a faithful<float> represents
double exact(const Faithful& v) { return double(v.high()) + double(v.low()); }

// a random faithful<float>: a head in [1, 2) * 2^k with a random sign, and an error at most half
// an ulp of the head, so that head + error is the head's best float and the pair is canonical
Faithful randomFaithful(std::mt19937_64& rng) {
	std::uniform_real_distribution<float> mant(1.0f, 2.0f);
	std::uniform_int_distribution<int>    expo(-8, 8);
	std::uniform_real_distribution<float> frac(-0.5f, 0.5f);
	std::bernoulli_distribution           neg(0.5);
	float head  = std::ldexp(mant(rng), expo(rng));
	if (neg(rng)) head = -head;
	float ulp   = std::nextafter(std::fabs(head), INFINITY) - std::fabs(head);
	float error = frac(rng) * ulp;
	return Faithful(head, error);
}

// |got - want| <= tol * scale, reporting the first few misses
int check(const char* op, double got, double want, double scale, double tol, const Faithful& x, const Faithful& y, bool report, int& reported) {
	if (std::fabs(got - want) <= tol * scale) return 0;
	if (report && reported++ < 5) {
		std::cerr << "FAIL " << op << " x = " << x << " y = " << y
		          << " got " << got << " want " << want << " (rel " << std::fabs(got - want) / scale << ")\n";
	}
	return 1;
}

int VerifyNegation(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const Faithful x(1.5f, 1.0e-8f);
	const Faithful y = -x;   // did not compile
	if (y.high() != -1.5f || y.low() != -1.0e-8f) ++nrOfFailedTests;
	if (x.high() != 1.5f || x.low() != 1.0e-8f) ++nrOfFailedTests;   // the operand is unchanged
	const Faithful z = -(-x);
	if (z.high() != x.high() || z.low() != x.low()) ++nrOfFailedTests;
	// x - y and x + (-y) are the same computation
	std::mt19937_64 rng(1930);
	for (int i = 0; i < 10000; ++i) {
		Faithful a = randomFaithful(rng), b = randomFaithful(rng);
		Faithful d1 = a - b, d2 = a + (-b);
		if (d1.high() != d2.high() || d1.low() != d2.low()) {
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL a - b != a + (-b) for " << a << ' ' << b << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

int VerifyCompensatedArithmetic(bool reportTestCases) {
	// float carries 24 bits and its error term 24 more; 2^-44 leaves room for the error term's
	// own rounding and the dropped second-order terms, and is 2^20 below the defects' size
	const double tol = std::ldexp(1.0, -44);
	int nrOfFailedTests = 0, reported = 0;
	std::mt19937_64 rng(19300);
	for (int i = 0; i < 100000; ++i) {
		Faithful x = randomFaithful(rng), y = randomFaithful(rng);
		double X = exact(x), Y = exact(y);
		// sum and difference can cancel, so measure against the operands
		double addScale = std::fmax(std::fabs(X), std::fabs(Y));
		nrOfFailedTests += check("+", exact(x + y), X + Y, addScale, tol, x, y, reportTestCases, reported);
		nrOfFailedTests += check("-", exact(x - y), X - Y, addScale, tol, x, y, reportTestCases, reported);
		nrOfFailedTests += check("*", exact(x * y), X * Y, std::fabs(X * Y), tol, x, y, reportTestCases, reported);
		nrOfFailedTests += check("/", exact(x / y), X / Y, std::fabs(X / Y), tol, x, y, reportTestCases, reported);
		// the in-place forms are what the binary operators call; check them on their own too
		Faithful s = x; s += y;
		nrOfFailedTests += check("+=", exact(s), X + Y, addScale, tol, x, y, reportTestCases, reported);
		Faithful d = x; d -= y;
		nrOfFailedTests += check("-=", exact(d), X - Y, addScale, tol, x, y, reportTestCases, reported);
		Faithful p = x; p *= y;
		nrOfFailedTests += check("*=", exact(p), X * Y, std::fabs(X * Y), tol, x, y, reportTestCases, reported);
		Faithful q = x; q /= y;
		nrOfFailedTests += check("/=", exact(q), X / Y, std::fabs(X / Y), tol, x, y, reportTestCases, reported);
	}
	return nrOfFailedTests;
}

} // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "faithful negation and compensated arithmetic";
	std::string test_tag    = "faithful<float>";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	Faithful x(1.5f, 1.0e-8f), y(0.75f, -2.0e-9f);
	std::cout << (-x) << '\n';
	std::cout << (x + y) << ' ' << exact(x) + exact(y) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNegation(reportTestCases), test_tag, "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyCompensatedArithmetic(reportTestCases), test_tag, "compensated + - * /");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
