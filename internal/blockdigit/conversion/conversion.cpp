// conversion.cpp: native type round-trip conversion tests for blockdigit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/internal/blockdigit/blockdigit.hpp>
#include <universal/internal/blockdecimal/blockdecimal.hpp>
#include <universal/verification/test_suite.hpp>

template<unsigned ndigits, unsigned radix>
int VerifyIntegerConversion(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// test a range of positive and negative values
	int testValues[] = { 0, 1, 2, 5, 9, 10, 42, 99, 100, 127, 255, 999, 1000, 12345 };
	for (int v : testValues) {
		blockdigit<ndigits, radix> a;
		a = v;
		int result = static_cast<int>(a);
		if (result != v) {
			if (reportTestCases) std::cerr << "FAIL: blockdigit<" << ndigits << ", " << radix << ">(" << v << ") = " << result << '\n';
			++nrOfFailedTestCases;
		}
		// negative
		a = -v;
		result = static_cast<int>(a);
		if (result != -v) {
			if (reportTestCases) std::cerr << "FAIL: blockdigit<" << ndigits << ", " << radix << ">(" << -v << ") = " << result << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<unsigned ndigits, unsigned radix>
int VerifyFloatConversion(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// blockdigit truncates floats to integer, so test integer-valued floats
	float testValues[] = { 0.0f, 1.0f, 42.0f, 100.0f, 255.0f, 1000.0f };
	for (float v : testValues) {
		blockdigit<ndigits, radix> a;
		a = v;
		float result = static_cast<float>(a);
		if (result != v) {
			if (reportTestCases) std::cerr << "FAIL: blockdigit<" << ndigits << ", " << radix << ">(" << v << ") = " << result << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<unsigned ndigits, unsigned radix>
int VerifyDoubleConversion(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// blockdigit truncates floats to integer, so test integer-valued floats
	double testValues[] = {0.0, 1.0, 42.0, 100.0, 255.0, 1000.0};
	for (double v : testValues) {
		blockdigit<ndigits, radix> a;
		a            = v;
		double result = static_cast<double>(a);
		if (result != v) {
			if (reportTestCases)
				std::cerr << "FAIL: blockdigit<" << ndigits << ", " << radix << ">(" << v << ") = " << result << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdigit conversion";
	std::string test_tag    = "blockdigit/conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "+---------    Integer round-trip: octal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 8>(reportTestCases);

	std::cout << "+---------    Double round-trip: decimal\n";
	{
		blockdecimal<8> a;
		a        = 12345;
		double d = static_cast<double>(a);
		if (d != 12345.0) {
			std::cerr << "FAIL: double round-trip for 12345\n";
			++nrOfFailedTestCases;
		}
	}

#else

#if REGRESSION_LEVEL_1
	std::cout << "+---------    Integer round-trip: octal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 8>(reportTestCases);

	std::cout << "+---------    Integer round-trip: decimal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 10>(reportTestCases);

	std::cout << "+---------    Integer round-trip: hexadecimal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 16>(reportTestCases);

	std::cout << "+---------    Float round-trip: octal\n";
	nrOfFailedTestCases += VerifyFloatConversion<8, 8>(reportTestCases);

	std::cout << "+---------    Float round-trip: decimal\n";
	nrOfFailedTestCases += VerifyFloatConversion<8, 10>(reportTestCases);

	std::cout << "+---------    Float round-trip: hexadecimal\n";
	nrOfFailedTestCases += VerifyFloatConversion<8, 16>(reportTestCases);

	std::cout << "+---------    Double round-trip: octal\n";
	nrOfFailedTestCases += VerifyDoubleConversion<8, 8>(reportTestCases);

	std::cout << "+---------    Double round-trip: decimal\n";
	nrOfFailedTestCases += VerifyDoubleConversion<8, 10>(reportTestCases);

	std::cout << "+---------    Double round-trip: hexadecimal\n";
	nrOfFailedTestCases += VerifyDoubleConversion<8, 16>(reportTestCases);

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING

}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
