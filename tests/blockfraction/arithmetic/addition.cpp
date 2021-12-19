// addition.cpp: functional tests for blockfraction addition
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp> // ReportBinaryArithmeticError

// enumerate all addition cases for an blockfraction<nbits,BlockType> configuration
template<typename BlockFractionConfiguration>
int VerifyAddition(bool bReportIndividualTestCases) {
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
			cref = aref + bref;
			c.add(a, b);
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
//		if (i % 1024 == 0) cout << '.'; /// if you enable this, also add the endl line back in
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blockfraction
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename BlockType, sw::universal::BitEncoding encoding>
void GenerateTestCase(const sw::universal::blockfraction<nbits, BlockType, encoding>& lhs, const sw::universal::blockfraction <nbits, BlockType, encoding>& rhs) {
	using namespace sw::universal;

	blockfraction<nbits, BlockType, encoding> a, b, c;

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

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;
		
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blockfraction addition failed: ";

#if MANUAL_TESTING

	{
		blockfraction<8, uint32_t, BitEncoding::Twos> a;
		a.setbits(0x41);
		cout << a << " : " << to_binary(a) << " : " << float(a) << endl;
	}

	blockfraction<23, uint32_t> a, b;

	// generate individual testcases to hand trace/debug
	GenerateTestCase(a, b);


	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),   "blockfraction<  8, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction< 12, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 12, uint16_t>", "addition");

#if STRESS_TESTING

#endif

#else

	std::cout << "blockfraction addition validation\n";
	constexpr BitEncoding twos = BitEncoding::Twos;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<4, uint8_t, twos> >(bReportIndividualTestCases),  "blockfraction< 4, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<4, uint16_t, twos> >(bReportIndividualTestCases), "blockfraction< 4, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<4, uint32_t, twos> >(bReportIndividualTestCases), "blockfraction< 4, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint8_t, twos> >(bReportIndividualTestCases),  "blockfraction< 8, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint16_t, twos> >(bReportIndividualTestCases), "blockfraction< 8, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint32_t, twos> >(bReportIndividualTestCases), "blockfraction< 8, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<9, uint8_t, twos> >(bReportIndividualTestCases),  "blockfraction< 9, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<9, uint16_t, twos> >(bReportIndividualTestCases), "blockfraction< 9, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<9, uint32_t, twos> >(bReportIndividualTestCases), "blockfraction< 9, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<10, uint8_t, twos> >(bReportIndividualTestCases),  "blockfraction<10, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<10, uint16_t, twos> >(bReportIndividualTestCases), "blockfraction<10, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<10, uint32_t, twos> >(bReportIndividualTestCases), "blockfraction<10, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<11, uint8_t, twos> >(bReportIndividualTestCases),  "blockfraction<11, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<11, uint16_t, twos> >(bReportIndividualTestCases), "blockfraction<11, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<11, uint32_t, twos> >(bReportIndividualTestCases), "blockfraction<11, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint8_t, twos> >(bReportIndividualTestCases),  "blockfraction<12, uint8_t >", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint16_t, twos> >(bReportIndividualTestCases), "blockfraction<12, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint32_t, twos> >(bReportIndividualTestCases), "blockfraction<12, uint32_t>", "addition");

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
