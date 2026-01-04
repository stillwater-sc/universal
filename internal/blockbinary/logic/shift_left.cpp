// shift_left.cpp: functional tests for block binary number shifts
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
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/blockbinary_test_status.hpp>

// enumerate all shift left cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyLeftShift(bool reportTestCases) {
	using namespace sw::universal;
	using BlockBinary = blockbinary<nbits, BlockType>;

	if (reportTestCases) std::cout << type_tag(BlockBinary()) << '\n';

	// take 1 and shift it left in all possible strides
	int nrOfFailedTests = 0;
	BlockBinary a, result;
	int64_t shiftRef, resultRef;
	for (size_t i = 0; i < nbits + 1; i++) {
		shiftRef = (0xFFFF'FFFF'FFFF'FFFFll << i);
		if (i == nbits) shiftRef = 0; // shift all bits out

		a = -1;
		result = a << long(i);
		resultRef = (long long)result;

		if (shiftRef != resultRef) {
			nrOfFailedTests++;
			if (reportTestCases) ReportArithmeticShiftError("FAIL", "<<", a, i, result, resultRef);
		}
		else {
			if (reportTestCases) ReportArithmeticShiftSuccess("PASS", "<<", a, i, result, resultRef);
		}
		if (nrOfFailedTests > 100) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
#undef REGRESSION_LEVEL_OVERRIDE
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
	
	std::string test_suite  = "blockbinary logic left shifting";
	std::string test_tag    = "arithmetic/logic left shift";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<12>(true), "blockbinary<12>", test_tag);

	{
		blockbinary<12> a;
		a.setbits(0x800);
		std::cout << to_hex(a) << ' ';
		a <<= 8;
		std::cout << to_hex(a) << '\n';
	}
	{
		blockbinary<8> a;
		for (int i = 0; i < 16; ++i) {
			a.setbits(0x80);
			a <<= i;
			std::cout << to_binary(a, true) << ' ' << (long long)(a) << "  right shift by " << i << '\n';
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<2>(reportTestCases), "blockbinary<2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<3>(reportTestCases), "blockbinary<3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<4>(reportTestCases), "blockbinary<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<5>(reportTestCases), "blockbinary<5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<6>(reportTestCases), "blockbinary<6>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<7>(reportTestCases), "blockbinary<7>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<8>(reportTestCases), "blockbinary<8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<9>(reportTestCases), "blockbinary<9>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<10>(reportTestCases), "blockbinary<10>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<11>(reportTestCases), "blockbinary<11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<12>(reportTestCases), "blockbinary<12>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<13>(reportTestCases), "blockbinary<13>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<14>(reportTestCases), "blockbinary<14>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<15>(reportTestCases), "blockbinary<15>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<16>(reportTestCases), "blockbinary<16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<17>(reportTestCases), "blockbinary<17>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<18>(reportTestCases), "blockbinary<18>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<20>(reportTestCases), "blockbinary<20>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<24>(reportTestCases), "blockbinary<24>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<28>(reportTestCases), "blockbinary<28>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<32>(reportTestCases), "blockbinary<32>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<40>(reportTestCases), "blockbinary<40>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<48>(reportTestCases), "blockbinary<48>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<56>(reportTestCases), "blockbinary<56>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<64>(reportTestCases), "blockbinary<64>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// using a more efficient storage class
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<32,uint32_t>(reportTestCases), "blockbinary<32,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<40,uint32_t>(reportTestCases), "blockbinary<40,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<48, uint32_t>(reportTestCases), "blockbinary<48,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<56, uint32_t>(reportTestCases), "blockbinary<56,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<64, uint32_t>(reportTestCases), "blockbinary<64,uint32_t>", test_tag);
#endif

#if	REGRESSION_LEVEL_4	
	// can't test this with VerifyLeftShift since we don't have a >64bit native integer type
	//nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<128, uint32_t>(reportTestCases), "blockbinary<128,uint32_t>", test_tag);
	//nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<256, uint32_t>(reportTestCases), "blockbinary<256,uint32_t>", test_tag);

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
