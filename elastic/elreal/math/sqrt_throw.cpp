// sqrt_throw.cpp: tests for elreal_negative_sqrt_arg exception (Phase E.2)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Companion to math/sqrt.cpp. The default-mode tests there exercise the
// IEEE-754 fall-through path (sqrt(-x) -> NaN). This file flips
// ELREAL_THROW_ARITHMETIC_EXCEPTION on and asserts the catchable
// elreal_negative_sqrt_arg exception path.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase E.2 sqrt negative-arg exception";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// sqrt(-1) must throw
	{
		elreal a(-1.0);
		bool threw = false;
		try {
			elreal r = sqrt(a);
			(void)r;
		}
		catch (const elreal_negative_sqrt_arg&) {
			threw = true;
		}
		if (!threw) {
			std::cerr << "FAIL: sqrt(-1) did not throw elreal_negative_sqrt_arg "
				<< "with ELREAL_THROW_ARITHMETIC_EXCEPTION set\n";
			++nrOfFailedTestCases;
		}
	}

	// sqrt(0) does not throw (zero is the boundary; IEEE-754 sqrt(0) = 0)
	{
		elreal zero;
		bool threw = false;
		double result_val = -1.0;
		try {
			result_val = double(sqrt(zero));
		}
		catch (...) {
			threw = true;
		}
		if (threw || result_val != 0.0) {
			std::cerr << "FAIL: sqrt(0) in throw mode (threw=" << threw
				<< " result=" << result_val << ")\n";
			++nrOfFailedTestCases;
		}
	}

	// Non-negative input still works in throw mode
	{
		elreal a(9.0);
		bool threw = false;
		double result = 0.0;
		try {
			result = double(sqrt(a));
		}
		catch (...) {
			threw = true;
		}
		if (threw) {
			std::cerr << "FAIL: sqrt(9) threw in throw mode (should only throw on negative)\n";
			++nrOfFailedTestCases;
		}
		if (result != 3.0) {
			std::cerr << "FAIL: sqrt(9) != 3 in throw mode (got " << result << ")\n";
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
