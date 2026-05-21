// hyperbolic.cpp: tests for elreal sinh/cosh/tanh + inverses (Phase E.4)
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

	std::string test_suite = "elreal Phase E.4 hyperbolic";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Anchor values at zero -----------------------------------------
	{
		if (double(sinh(elreal(0.0))) != 0.0) { std::cerr << "FAIL: sinh(0) != 0\n"; ++nrOfFailedTestCases; }
		if (double(cosh(elreal(0.0))) != 1.0) { std::cerr << "FAIL: cosh(0) != 1\n"; ++nrOfFailedTestCases; }
		if (double(tanh(elreal(0.0))) != 0.0) { std::cerr << "FAIL: tanh(0) != 0\n"; ++nrOfFailedTestCases; }

		if (double(asinh(elreal(0.0))) != 0.0) { std::cerr << "FAIL: asinh(0) != 0\n"; ++nrOfFailedTestCases; }
		if (double(acosh(elreal(1.0))) != 0.0) { std::cerr << "FAIL: acosh(1) != 0\n"; ++nrOfFailedTestCases; }
		if (double(atanh(elreal(0.0))) != 0.0) { std::cerr << "FAIL: atanh(0) != 0\n"; ++nrOfFailedTestCases; }
	}

	// --- Hyperbolic Pythagorean identity: cosh^2 - sinh^2 == 1 ---------
	{
		for (double v : {-2.0, -0.5, 0.5, 1.0, 3.0}) {
			elreal x(v);
			elreal s = sinh(x);
			elreal c = cosh(x);
			elreal one = c*c - s*s;
			nrOfFailedTestCases += check_close(
				(std::string("cosh^2 - sinh^2 (x=") + std::to_string(v) + ")").c_str(),
				double(one), 1.0, 1e-13);
		}
	}

	// --- Cross-validation against std lib at depth 0 ------------------
	{
		for (double v : {-3.0, -1.0, -0.5, 0.1, 0.5, 1.0, 2.0, 5.0}) {
			elreal x(v);
			if (double(sinh(x)) != std::sinh(v)) { std::cerr << "FAIL: sinh(" << v << ") vs std::sinh\n"; ++nrOfFailedTestCases; }
			if (double(cosh(x)) != std::cosh(v)) { std::cerr << "FAIL: cosh(" << v << ") vs std::cosh\n"; ++nrOfFailedTestCases; }
			if (double(tanh(x)) != std::tanh(v)) { std::cerr << "FAIL: tanh(" << v << ") vs std::tanh\n"; ++nrOfFailedTestCases; }
			if (double(asinh(x)) != std::asinh(v)) { std::cerr << "FAIL: asinh(" << v << ") vs std::asinh\n"; ++nrOfFailedTestCases; }
		}
		// acosh requires argument >= 1
		for (double v : {1.0, 1.5, 2.0, 10.0}) {
			if (double(acosh(elreal(v))) != std::acosh(v)) {
				std::cerr << "FAIL: acosh(" << v << ") vs std::acosh\n";
				++nrOfFailedTestCases;
			}
		}
		// atanh requires |x| < 1
		for (double v : {-0.9, -0.5, 0.0, 0.5, 0.9}) {
			if (double(atanh(elreal(v))) != std::atanh(v)) {
				std::cerr << "FAIL: atanh(" << v << ") vs std::atanh\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// --- Round-trip identities ---------------------------------------
	{
		for (double v : {-2.0, -0.5, 0.5, 2.0}) {
			elreal x(v);
			nrOfFailedTestCases += check_close("asinh(sinh(x)) ~= x",
				double(asinh(sinh(x))), v, 1e-13);
		}
		for (double v : {1.5, 2.0, 5.0}) {
			elreal x(v);
			nrOfFailedTestCases += check_close("acosh(cosh(x)) ~= |x|",
				double(acosh(cosh(x))), std::abs(v), 1e-13);
		}
		for (double v : {-0.5, 0.3, 0.7}) {
			elreal x(v);
			nrOfFailedTestCases += check_close("atanh(tanh(x)) ~= x",
				double(atanh(tanh(x))), v, 1e-13);
		}
	}

	// --- Edge cases: acosh(<1) and atanh(>=1) propagate NaN/inf ------
	{
		if (!acosh(elreal(0.5)).isnan()) {
			std::cerr << "FAIL: acosh(0.5) != NaN\n"; ++nrOfFailedTestCases;
		}
		if (!atanh(elreal(2.0)).isnan()) {
			std::cerr << "FAIL: atanh(2.0) != NaN\n"; ++nrOfFailedTestCases;
		}
		if (!atanh(elreal(1.0)).isinf()) {
			std::cerr << "FAIL: atanh(1.0) != inf (boundary case)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Special-value propagation -----------------------------------
	{
		elreal nan_v(SpecificValue::qnan);
		if (!sinh(nan_v).isnan() || !cosh(nan_v).isnan() || !tanh(nan_v).isnan()
		    || !asinh(nan_v).isnan() || !acosh(nan_v).isnan() || !atanh(nan_v).isnan()) {
			std::cerr << "FAIL: a NaN propagation case in the hyperbolic family\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Depth-1 propagation on rational input -----------------------
	// The derivative-based depth-1 generator must propagate a.at(1) from
	// the operand. Use a rational operand to guarantee at(1) != 0.
	{
		elreal third(1LL, 3LL);
		if (sinh(third).at(1) == 0.0)  { std::cerr << "FAIL: sinh(1/3).at(1) == 0\n";  ++nrOfFailedTestCases; }
		if (cosh(third).at(1) == 0.0)  { std::cerr << "FAIL: cosh(1/3).at(1) == 0\n";  ++nrOfFailedTestCases; }
		if (tanh(third).at(1) == 0.0)  { std::cerr << "FAIL: tanh(1/3).at(1) == 0\n";  ++nrOfFailedTestCases; }
		if (asinh(third).at(1) == 0.0) { std::cerr << "FAIL: asinh(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		if (atanh(third).at(1) == 0.0) { std::cerr << "FAIL: atanh(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		// acosh requires arg >= 1; use 1 + 1/3
		elreal four_thirds = elreal(4LL, 3LL);
		if (acosh(four_thirds).at(1) == 0.0) {
			std::cerr << "FAIL: acosh(4/3).at(1) == 0\n";
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
