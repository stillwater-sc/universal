// bit_manipulation.cpp : test runner for bit manipulation of native integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/native/integers.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

		template<typename UnsignedInteger,
			typename = typename std::enable_if< std::is_unsigned<UnsignedInteger>::value, UnsignedInteger >::type>
		int VerifyNLZ(bool reportTestCases) {
			constexpr unsigned nbits = sizeof(UnsignedInteger) * 8;
			int nrOfFailedTests = 0;

			UnsignedInteger a(0);
			int shift = nlz(a);
			if (reportTestCases) std::cout << to_binary(a, nbits, true) << " : nlz shift " << shift << '\n';
			if (shift != nbits) ++nrOfFailedTests;

			a = UnsignedInteger(1);
			for (int i = 1; i <= static_cast<int>(nbits); ++i) {
				shift = nlz(a);
				if (reportTestCases) std::cout << to_binary(a, nbits, true) << " : nlz shift " << shift << '\n';
				if (shift != (static_cast<int>(nbits) - i)) ++nrOfFailedTests;
				a <<= 1;
			}

			return nrOfFailedTests;
		}

	}
} // namespace sw::universal

// test the nlz method which returns the shift required to move the leading non-zero into the most significant bit position of the type
void TestNLZ() {
	using namespace sw::universal;
	{
		uint8_t a = 0x1;
		for (uint32_t i = 0; i < 8; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 8) << '\n';
			a <<= 1;
		}
	}

	{
		uint16_t a = 0x1;
		for (uint32_t i = 0; i < 16; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 16) << '\n';
			a <<= 1;
		}
	}
	{
		uint32_t a = 0x1;
		for (uint32_t i = 0; i < 32; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 32) << '\n';
			a <<= 1;
		}
	}
	{
		uint64_t a = 0x1;
		for (uint32_t i = 0; i < 64; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 64) << '\n';
			a <<= 1;
		}
	}
}

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

	std::string test_suite  = "native integer bit manipulation verification";
	std::string test_tag    = "bit manipulators";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING


	TestNLZ();

	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint8_t>(reportTestCases), "std::uint8_t", "nlz");
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint16_t>(reportTestCases), "std::uint16_t", "nlz");
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint32_t>(reportTestCases), "std::uint32_t", "nlz");
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint64_t>(reportTestCases), "std::uint64_t", "nlz");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint8_t>(reportTestCases), "std::uint8_t", "nlz");
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint16_t>(reportTestCases), "std::uint16_t", "nlz");
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint32_t>(reportTestCases), "std::uint32_t", "nlz");
	nrOfFailedTestCases += ReportTestResult(VerifyNLZ<uint64_t>(reportTestCases), "std::uint64_t", "nlz");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if	REGRESSION_LEVEL_4

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
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
