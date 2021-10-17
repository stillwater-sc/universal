// multiplication.cpp: functional tests for blockfraction multiplication
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp> // ReportBinaryArithmeticError

// enumerate all addition cases for an blockfraction<nbits,BlockType> configuration
template<typename BlockFractionConfiguration>
int VerifyMultiplication(bool bReportIndividualTestCases) {
	constexpr size_t nbits = BlockFractionConfiguration::nbits;
	using BlockType = typename BlockFractionConfiguration::BlockType;
	constexpr sw::universal::BitEncoding encoding = BlockFractionConfiguration::encoding;

	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blockfraction<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blockfraction<nbits, BlockType, encoding> a, b, c;
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
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, refResult);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions blockfraction
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {

}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 1) std::cout << argv[0] << std::endl; 
	int nrOfFailedTestCases = 0;

	std::string tag = "block multiplication: ";

#if MANUAL_TESTING

	GenerateTestCase<4>(0x1, 0x9);
	GenerateTestCase<4>(0xF, 0x9);
	GenerateTestCase<4>(0xF, 0x8);

	blockfraction<8, uint32_t, BitEncoding::Ones> a, b, c;
	a.setbits(0xF);
	b.setbits(0x9);
	c.mul(a, b);
	blockfraction<8, uint32_t, BitEncoding::Ones> result = c; // take the lower nbits
	std::cout << to_binary(result) << '\n';

	uint8_t mask;
//	mask = (1 << (bitsInBlock - ((nbits % (nrBlocks * bitsInBlock)) - 1)))
	int bitsInBlock = 8;
	for (int nbits = 0; nbits < 36; ++nbits) {
		bitsInBlock = 8;
		int nrBlocks = 1 + ((nbits - 1) / bitsInBlock);
		mask = (uint8_t(1) << ((nbits-1) % bitsInBlock));
		std::cout << "nbits = " << nbits << " nrBlocks = " << nrBlocks << " mask = 0x" << to_binary(mask) << " " << int(mask) << '\n';
	}

	// generate individual testcases to hand trace/debug
	GenerateTestCase<8>(12345, 54321);
	
	{
		blockfraction<24, uint32_t, BitEncoding::Ones> a, b, c, d;
		// a = 0x7FF;  worked at one point, must have gone through the default assignment: broke when adding radixPoint
		a.setbits(0x7FFu);  // maxpos
		b.setbits(0x7FFu);  // maxpos
//		c = a * b;  // rounded mul
		d.mul(a, b); // unrounded mul yields
		std::cout << to_hex(a) << " + " << to_hex(b) << " = " << to_hex(c) << " modular, " << to_hex(d) << " unrounded" << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<4, uint8_t, BitEncoding::Ones> >(true), "blockfraction<4,uint8>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<8, uint8_t, BitEncoding::Ones> >(true), "blockfraction<8,uint8>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<8, uint16_t, BitEncoding::Ones> >(true), "blockfraction<8,uint16>", "multiplication");

	nrOfFailedTestCases = 0;

#if STRESS_TESTING


#endif

#else
	bool bReportIndividualTestCases = false;
	cout << "block multiplication validation" << endl;;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<4, uint8_t, BitEncoding::Ones> >(bReportIndividualTestCases),  "blockfraction< 8, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<4, uint16_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction< 8, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<4, uint32_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction< 8, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<8, uint8_t, BitEncoding::Ones> >(bReportIndividualTestCases),  "blockfraction< 8, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<8, uint16_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction< 8, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<8, uint32_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction< 8, uint32>", "multiplication");
	 
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<9, uint8_t, BitEncoding::Ones> >(bReportIndividualTestCases),  "blockfraction< 9, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<9, uint16_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction< 9, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<9, uint32_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction< 9, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<10, uint8_t, BitEncoding::Ones> >(bReportIndividualTestCases),  "blockfraction<10, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<10, uint16_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction<10, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<10, uint32_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction<10, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<11, uint8_t, BitEncoding::Ones> >(bReportIndividualTestCases),  "blockfraction<11, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<11, uint16_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction<11, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<11, uint32_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction<11, uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<12, uint8_t, BitEncoding::Ones> >(bReportIndividualTestCases),  "blockfraction<12, uint8 >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<12, uint16_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction<12, uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blockfraction<12, uint32_t, BitEncoding::Ones> >(bReportIndividualTestCases), "blockfraction<12, uint32>", "multiplication");



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
