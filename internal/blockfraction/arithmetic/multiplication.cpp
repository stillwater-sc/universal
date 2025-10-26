// multiplication.cpp: functional tests for blockfraction multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <bitset>      // not used: just here to access the API

#include <universal/native/integers.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/blockfraction_test_suite.hpp>

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
	using namespace sw::universal::internal;

	std::string test_suite  = "blockfraction multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockfraction<8, uint32_t> a, b, c;  // BitEncoding::Ones
		a.setbits(0xF);
		b.setbits(0x9);
		c.mul(a, b);
		blockfraction<8, uint32_t> result = c; // take the lower nbits // BitEncoding::Ones
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
		blockfraction<24, uint32_t> a, b, c, d; // BitEncoding::Ones
		// a = 0x7FF;  worked at one point, must have gone through the default assignment: broke when adding radixPoint
		a.setbits(0x7FFu);  // maxpos
		b.setbits(0x7FFu);  // maxpos
//		c = a * b;  // rounded mul
		d.mul(a, b); // unrounded mul yields
		std::cout << to_hex(a) << " + " << to_hex(b) << " = " << to_hex(c) << " modular, " << to_hex(d) << " unrounded" << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<4, uint8_t> >(reportTestCases), "blockfraction<4,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<8, uint8_t> >(reportTestCases), "blockfraction<8,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<8, uint16_t> >(reportTestCases), "blockfraction<8,uint16>", "multiplication");


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	// NOTE blockfraction<nbits, ...>   nbits must be even as it represents 2 * fhbits of the multiplier
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<4, uint8_t> >(reportTestCases),  "blockfraction< 8, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<4, uint16_t> >(reportTestCases), "blockfraction< 8, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<4, uint32_t> >(reportTestCases), "blockfraction< 8, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<8, uint8_t> >(reportTestCases),  "blockfraction< 8, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<8, uint16_t> >(reportTestCases), "blockfraction< 8, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<8, uint32_t> >(reportTestCases), "blockfraction< 8, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<10, uint32_t> >(reportTestCases), "blockfraction<10, uint32>", "multiplication");
#endif

#if REGRESSION_LEVEL_2	 
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<10, uint8_t> >(reportTestCases),  "blockfraction< 9, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<10, uint16_t> >(reportTestCases), "blockfraction< 9, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<10, uint32_t> >(reportTestCases), "blockfraction< 9, uint32>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<12, uint8_t> >(reportTestCases),  "blockfraction<10, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<12, uint16_t> >(reportTestCases), "blockfraction<10, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<12, uint32_t> >(reportTestCases), "blockfraction<10, uint32>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<14, uint8_t> >(reportTestCases),  "blockfraction<12, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<14, uint16_t> >(reportTestCases), "blockfraction<12, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockFractionMultiplication< blockfraction<14, uint32_t> >(reportTestCases), "blockfraction<12, uint32>", "multiplication");
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
