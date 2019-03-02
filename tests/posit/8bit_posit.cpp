// 8bit_posit.cpp: Functionality tests for standard 8-bit posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// Configure the posit template environment
// first: enable fast specialized posit<8,0>
#define POSIT_FAST_SPECIALIZATION
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"

/*
Standard posits with nbits = 8 have no exponent bits, i.e. es = 0.
*/

#define signP8UI( a ) ((bool) ((uint8_t) (a)>>7))
#define signregP8UI( a ) ((bool) (((uint8_t) (a)>>6) & 0x1))
#define packToP8UI( regime, fracA) ((uint8_t) regime + ((uint8_t)(fracA)) )

// mul reference from SoftPosit
using posit8_t = uint8_t;

posit8_t p8_mul(posit8_t pA, posit8_t pB) {
	using namespace std;
	posit8_t uA, uB, uZ;
	uint_fast8_t uiA, uiB;
	uint_fast8_t regA, fracA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t kA = 0;
	uint_fast16_t frac16Z;

	uA = pA;
	uiA = pA;
	uB = pB;
	uiB = pB;

	cout << hex;
	cout << "a = 0x" << int(uiA) << endl;
	cout << "b = 0x" << int(uiB) << endl;
	cout << dec;

	//NaR or Zero
	if (uiA == 0x80 || uiB == 0x80) {
		uZ = 0x80;
		return uZ;
	}
	else if (uiA == 0 || uiB == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP8UI(uiA);
	signB = signP8UI(uiB);
	signZ = signA ^ signB;

	if (signA) uiA = (-uiA & 0xFF);
	if (signB) uiB = (-uiB & 0xFF);

	regSA = signregP8UI(uiA);
	regSB = signregP8UI(uiB);

	tmp = (uiA << 2) & 0xFF;
	if (regSA) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	fracA = (0x80 | tmp);

	tmp = (uiB << 2) & 0xFF;
	if (regSB) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA--;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16Z = (uint_fast16_t)fracA * (0x80 | tmp);
	cout << hex << (frac16Z) << dec << endl;

	rcarry = frac16Z >> 15;//1st bit of frac32Z
	if (rcarry) {
		kA++;
		frac16Z >>= 1;
	}

	if (kA<0) {
		regA = (-kA & 0xFF);
		regSA = 0;
		regime = 0x40 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7F - (0x7F >> regA);
	}

	if (regA>6) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7F) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position
		frac16Z = (frac16Z & 0x3FFF) >> regA;
		fracA = (uint_fast8_t)(frac16Z >> 8);
		bitNPlusOne = (0x80 & frac16Z);
		uZ = packToP8UI(regime, fracA);

		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7F & frac16Z) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (signZ) uZ = -uZ & 0xFF;
	return uZ;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	const size_t nbits = 8;
	const size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<8,0>";

#if defined(POSIT_FAST_POSIT_8_0)
	cout << "Fast specialization posit<8,0> configuration tests" << endl;
#else
	cout << "Standard posit<8,0> configuration tests" << endl;
#endif

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl;

	// logic tests
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual             <nbits, es>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan          <nbits, es>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         ");
	// conversion tests
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion       <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   ");
	// arithmetic tests
	nrOfFailedTestCases += ReportTestResult( ValidateAddition         <nbits, es>(tag, bReportIndividualTestCases), tag, "add            ");
	nrOfFailedTestCases += ReportTestResult( ValidateSubtraction      <nbits, es>(tag, bReportIndividualTestCases), tag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult( ValidateMultiplication   <nbits, es>(tag, bReportIndividualTestCases), tag, "multiply       ");
//	nrOfFailedTestCases += ReportTestResult( ValidateDivision         <nbits, es>(tag, bReportIndividualTestCases), tag, "divide         ");
	nrOfFailedTestCases += ReportTestResult( ValidateNegation         <nbits, es>(tag, bReportIndividualTestCases), tag, "negate         ");
//	nrOfFailedTestCases += ReportTestResult( ValidateReciprocation    <nbits, es>(tag, bReportIndividualTestCases), tag, "reciprocate    ");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt              <nbits, es>(tag, bReportIndividualTestCases), tag, "sqrt           ");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
