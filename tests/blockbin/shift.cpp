// shift.cpp: functional tests for block binary number shifts
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/blockbin/blockbinary.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/blockbinary_helpers.hpp"

// enumerate all addition cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyArithmeticRightShift(std::string tag, bool bReportIndividualTestCases) {
	using namespace std;
	using namespace sw::unum;

	cout << endl;
	cout << "blockbinary<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;
	cout << typeid(blockbinary<nbits, BlockType>).name() << endl;

	// take maxneg and shift it right in all possible strides
	bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> a, result;
	blockbinary<nbits, BlockType> mostNegative = maxneg<nbits, BlockType>();
	int64_t shiftRef, resultRef;
	for (size_t i = 0; i < nbits+1; i++) {
		a = mostNegative;
		int64_t denominator = (1ll << i);
		if (i == 63) { // special case for int64_t shift
			shiftRef = -1;
		}
		else if (i == 64) {
			shiftRef = 0;
		}
		else {
			shiftRef = ((long long)a / denominator);
		}

		result = a >> long(i);
		resultRef = (long long)result;

		if (shiftRef != resultRef) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportArithmeticShiftError("FAIL", ">>", a, i, result, resultRef);
		}
		else {
			if (bReportIndividualTestCases) ReportArithmeticShiftSuccess("PASS", ">>", a, i, result, resultRef);
		}
		if (nrOfFailedTests > 100) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

void ShiftExamples() {
	using namespace std;
	using namespace sw::unum;

	blockbinary<37, uint8_t> a;
	blockbinary<37, uint16_t> b;
	blockbinary<37, uint32_t> c;
	//	blockbinary<37, int64_t> d;

	a.set_raw_bits(0xAAAAAAAAAA);
	b.set_raw_bits(0x5555555555);
	c.set_raw_bits(0xAAAAAAAAAA);
	//	d.set_raw_bits(0x5555555555);

	cout << to_binary(a, true) << endl;
	cout << to_binary(b, true) << endl;
	cout << to_binary(c, true) << endl;
	cout << to_hex(a, true) << endl;
	cout << to_hex(b, true) << endl;
	cout << to_hex(c, true) << endl;
	//	cout << to_hex(d, true) << endl;

	cout << "shifting\n";
	a.set_raw_bits(0x155555555);
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a >>= 4;
	cout << to_binary(a, true) << endl;
	a >>= 9;
	cout << to_binary(a, true) << endl;

	b.set_raw_bits(0x155555555);
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b >>= 4;
	cout << to_binary(b, true) << endl;
	b >>= 17;
	cout << to_binary(b, true) << endl;
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "block shifts: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
#define NOW_
#ifdef NOW
	ShiftExamples();

	{
		// sign-extended right shift as we are a 2's complement representation
		blockbinary<32> a;
		for (int i = 0; i < 32; ++i) {
			a.set_raw_bits(0x80000000ull);
			a >>= i;
			cout << to_binary(a, true) << ' ' << (long long)(a) << endl;
		}
		for (int i = 0; i < 32; ++i) {
			a.set_raw_bits(0xFFFFFFFFull);
			a <<= i;
			cout << to_binary(a, true) << ' ' << (long long)(a) << endl;
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
		a.set_raw_bits(0x0fe0);
		a >>= i;
		cout << to_binary(a, true) << ' ' << (long long)(a) << "  right shift by " << i << endl;
	}
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<12>("Manual Testing", true), "blockbinary<12>", "arithmetic right shift");

	{
		blockbinary<12> a;
		a = maxneg<12, uint8_t>();
		cout << to_hex(a) << ' ';
		a >>= 8;
		cout << to_hex(a) << endl;
	}
	{
		blockbinary<8> a;
		for (int i = 0; i < 16; ++i) {
			a = maxneg<8>();
			a >>= i;
			cout << to_binary(a, true) << ' ' << (long long)(a) << "  right shift by " << i << endl;
		}
	}

#if STRESS_TESTING

#endif

#else

	cout << "block shifts validation" << endl;
	bReportIndividualTestCases = false;
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<2>("Manual Testing", bReportIndividualTestCases), "blockbinary<2>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<3>("Manual Testing", bReportIndividualTestCases), "blockbinary<3>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<4>("Manual Testing", bReportIndividualTestCases), "blockbinary<4>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<5>("Manual Testing", bReportIndividualTestCases), "blockbinary<5>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<6>("Manual Testing", bReportIndividualTestCases), "blockbinary<6>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<7>("Manual Testing", bReportIndividualTestCases), "blockbinary<7>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<8>("Manual Testing", bReportIndividualTestCases), "blockbinary<8>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<9>("Manual Testing", bReportIndividualTestCases), "blockbinary<9>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<10>("Manual Testing", bReportIndividualTestCases), "blockbinary<10>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<11>("Manual Testing", bReportIndividualTestCases), "blockbinary<11>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<12>("Manual Testing", bReportIndividualTestCases), "blockbinary<12>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<13>("Manual Testing", bReportIndividualTestCases), "blockbinary<13>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<14>("Manual Testing", bReportIndividualTestCases), "blockbinary<14>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<15>("Manual Testing", bReportIndividualTestCases), "blockbinary<15>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<16>("Manual Testing", bReportIndividualTestCases), "blockbinary<16>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<17>("Manual Testing", bReportIndividualTestCases), "blockbinary<17>", "arithmetic right shift");

	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<18>("Manual Testing", bReportIndividualTestCases), "blockbinary<18>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<20>("Manual Testing", bReportIndividualTestCases), "blockbinary<20>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<24>("Manual Testing", bReportIndividualTestCases), "blockbinary<24>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<28>("Manual Testing", bReportIndividualTestCases), "blockbinary<28>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<32>("Manual Testing", bReportIndividualTestCases), "blockbinary<32>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<40>("Manual Testing", bReportIndividualTestCases), "blockbinary<40>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<48>("Manual Testing", bReportIndividualTestCases), "blockbinary<48>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<56>("Manual Testing", bReportIndividualTestCases), "blockbinary<56>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<64>("Manual Testing", bReportIndividualTestCases), "blockbinary<64>", "arithmetic right shift");

	// using a more efficient storage class
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<32,uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<32,uint32_t>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<40,uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<40,uint32_t>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<48, uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<48,uint32_t>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<56, uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<56,uint32_t>", "arithmetic right shift");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<64, uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<64,uint32_t>", "arithmetic right shift");
	
	// can't test this with VerifyArithmeticRightShift since we don't have a >64bit native integer type
	//nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<128, uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<128,uint32_t>", "arithmetic right shift");
	//nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<256, uint32_t>("Manual Testing", bReportIndividualTestCases), "blockbinary<256,uint32_t>", "arithmetic right shift");


#if STRESS_TESTING



#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
