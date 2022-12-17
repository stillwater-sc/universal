// multiplication.cpp: functional tests for blocksignificant multiplication
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <bitset>      // not used: just here to access the API

#include <universal/native/integers.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificant/blocksignificant.hpp>
#include <universal/verification/test_suite.hpp>

// enumerate all addition cases for an blocksignificant<nbits,BlockType> configuration
template<typename blocksignificantConfiguration>
int VerifyBlockSignificantMultiplication(bool reportTestCases) {
	constexpr unsigned nbits = blocksignificantConfiguration::nbits;
	using BlockType = typename blocksignificantConfiguration::BlockType;
	constexpr unsigned fhbits = (nbits >> 1);
	constexpr unsigned fbits = fhbits - 1;
	constexpr unsigned NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blocksignificant<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificantConfiguration a, b, c;
	a.setradix(fbits);
	b.setradix(fbits);
	a.setradix(2 * fbits);
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	constexpr size_t nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref * bref;
			c.mul(a, b);
			for (size_t k = 0; k < nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}

			if (refResult != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, c, refResult);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
	return nrOfFailedTests;
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

	std::string test_suite  = "blocksignificant multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blocksignificant<8, uint32_t> a, b, c;  // BitEncoding::Ones
		a.setbits(0xF);
		b.setbits(0x9);
		c.mul(a, b);
		blocksignificant<8, uint32_t> result = c; // take the lower nbits // BitEncoding::Ones
		std::cout << to_binary(result) << '\n';
	}

	{
		uint64_t mask;
		//	mask = (1 << (bitsInBlock - ((nbits % (nrBlocks * bitsInBlock)) - 1)))
		int bitsInBlock = 8;
		for (int nbits = 0; nbits < 36; ++nbits) {
			bitsInBlock = 8;
			int nrBlocks = 1 + ((nbits - 1) / bitsInBlock);
			mask = (1ull << ((nbits - 1) % bitsInBlock));
			std::cout << "nbits = " << nbits << " nrBlocks = " << nrBlocks << " mask = 0x" << to_binary(mask) << " " << int(mask) << '\n';
		}
	}
	
	{
		blocksignificant<24, uint32_t> a, b, c, d; // BitEncoding::Ones
		// a = 0x7FF;  worked at one point, must have gone through the default assignment: broke when adding radixPoint
		a.setbits(0x7FFu);  // maxpos
		b.setbits(0x7FFu);  // maxpos
//		c = a * b;  // rounded mul
		d.mul(a, b); // unrounded mul yields
		std::cout << to_hex(a) << " + " << to_hex(b) << " = " << to_hex(c) << " modular, " << to_hex(d) << " unrounded" << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<4, uint8_t> >(reportTestCases), "blocksignificant<4,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<8, uint8_t> >(reportTestCases), "blocksignificant<8,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<8, uint16_t> >(reportTestCases), "blocksignificant<8,uint16>", "multiplication");


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	// NOTE blocksignificant<nbits, ...>   nbits must be even as it represents 2 * fhbits of the multiplier
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<4, uint8_t> >(reportTestCases),  "blocksignificant< 8, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<4, uint16_t> >(reportTestCases), "blocksignificant< 8, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<4, uint32_t> >(reportTestCases), "blocksignificant< 8, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<8, uint8_t> >(reportTestCases),  "blocksignificant< 8, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<8, uint16_t> >(reportTestCases), "blocksignificant< 8, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<8, uint32_t> >(reportTestCases), "blocksignificant< 8, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<10, uint32_t> >(reportTestCases), "blocksignificant<10, uint32>", "multiplication");
#endif

#if REGRESSION_LEVEL_2	 
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<10, uint8_t> >(reportTestCases),  "blocksignificant< 9, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<10, uint16_t> >(reportTestCases), "blocksignificant< 9, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<10, uint32_t> >(reportTestCases), "blocksignificant< 9, uint32>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<12, uint8_t> >(reportTestCases),  "blocksignificant<10, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<12, uint16_t> >(reportTestCases), "blocksignificant<10, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<12, uint32_t> >(reportTestCases), "blocksignificant<10, uint32>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<14, uint8_t> >(reportTestCases),  "blocksignificant<12, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<14, uint16_t> >(reportTestCases), "blocksignificant<12, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantMultiplication< blocksignificant<14, uint32_t> >(reportTestCases), "blocksignificant<12, uint32>", "multiplication");
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
