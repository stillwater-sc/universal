// addition.cpp: functional tests for blocksignificand addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/blocksignificand_test_suite.hpp>


// generate specific test case that you can trace with the trace conditions in blocksignificand
// for most bugs they are traceable with _trace_conversion and _trace_add
template<unsigned nbits, typename BlockType>
void GenerateTestCase(const sw::universal::blocksignificand<nbits, BlockType>& lhs, const sw::universal::blocksignificand <nbits, BlockType>& rhs) {
	using namespace sw::universal;

	blocksignificand<nbits, BlockType> a, b, c;

	a = lhs;
	b = rhs;
	c.add(a, b);

	double _a, _b, _c;
	_a = double(a);
	_b = double(b);
	_c = _a + _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << lhs << " + " << std::setw(nbits) << rhs 
		<< " = " << std::setw(nbits) << c << '\n';
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b 
		<< " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " + " << to_binary(b) 
		<< " = " << to_binary(c) << " (reference: " << _c << ")   " << '\n';
	double cref = double(c);
	std::cout << (_c == cref ? "PASS" : "FAIL") << '\n' << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

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
		
	std::string test_suite  = "blocksignificand addition validation";
	std::string test_tag    = "addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockbinary<8, uint8_t> refResult;
		refResult = 0;
		std::cout << to_binary(refResult) << '\n';
	}

	{
		blocksignificand<8, uint32_t> a; // BitEncoding::Twos
		a.setbits(0x41);
		std::cout << a << " : " << to_binary(a) << " : " << float(a) << '\n';
	}

	blocksignificand<23, uint32_t> a, b;

	// generate individual testcases to hand trace/debug
	GenerateTestCase(a, b);


	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<8, uint8_t> >(reportTestCases),   "blocksignificand<  8, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<12, uint8_t> >(reportTestCases),  "blocksignificand< 12, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<12, uint16_t> >(reportTestCases), "blocksignificand< 12, uint16_t>", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<4, uint8_t> >(reportTestCases),  "blocksignificand< 4, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<4, uint16_t> >(reportTestCases), "blocksignificand< 4, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<4, uint32_t> >(reportTestCases), "blocksignificand< 4, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<8, uint8_t> >(reportTestCases),  "blocksignificand< 8, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<8, uint16_t> >(reportTestCases), "blocksignificand< 8, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<8, uint32_t> >(reportTestCases), "blocksignificand< 8, uint32_t>", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<9, uint8_t> >(reportTestCases),  "blocksignificand< 9, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<9, uint16_t> >(reportTestCases), "blocksignificand< 9, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<9, uint32_t> >(reportTestCases), "blocksignificand< 9, uint32_t>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<10, uint8_t> >(reportTestCases),  "blocksignificand<10, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<10, uint16_t> >(reportTestCases), "blocksignificand<10, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<10, uint32_t> >(reportTestCases), "blocksignificand<10, uint32_t>", "addition");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<11, uint8_t> >(reportTestCases),  "blocksignificand<11, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<11, uint16_t> >(reportTestCases), "blocksignificand<11, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<11, uint32_t> >(reportTestCases), "blocksignificand<11, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<12, uint8_t> >(reportTestCases),  "blocksignificand<12, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<12, uint16_t> >(reportTestCases), "blocksignificand<12, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificandAddition< blocksignificand<12, uint32_t> >(reportTestCases), "blocksignificand<12, uint32_t>", "addition");
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
