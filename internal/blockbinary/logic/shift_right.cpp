// shift_right.cpp: functional tests for block binary number shifts
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

// enumerate all shift right cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyArithmeticRightShift(bool reportTestCases) {
	using namespace sw::universal;
	using BlockBinary = blockbinary<nbits, BlockType>;

	if (reportTestCases) std::cout << type_tag(BlockBinary()) << '\n';

	// take maxneg and shift it right in all possible strides
	int nrOfFailedTests = 0;
	BlockBinary a, result;
	BlockBinary mostNegative; maxneg(mostNegative);
	int64_t shiftRef, resultRef;
	for (size_t i = 0; i < nbits+1; i++) {
		a = mostNegative;
		int64_t denominator = 0;
		if (i == 64) { 
			shiftRef = 0;
		}
		else if (i == 63) { // special case for int64_t shift as it is maxneg
			shiftRef = -1;
		}
		else { // i < 63
			denominator = (1ll << i);
			shiftRef = ((long long)a / denominator);
		}

		result = a >> long(i);
		resultRef = (long long)result;

		if (shiftRef != resultRef) {
			nrOfFailedTests++;
			if (reportTestCases) ReportArithmeticShiftError("FAIL", ">>", a, i, result, resultRef);
		}
		else {
			if (reportTestCases) ReportArithmeticShiftSuccess("PASS", ">>", a, i, result, resultRef);
		}
		if (nrOfFailedTests > 100) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

void ShiftExamples() {
	using namespace sw::universal;

	blockbinary<37, uint8_t> a;
	blockbinary<37, uint16_t> b;
	blockbinary<37, uint32_t> c;
	//	blockbinary<37, int64_t> d;

	a.setbits(0xAAAAAAAAAA);
	b.setbits(0x5555555555);
	c.setbits(0xAAAAAAAAAA);
	//	d.set_raw_bits(0x5555555555);

	std::cout << to_binary(a, true) << '\n';
	std::cout << to_binary(b, true) << '\n';
	std::cout << to_binary(c, true) << '\n';
	std::cout << to_hex(a, true) << '\n';
	std::cout << to_hex(b, true) << '\n';
	std::cout << to_hex(c, true) << '\n';
	//	std::cout << to_hex(d, true) << '\n';

	std::cout << "shifting\n";
	a.setbits(0x155555555);
	std::cout << to_binary(a, true) << '\n';
	a <<= 1;
	std::cout << to_binary(a, true) << '\n';
	a <<= 1;
	std::cout << to_binary(a, true) << '\n';
	a <<= 1;
	std::cout << to_binary(a, true) << '\n';
	a <<= 1;
	std::cout << to_binary(a, true) << '\n';
	a >>= 4;
	std::cout << to_binary(a, true) << '\n';
	a >>= 9;
	std::cout << to_binary(a, true) << '\n';

	b.setbits(0x155555555);
	std::cout << to_binary(b, true) << '\n';
	b <<= 1;
	std::cout << to_binary(b, true) << '\n';
	b <<= 1;
	std::cout << to_binary(b, true) << '\n';
	b <<= 1;
	std::cout << to_binary(b, true) << '\n';
	b <<= 1;
	std::cout << to_binary(b, true) << '\n';
	b >>= 4;
	std::cout << to_binary(b, true) << '\n';
	b >>= 17;
	std::cout << to_binary(b, true) << '\n';
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
	
	std::string test_suite  = "blockbinary arithmetic right shifting";
	std::string test_tag    = "arithmetic right shift";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	ShiftExamples();

	{
		// sign-extended right shift as we are a 2's complement representation
		blockbinary<32> a;
		for (int i = 0; i < 32; ++i) {
			a.setbits(0x8000'0000u);
			a >>= i;
			std::cout << to_binary(a, true) << ' ' << long(a) << '\n';
		}
		for (int i = 0; i < 32; ++i) {
			a.setbits(0xFFFF'FFFFu);
			a <<= i;
			std::cout << to_binary(a, true) << ' ' << long(a) << '\n';
		}
	}

	/* FAIL
	b1111'1110'0000 -32  right shift by 0
	b1111'1111'0000 -16  right shift by 1
	b1111'1111'1000 -8  right shift by 2
	b1111'1111'1100 -4  right shift by 3
	b1111'1111'1110 -2  right shift by 4
	b0111'1111'1111 2047  right shift by 5
	b0011'1111'1111 1023  right shift by 6
	b0001'1111'1111 511  right shift by 7
	b1111'0000'1111 -241  right shift by 8
	b1111'1000'0111 -121  right shift by 9
	b1111'1100'0011 -61  right shift by 10
	b1111'1110'0001 -31  right shift by 11
	b1111'1111'0000 -16  right shift by 12
	b1111'1111'0000 -16  right shift by 13
	b1111'1111'0000 -16  right shift by 14
	b1111'1111'0000 -16  right shift by 15
	 */
	blockbinary<12> a;
	for (int i = 0; i < 16; ++i) {
		a.setbits(0x0fe0);
		a >>= i;
		std::cout << to_binary(a, true) << ' ' << (long long)(a) << "  right shift by " << i << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<12>(true), "blockbinary<12>", test_tag);

	{
		blockbinary<12> a;
		a.setbits(0x800);
		std::cout << to_hex(a) << ' ';
		a >>= 8;
		std::cout << to_hex(a) << '\n';
	}
	{
		blockbinary<8> a;
		for (int i = 0; i < 16; ++i) {
			a.setbits(0x80);
			a >>= i;
			std::cout << to_binary(a, true) << ' ' << (long long)(a) << "  right shift by " << i << '\n';
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<2>(reportTestCases), "blockbinary<2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<3>(reportTestCases), "blockbinary<3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<4>(reportTestCases), "blockbinary<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<5>(reportTestCases), "blockbinary<5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<6>(reportTestCases), "blockbinary<6>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<7>(reportTestCases), "blockbinary<7>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<8>(reportTestCases), "blockbinary<8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<9>(reportTestCases), "blockbinary<9>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<10>(reportTestCases), "blockbinary<10>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<11>(reportTestCases), "blockbinary<11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<12>(reportTestCases), "blockbinary<12>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<13>(reportTestCases), "blockbinary<13>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<14>(reportTestCases), "blockbinary<14>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<15>(reportTestCases), "blockbinary<15>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<16>(reportTestCases), "blockbinary<16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<17>(reportTestCases), "blockbinary<17>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<18>(reportTestCases), "blockbinary<18>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<20>(reportTestCases), "blockbinary<20>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<24>(reportTestCases), "blockbinary<24>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<28>(reportTestCases), "blockbinary<28>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<32>(reportTestCases), "blockbinary<32>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<40>(reportTestCases), "blockbinary<40>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<48>(reportTestCases), "blockbinary<48>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<56>(reportTestCases), "blockbinary<56>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<64>(reportTestCases), "blockbinary<64>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// using a more efficient storage class
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<32,uint32_t>(reportTestCases), "blockbinary<32,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<40,uint32_t>(reportTestCases), "blockbinary<40,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<48, uint32_t>(reportTestCases), "blockbinary<48,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<56, uint32_t>(reportTestCases), "blockbinary<56,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<64, uint32_t>(reportTestCases), "blockbinary<64,uint32_t>", test_tag);
#endif

#if	REGRESSION_LEVEL_4	
	// can't test this with VerifyArithmeticRightShift since we don't have a >64bit native integer type
	//nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<128, uint32_t>(reportTestCases), "blockbinary<128,uint32_t>", test_tag);
	//nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<256, uint32_t>(reportTestCases), "blockbinary<256,uint32_t>", test_tag);

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
