// subtraction.cpp: functional tests for blockfraction subtraction
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp> // for to_binary(int)
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp> // ReportBinaryArithmeticError

// enumerate all addition cases for an blockfraction configuration
template<typename BlockFractionConfiguration>
int VerifySubtraction(bool bReportIndividualTestCases) {
	constexpr size_t nbits = BlockFractionConfiguration::nbits;
	using BlockType = typename BlockFractionConfiguration::BlockType;

	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace std;
	using namespace sw::universal;

//	cout << endl;
//	cout << "blockfraction<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blockfraction<nbits, BlockType, sw::universal::BitEncoding::Twos> a, b, c;
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref - bref;
			c.sub(a, b);
			for (size_t k = 0; k < blockbinary<nbits, BlockType>::nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}
			if (refResult != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, c, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blockfraction
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename BlockType, sw::universal::BitEncoding encoding>
void GenerateTestCase(const sw::universal::blockfraction<nbits, BlockType, encoding>& lhs, const sw::universal::blockfraction <nbits, BlockType, encoding>& rhs) {
	using namespace sw::universal;

	blockfraction<nbits, BlockType> a, b, c;

	a = lhs;
	b = rhs;
	c.sub(a, b);

	double _a, _b, _c;
	_a = double(a);
	_b = double(b);
	_c = _a - _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << lhs << " - " << std::setw(nbits) << rhs 
		<< " = " << std::setw(nbits) << c << '\n';
	std::cout << std::setw(nbits) << _a << " - " << std::setw(nbits) << _b 
		<< " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " - " << to_binary(b) 
		<< " = " << to_binary(c) << " (reference: " << _c << ")   " << '\n';
	double cref = double(c);
	std::cout << (_c == cref ? "PASS" : "FAIL") << '\n' << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

void GenerateMaxValues() {
	unsigned max = (uint64_t(1) << 8) - 1;
	std::cout << "max = " << max << std::endl;
	max = (uint64_t(1) << 16) - 1;
	std::cout << "max = " << max << std::endl;
	max = (uint64_t(1) << 32) - 1;
	std::cout << "max = " << max << std::endl;
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 1) std::cout << argv[0] << std::endl; 
	
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular subtraction failed: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	{
		blockfraction<8, uint32_t> a, b;
		a.set_raw_bits(0x40);
		b.set_raw_bits(0x41);
		GenerateTestCase(a, b);
	}

	blockfraction<12, uint8_t> a, b;
	a.set_raw_bits(0xfff);
	b = twosComplement(a);
	cout << to_hex(a) << ' ' << to_hex(b) << ' ' << to_hex(twosComplement(b)) << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<4, uint8_t> >(true),  "blockfraction<4, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<4, uint16_t> >(true), "blockfraction<4, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<4, uint32_t> >(true), "blockfraction<4, uint32_t>", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<4, uint64_t> >(true), "blockfraction<4, uint64_t>", "subtraction");

	nrOfFailedTestCases = (bReportIndividualTestCases ? 0 : -1);

#if STRESS_TESTING

#endif

#else

	cout << "block subtraction validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 4, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction< 4, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 4, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 4, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 4, uint32_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 4, uint32_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 4, uint64_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 4, uint64_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 8, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction< 8, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 8, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 8, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 8, uint32_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 8, uint32_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 9, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction< 9, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 9, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 9, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction< 9, uint32_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction< 9, uint32_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<10, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction<10, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<10, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction<10, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<10, uint32_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction<10, uint32_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<11, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction<11, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<11, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction<11, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<11, uint32_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction<11, uint32_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<12, uint8_t, BitEncoding::Twos> >(bReportIndividualTestCases),  "blockfraction<12, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<12, uint16_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction<12, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< blockfraction<12, uint32_t, BitEncoding::Twos> >(bReportIndividualTestCases), "blockfraction<12, uint32_t>", "subtraction");

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
