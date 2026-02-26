// subtraction.cpp: verify subtraction of dfloat decimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "dfloat<> subtraction validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Basic subtraction
	std::cout << "+---------    Basic subtraction\n";
	{
		struct TestCase { double a; double b; double expected; };
		TestCase cases[] = {
			{ 0, 0, 0 },
			{ 1, 0, 1 },
			{ 0, 1, -1 },
			{ 5, 3, 2 },
			{ 3, 5, -2 },
			{ 100, 1, 99 },
			{ 1000, 999, 1 },
			{ -5, -3, -2 },
			{ -3, -5, 2 },
			{ 42, 42, 0 },
		};
		for (const auto& tc : cases) {
			Decimal32 a(tc.a), b(tc.b);
			Decimal32 result = a - b;
			double d = double(result);
			if (d != tc.expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << tc.a << " - " << tc.b << " = " << d
				                               << " (expected " << tc.expected << ")\n";
			}
		}
	}

	// Test 2: Anti-commutativity: a - b == -(b - a)
	std::cout << "+---------    Anti-commutativity: a - b == -(b - a)\n";
	{
		double values[] = { 1, 42, -7, 0.5, 100 };
		for (double va : values) {
			for (double vb : values) {
				Decimal32 a(va), b(vb);
				Decimal32 ab = a - b;
				Decimal32 ba = -(b - a);
				if (double(ab) != double(ba)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << va << " - " << vb
					                               << " = " << double(ab) << " but -(b-a) = " << double(ba) << '\n';
				}
			}
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
