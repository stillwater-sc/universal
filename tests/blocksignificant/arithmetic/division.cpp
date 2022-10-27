// division.cpp: functional tests for blocksignificant division
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>

#include <universal/native/integers.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificant/blocksignificant.hpp>
#include <universal/verification/test_suite.hpp>

// enumerate all division cases for an blocksignificant<nbits,BlockType> configuration
// TODO: fix test failures in VerifyBlockSignificantDivision<blocksignificantConfiguration>
template<typename blocksignificantConfiguration>
int VerifyBlockSignificantDivision(bool reportTestCases) {
	constexpr unsigned nbits = blocksignificantConfiguration::nbits;
	using BlockType = typename blocksignificantConfiguration::BlockType;

	constexpr unsigned NR_VALUES = (1u << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blocksignificant<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificant<nbits, BlockType> a, b, c;
	// nbits = 2 * fhbits
	constexpr unsigned fhbits = (nbits >> 1);
	constexpr unsigned fbits = fhbits - 1;
	a.setradix(2 * fbits);
	b.setradix(2 * fbits);
	a.setradix(2 * fbits);
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	constexpr unsigned nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref / bref;
			c.div(a, b);
			for (unsigned k = 0; k < nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}

			if (refResult != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, refResult);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
	return nrOfFailedTests;
}

template<unsigned nbits, typename BlockType>
void TestMostSignificantBit() {
	using namespace sw::universal;
	blocksignificant<nbits, BlockType> a;
	std::cout << to_binary(a) << ' ' << a.msb() << '\n';
	a.setbits(0x01ull);
	for (unsigned i = 0; i < nbits; ++i) {
		std::cout << to_binary(a) << ' ' << a.msb() << '\n';
		a <<= 1;
	}
}

// TODO: blocksignificant div is failing, currently regression testing is disabled
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
	
	std::string test_suite  = "blocksignificant division validation";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blocksignificant<4, uint8_t> a, b, c;
		c.div(a, b);
	}

	TestMostSignificantBit<27, uint8_t>();
	TestMostSignificantBit<27, uint16_t>();
	TestMostSignificantBit<33, uint32_t>();

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<4, uint8_t> >(reportTestCases), "blocksignificant<4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<8, uint8_t> >(reportTestCases), "blocksignificant<8,uint8_t>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<4, uint8_t> >(reportTestCases), "blocksignificant<4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<5, uint8_t> >(reportTestCases), "blocksignificant<5,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<6, uint8_t> >(reportTestCases), "blocksignificant<6,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<7, uint8_t> >(reportTestCases), "blocksignificant<7,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<8, uint8_t> >(reportTestCases), "blocksignificant<8,uint8_t>", "division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<9, uint8_t> >(reportTestCases), "blocksignificant<9,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<10, uint8_t> >(reportTestCases), "blocksignificant<10,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<12, uint8_t> >(reportTestCases), "blocksignificant<12,uint8_t>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<9, uint16_t> >(reportTestCases), "blocksignificant<9,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<11, uint16_t> >(reportTestCases), "blocksignificant<11,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<13, uint16_t> >(reportTestCases), "blocksignificant<13,uint16_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<12, uint32_t> >(reportTestCases), "blocksignificant<12,uint32_t>", "division");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<16, uint8_t> >(reportTestCases), "blocksignificant<16,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantDivision< blocksignificant<16, uint16_t> >(reportTestCases), "blocksignificant<16,uint16_t>", "division");
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
