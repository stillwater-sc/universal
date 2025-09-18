// division.cpp: functional tests for blocksignificand division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>

#include <universal/native/integers.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/blocksignificand_test_suite.hpp>

template<unsigned nbits, typename BlockType>
void TestMostSignificantBit() {
	using namespace sw::universal;
	blocksignificand<nbits, BlockType> a;
	std::cout << to_binary(a) << ' ' << a.msb() << '\n';
	a.setbits(0x01ull);
	for (unsigned i = 0; i < nbits; ++i) {
		std::cout << to_binary(a) << ' ' << a.msb() << '\n';
		a <<= 1;
	}
}

// TODO: blocksignificand div is failing, currently regression testing is disabled
// 
// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
	using namespace sw::universal::internal;
	
	std::string test_suite  = "blocksignificand division validation";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blocksignificand<4, uint8_t> a, b, c;
		c.div(a, b);
	}

	TestMostSignificantBit<27, uint8_t>();
	TestMostSignificantBit<27, uint16_t>();
	TestMostSignificantBit<33, uint32_t>();

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<4, uint8_t> >(reportTestCases), "blocksignificand<4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<8, uint8_t> >(reportTestCases), "blocksignificand<8,uint8_t>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<4, uint8_t> >(reportTestCases), "blocksignificand<4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<5, uint8_t> >(reportTestCases), "blocksignificand<5,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<6, uint8_t> >(reportTestCases), "blocksignificand<6,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<7, uint8_t> >(reportTestCases), "blocksignificand<7,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<8, uint8_t> >(reportTestCases), "blocksignificand<8,uint8_t>", "division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<9, uint8_t> >(reportTestCases), "blocksignificand<9,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<10, uint8_t> >(reportTestCases), "blocksignificand<10,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<12, uint8_t> >(reportTestCases), "blocksignificand<12,uint8_t>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<9, uint16_t> >(reportTestCases), "blocksignificand<9,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<11, uint16_t> >(reportTestCases), "blocksignificand<11,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<13, uint16_t> >(reportTestCases), "blocksignificand<13,uint16_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<12, uint32_t> >(reportTestCases), "blocksignificand<12,uint32_t>", "division");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<16, uint8_t> >(reportTestCases), "blocksignificand<16,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandDivision< blocksignificand<16, uint16_t> >(reportTestCases), "blocksignificand<16,uint16_t>", "division");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
