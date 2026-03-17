// math.cpp: math function tests for unum Type I
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define UNUM_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/unum/unum.hpp>
#include <universal/number/unum/manipulators.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "unum Type I math function tests";
	std::string test_tag    = "unum math";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Unum = unum<3, 4>;

	/////////////////////////////////////////////////////////////////////////////////////
	// sqrt
	std::cout << "*** sqrt\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, r;

		// sqrt(4) = 2 (exact)
		a = 4.0;
		r = sqrt(a);
		if (r.to_double() != 2.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: sqrt(4)=" << r << '\n'; }
		if (r.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: sqrt(4) should be exact\n"; }

		// sqrt(2) is inexact
		a = 2.0;
		r = sqrt(a);
		if (!r.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: sqrt(2) should be inexact\n"; }

		// sqrt(negative) = NaN
		a = -1.0;
		r = sqrt(a);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: sqrt(-1) should be NaN\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: sqrt\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// exp and log
	std::cout << "*** exp and log\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, r;

		// exp(0) = 1
		a = 0.0;
		r = exp(a);
		if (r.to_double() != 1.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: exp(0)=" << r << '\n'; }

		// log(1) = 0
		a = 1.0;
		r = log(a);
		if (r.to_double() != 0.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: log(1)=" << r << '\n'; }

		// log(negative) = NaN
		a = -1.0;
		r = log(a);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: log(-1) should be NaN\n"; }

		// log(0) = NaN (our convention, since unum has no -inf)
		a = 0.0;
		r = log(a);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: log(0) should be NaN\n"; }

		// exp(log(x)) round-trip (limited by unum precision)
		a = 2.0;
		r = exp(log(a));
		double err = std::abs(r.to_double() - 2.0);
		if (err > 0.01) { ++nrOfFailedTestCases; std::cout << "  FAIL: exp(log(2))=" << r << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: exp/log\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// trigonometric
	std::cout << "*** trigonometry\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, r;

		// sin(0) = 0
		a = 0.0;
		r = sin(a);
		if (r.to_double() != 0.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: sin(0)=" << r << '\n'; }

		// cos(0) = 1
		a = 0.0;
		r = cos(a);
		if (r.to_double() != 1.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: cos(0)=" << r << '\n'; }

		// sin^2 + cos^2 = 1 (limited by unum precision)
		a = 1.0;
		Unum s = sin(a);
		Unum c = cos(a);
		double sum = s.to_double() * s.to_double() + c.to_double() * c.to_double();
		if (std::abs(sum - 1.0) > 0.01) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: sin^2(1)+cos^2(1)=" << sum << '\n';
		}

		// atan(1) ~= pi/4 (limited by unum precision)
		a = 1.0;
		r = atan(a);
		double pi_over_4 = std::atan(1.0);
		if (std::abs(r.to_double() - pi_over_4) > 0.01) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: atan(1)=" << r << '\n';
		}

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: trigonometry\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// pow
	std::cout << "*** pow\n";
	{
		int start = nrOfFailedTestCases;
		Unum base, exponent, r;

		base = 2.0; exponent = 8.0;
		r = pow(base, exponent);
		if (r.to_double() != 256.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2^8=" << r << '\n'; }

		base = 3.0; exponent = 0.0;
		r = pow(base, exponent);
		if (r.to_double() != 1.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 3^0=" << r << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: pow\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// floor, ceil, trunc
	std::cout << "*** floor, ceil, trunc\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, r;

		a = 2.7;
		r = floor(a);
		if (r.to_double() != 2.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: floor(2.7)=" << r << '\n'; }

		r = ceil(a);
		if (r.to_double() != 3.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: ceil(2.7)=" << r << '\n'; }

		r = trunc(a);
		if (r.to_double() != 2.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: trunc(2.7)=" << r << '\n'; }

		a = -2.7;
		r = floor(a);
		if (r.to_double() != -3.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: floor(-2.7)=" << r << '\n'; }

		r = ceil(a);
		if (r.to_double() != -2.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: ceil(-2.7)=" << r << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: floor/ceil/trunc\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// abs, min, max
	std::cout << "*** abs, min, max\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b, r;

		a = -3.5;
		r = abs(a);
		if (r.to_double() != 3.5) { ++nrOfFailedTestCases; std::cout << "  FAIL: abs(-3.5)=" << r << '\n'; }

		a = 1.0; b = 2.0;
		r = min(a, b);
		if (r.to_double() != 1.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: min(1,2)=" << r << '\n'; }

		r = max(a, b);
		if (r.to_double() != 2.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: max(1,2)=" << r << '\n'; }

		// min/max with NaN propagate NaN
		Unum nan_val;
		nan_val.setnan();
		r = min(a, nan_val);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: min(1, NaN) should be NaN\n"; }
		r = min(nan_val, a);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: min(NaN, 1) should be NaN\n"; }
		r = max(a, nan_val);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: max(1, NaN) should be NaN\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: abs/min/max\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// overflow to NaN (unum has no infinity)
	std::cout << "*** overflow maps to NaN\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, r;

		a = 1000.0;
		r = exp(a);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: exp(1000) should be NaN (overflow)\n"; }

		a = 1000.0;
		r = sinh(a);
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: sinh(1000) should be NaN (overflow)\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: overflow\n";
		else std::cout << "  overflow correctly maps to NaN\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// NaN propagation through math functions
	std::cout << "*** NaN propagation\n";
	{
		int start = nrOfFailedTestCases;
		Unum nan;
		nan.setnan();

		if (!sqrt(nan).isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: sqrt(NaN)\n"; }
		if (!exp(nan).isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: exp(NaN)\n"; }
		if (!log(nan).isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: log(NaN)\n"; }
		if (!sin(nan).isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: sin(NaN)\n"; }
		if (!cos(nan).isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: cos(NaN)\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: NaN propagation\n";
		else std::cout << "  NaN propagates correctly through math functions\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum_arithmetic_exception& err) {
	std::cerr << "Uncaught unum arithmetic exception: " << err.what() << std::endl;
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
