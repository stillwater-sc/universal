// arithmetic.cpp: arithmetic operator tests for unum Type I
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

	std::string test_suite  = "unum Type I arithmetic tests";
	std::string test_tag    = "unum arithmetic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Unum = unum<3, 4>;

	/////////////////////////////////////////////////////////////////////////////////////
	// addition
	std::cout << "*** addition\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b, c;

		a = 1.0; b = 2.0;
		c = a + b;
		if (c.to_double() != 3.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1+2=" << c << '\n'; }
		if (c.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1+2 should be exact\n"; }

		a = 0.5; b = 0.25;
		c = a + b;
		if (c.to_double() != 0.75) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0.5+0.25=" << c << '\n'; }

		// adding zero
		a = 3.5; b = 0.0;
		c = a + b;
		if (c.to_double() != 3.5) { ++nrOfFailedTestCases; std::cout << "  FAIL: 3.5+0=" << c << '\n'; }

		// negative addition
		a = 1.0; b = -1.0;
		c = a + b;
		if (c.to_double() != 0.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1+(-1)=" << c << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: addition\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// subtraction
	std::cout << "*** subtraction\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b, c;

		a = 4.0; b = 1.0;
		c = a - b;
		if (c.to_double() != 3.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 4-1=" << c << '\n'; }

		a = 1.0; b = 1.0;
		c = a - b;
		if (!c.iszero()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1-1 should be zero\n"; }

		a = 0.0; b = 2.5;
		c = a - b;
		if (c.to_double() != -2.5) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0-2.5=" << c << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: subtraction\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// multiplication
	std::cout << "*** multiplication\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b, c;

		a = 2.0; b = 4.0;
		c = a * b;
		if (c.to_double() != 8.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2*4=" << c << '\n'; }

		a = 1.5; b = 2.0;
		c = a * b;
		if (c.to_double() != 3.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.5*2=" << c << '\n'; }

		// multiply by zero
		a = 5.0; b = 0.0;
		c = a * b;
		if (!c.iszero()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 5*0 should be zero\n"; }

		// negative multiplication
		a = -2.0; b = 3.0;
		c = a * b;
		if (c.to_double() != -6.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: -2*3=" << c << '\n'; }

		// negative * negative
		a = -2.0; b = -4.0;
		c = a * b;
		if (c.to_double() != 8.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: -2*-4=" << c << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: multiplication\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// division
	std::cout << "*** division\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b, c;

		a = 8.0; b = 2.0;
		c = a / b;
		if (c.to_double() != 4.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 8/2=" << c << '\n'; }

		a = 1.0; b = 4.0;
		c = a / b;
		if (c.to_double() != 0.25) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1/4=" << c << '\n'; }

		// inexact division sets ubit
		a = 1.0; b = 3.0;
		c = a / b;
		if (!c.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1/3 ubit should be 1\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: division\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// divide by zero
	std::cout << "*** divide by zero\n";
	{
		Unum a, b;
		a = 1.0; b = 0.0;
		bool caught = false;
		try {
			Unum c = a / b;
			(void)c;
		}
		catch (const unum_divide_by_zero&) {
			caught = true;
		}
		if (!caught) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: divide by zero should throw\n";
		}
		else {
			std::cout << "  divide by zero correctly throws exception\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// NaN propagation
	std::cout << "*** NaN propagation\n";
	{
		int start = nrOfFailedTestCases;
		Unum nan, a, c;
		nan.setnan();
		a = 2.0;

		c = nan + a;
		if (!c.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN+2 should be NaN\n"; }

		c = a - nan;
		if (!c.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2-NaN should be NaN\n"; }

		c = nan * a;
		if (!c.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN*2 should be NaN\n"; }

		c = a / nan;
		if (!c.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2/NaN should be NaN\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: NaN propagation\n";
		else std::cout << "  NaN propagates correctly through arithmetic\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// compound assignment
	std::cout << "*** compound assignment\n";
	{
		int start = nrOfFailedTestCases;
		Unum a;

		a = 1.0; a += Unum(2.0);
		if (a.to_double() != 3.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1+=2 -> " << a << '\n'; }

		a = 10.0; a -= Unum(4.0);
		if (a.to_double() != 6.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 10-=4 -> " << a << '\n'; }

		a = 3.0; a *= Unum(2.0);
		if (a.to_double() != 6.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 3*=2 -> " << a << '\n'; }

		a = 8.0; a /= Unum(2.0);
		if (a.to_double() != 4.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: 8/=2 -> " << a << '\n'; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: compound assignment\n";
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
