// logic.cpp: comparison and logic operator tests for unum Type I
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

	std::string test_suite  = "unum Type I comparison and logic tests";
	std::string test_tag    = "unum logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Unum = unum<3, 4>;

	/////////////////////////////////////////////////////////////////////////////////////
	// equality
	std::cout << "*** equality\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b;

		// zero == zero
		a = 0.0; b = 0.0;
		if (!(a == b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0 == 0\n"; }

		// same value
		a = 1.5; b = 1.5;
		if (!(a == b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.5 == 1.5\n"; }

		// different values
		a = 1.0; b = 2.0;
		if (a == b) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 != 2.0\n"; }

		// negative equality
		a = -3.5; b = -3.5;
		if (!(a == b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: -3.5 == -3.5\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: equality\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// inequality
	std::cout << "*** inequality\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b;

		a = 1.0; b = 2.0;
		if (!(a != b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 != 2.0\n"; }

		a = 1.0; b = 1.0;
		if (a != b) { ++nrOfFailedTestCases; std::cout << "  FAIL: !(1.0 != 1.0)\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: inequality\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// less than
	std::cout << "*** less than\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b;

		a = 1.0; b = 2.0;
		if (!(a < b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 < 2.0\n"; }

		a = 2.0; b = 1.0;
		if (a < b) { ++nrOfFailedTestCases; std::cout << "  FAIL: !(2.0 < 1.0)\n"; }

		a = 1.0; b = 1.0;
		if (a < b) { ++nrOfFailedTestCases; std::cout << "  FAIL: !(1.0 < 1.0)\n"; }

		// negative values
		a = -2.0; b = -1.0;
		if (!(a < b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: -2.0 < -1.0\n"; }

		// cross sign
		a = -1.0; b = 1.0;
		if (!(a < b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: -1.0 < 1.0\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: less than\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// greater than
	std::cout << "*** greater than\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b;

		a = 2.0; b = 1.0;
		if (!(a > b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2.0 > 1.0\n"; }

		a = 1.0; b = 2.0;
		if (a > b) { ++nrOfFailedTestCases; std::cout << "  FAIL: !(1.0 > 2.0)\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: greater than\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// less than or equal
	std::cout << "*** less than or equal\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b;

		a = 1.0; b = 2.0;
		if (!(a <= b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 <= 2.0\n"; }

		a = 1.0; b = 1.0;
		if (!(a <= b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 <= 1.0\n"; }

		a = 2.0; b = 1.0;
		if (a <= b) { ++nrOfFailedTestCases; std::cout << "  FAIL: !(2.0 <= 1.0)\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: less than or equal\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// greater than or equal
	std::cout << "*** greater than or equal\n";
	{
		int start = nrOfFailedTestCases;
		Unum a, b;

		a = 2.0; b = 1.0;
		if (!(a >= b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2.0 >= 1.0\n"; }

		a = 1.0; b = 1.0;
		if (!(a >= b)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 >= 1.0\n"; }

		a = 1.0; b = 2.0;
		if (a >= b) { ++nrOfFailedTestCases; std::cout << "  FAIL: !(1.0 >= 2.0)\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: greater than or equal\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// NaN comparison semantics
	std::cout << "*** NaN comparison semantics\n";
	{
		int start = nrOfFailedTestCases;
		Unum nan, a;
		nan.setnan();
		a = 1.0;

		// NaN != NaN (IEEE semantics)
		if (nan == nan) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN == NaN should be false\n"; }
		if (!(nan != nan)) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN != NaN should be true\n"; }

		// NaN is not ordered
		if (nan < a) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN < 1.0 should be false\n"; }
		if (nan > a) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN > 1.0 should be false\n"; }
		if (nan <= a) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN <= 1.0 should be false\n"; }
		if (nan >= a) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN >= 1.0 should be false\n"; }

		// NaN compared to NaN
		if (nan < nan) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN < NaN should be false\n"; }
		if (nan > nan) { ++nrOfFailedTestCases; std::cout << "  FAIL: NaN > NaN should be false\n"; }

		// value compared to NaN
		if (a == nan) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 == NaN should be false\n"; }
		if (a < nan) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 < NaN should be false\n"; }
		if (a > nan) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1.0 > NaN should be false\n"; }

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: NaN semantics\n";
		}
		else {
			std::cout << "  all NaN comparisons follow IEEE semantics\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// zero comparisons
	std::cout << "*** zero comparisons\n";
	{
		int start = nrOfFailedTestCases;
		Unum zero, pos, neg;
		zero = 0.0;
		pos = 1.0;
		neg = -1.0;

		if (!(zero == zero)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0 == 0\n"; }
		if (!(zero < pos)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0 < 1\n"; }
		if (!(neg < zero)) { ++nrOfFailedTestCases; std::cout << "  FAIL: -1 < 0\n"; }
		if (!(zero >= zero)) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0 >= 0\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: zero comparisons\n";
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
