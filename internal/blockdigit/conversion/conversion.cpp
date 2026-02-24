// conversion.cpp: native type round-trip conversion tests for blockdigit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/internal/blockdigit/blockdigit.hpp>
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdigit conversion";
	std::string test_tag    = "blockdigit/conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::cout << "+---------    Integer round-trip: octal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 8>(reportTestCases);

	std::cout << "+---------    Integer round-trip: decimal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 10>(reportTestCases);

	std::cout << "+---------    Integer round-trip: hexadecimal\n";
	nrOfFailedTestCases += VerifyIntegerConversion<8, 16>(reportTestCases);

	std::cout << "+---------    Float round-trip: decimal\n";
	nrOfFailedTestCases += VerifyFloatConversion<8, 10>(reportTestCases);

	std::cout << "+---------    Double round-trip: decimal\n";
	{
		blockdecimal_t<8> a;
		a = 12345;
		double d = static_cast<double>(a);
		if (d != 12345.0) {
			std::cerr << "FAIL: double round-trip for 12345\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
