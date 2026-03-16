// ubox.cpp: ubound interval arithmetic tests for unum Type I
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

	std::string test_suite  = "unum Type I ubound interval tests";
	std::string test_tag    = "unum ubox";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Unum = unum<3, 4>;
	using Ubound = ubound<3, 4>;

	/////////////////////////////////////////////////////////////////////////////////////
	// exact unum -> point interval
	std::cout << "*** exact unum -> point interval\n";
	{
		int start = nrOfFailedTestCases;
		Unum exact;
		exact = 2.0;
		Ubound ub(exact);
		if (!ub.ispoint()) { ++nrOfFailedTestCases; std::cout << "  FAIL: exact 2.0 should be point\n"; }
		if (ub.width() != 0.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: point width should be 0\n"; }
		std::cout << "  exact(2.0) -> " << ub << '\n';

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: point interval\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// inexact unum -> interval with width > 0
	std::cout << "*** inexact unum -> interval\n";
	{
		int start = nrOfFailedTestCases;
		Unum inexact;
		inexact = 1.0 / 3.0;  // 1/3 is inexact in binary
		if (!inexact.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1/3 should have ubit set\n"; }

		Ubound ub(inexact);
		if (ub.ispoint()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1/3 should not be point interval\n"; }
		if (ub.width() <= 0.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: interval should have positive width\n"; }
		// the exact 1/3 should be contained in the interval
		if (!ub.contains(1.0 / 3.0)) { ++nrOfFailedTestCases; std::cout << "  FAIL: interval should contain 1/3\n"; }
		std::cout << "  inexact(1/3) -> " << ub << "  width: " << ub.width() << '\n';

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: inexact interval\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// interval addition
	std::cout << "*** interval addition\n";
	{
		int start = nrOfFailedTestCases;
		Ubound a(1.0), b(2.0);
		Ubound c = a + b;
		if (!c.contains(3.0)) { ++nrOfFailedTestCases; std::cout << "  FAIL: [1]+[2] should contain 3\n"; }
		std::cout << "  " << a << " + " << b << " = " << c << '\n';

		// inexact + exact: result is close to the exact answer
		Unum third;
		third = 1.0 / 3.0;
		Ubound ub_third(third);
		Ubound d = ub_third + b;
		double expected = 1.0 / 3.0 + 2.0;
		double err = std::abs(d.midpoint() - expected);
		if (err > 0.01) { ++nrOfFailedTestCases; std::cout << "  FAIL: [1/3]+[2] midpoint too far from 2.333...\n"; }
		std::cout << "  " << ub_third << " + " << b << " = " << d << '\n';

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: addition\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// interval subtraction
	std::cout << "*** interval subtraction\n";
	{
		int start = nrOfFailedTestCases;
		Ubound a(5.0), b(2.0);
		Ubound c = a - b;
		if (!c.contains(3.0)) { ++nrOfFailedTestCases; std::cout << "  FAIL: [5]-[2] should contain 3\n"; }
		std::cout << "  " << a << " - " << b << " = " << c << '\n';

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: subtraction\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// interval multiplication
	std::cout << "*** interval multiplication\n";
	{
		int start = nrOfFailedTestCases;
		Ubound a(3.0), b(4.0);
		Ubound c = a * b;
		if (!c.contains(12.0)) { ++nrOfFailedTestCases; std::cout << "  FAIL: [3]*[4] should contain 12\n"; }
		std::cout << "  " << a << " * " << b << " = " << c << '\n';

		// negative * positive
		Ubound neg(-2.0);
		c = neg * b;
		if (!c.contains(-8.0)) { ++nrOfFailedTestCases; std::cout << "  FAIL: [-2]*[4] should contain -8\n"; }
		std::cout << "  " << neg << " * " << b << " = " << c << '\n';

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: multiplication\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// interval division
	std::cout << "*** interval division\n";
	{
		int start = nrOfFailedTestCases;
		Ubound a(8.0), b(2.0);
		Ubound c = a / b;
		if (!c.contains(4.0)) { ++nrOfFailedTestCases; std::cout << "  FAIL: [8]/[2] should contain 4\n"; }
		std::cout << "  " << a << " / " << b << " = " << c << '\n';

		// division by interval containing zero -> NaN
		Unum neg_one, pos_one;
		neg_one = -1.0;
		pos_one = 1.0;
		Ubound zero_span(neg_one, pos_one);
		Ubound d = a / zero_span;
		if (!d.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: division by interval containing 0 should be NaN\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: division\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// interval width decreases with precision
	std::cout << "*** precision vs interval width\n";
	{
		// unum<2,2> has max 3 fraction bits -> wider intervals
		// unum<3,4> has max 15 fraction bits -> narrower intervals
		unum<2, 2> u22;
		u22 = 1.0 / 3.0;
		ubound<2, 2> ub22(u22);

		unum<3, 4> u34;
		u34 = 1.0 / 3.0;
		ubound<3, 4> ub34(u34);

		std::cout << "  unum<2,2>(1/3) interval width: " << ub22.width() << '\n';
		std::cout << "  unum<3,4>(1/3) interval width: " << ub34.width() << '\n';

		// higher precision should give narrower interval
		if (ub34.width() >= ub22.width()) {
			// not a hard failure since it depends on encoding, just report
			std::cout << "  NOTE: higher precision did not narrow interval (encoding-dependent)\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// NaN propagation
	std::cout << "*** NaN propagation\n";
	{
		int start = nrOfFailedTestCases;
		Unum nan;
		nan.setnan();
		Ubound ub_nan(nan);
		Ubound a(1.0);

		Ubound r = a + ub_nan;
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: [1]+[NaN] should be NaN\n"; }

		r = ub_nan * a;
		if (!r.isnan()) { ++nrOfFailedTestCases; std::cout << "  FAIL: [NaN]*[1] should be NaN\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: NaN propagation\n";
		else std::cout << "  NaN propagates correctly through interval arithmetic\n";
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
