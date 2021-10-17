// division.cpp: functional tests for blockfraction division
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>

#include <universal/native/integers.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp> // ReportBinaryArithmeticError

// enumerate all multiplication cases for an blockfraction<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyDivision(bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	/*
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	cout << endl;
	cout << "blockfraction<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	bool bReportOverflowCondition = false;

	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockfraction<nbits, BlockType> a, b, result, refResult;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		aref = int64_t(a.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			bref = int64_t(b.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
//			result = a / b;
		
			if (bref == 0) continue;
			cref = aref / bref;

			if (cref < -(1 << (nbits - 1))) {
				if (bReportOverflowCondition) cout << setw(5) << aref << " / " << setw(5) << bref << " = " << setw(5) << cref << " : ";
				if (bReportOverflowCondition) cout << "underflow: " << setw(5) << cref << " < " << setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << setw(5) << result.to_long_long() << " " << setw(5) << to_hex(result) << " vs " << to_binary(cref, 12) << endl;
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if (bReportOverflowCondition) cout << setw(5) << aref << " / " << setw(5) << bref << " = " << setw(5) << cref << " : ";
				if (bReportOverflowCondition) cout << "overflow: " << setw(5) << cref << " > " << setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << setw(5) << result.to_long_long() << " " << setw(5) << to_hex(result) << " vs " << to_binary(cref, 12) << endl;
				++nrOfOverflows;
			}

			refResult.set_raw_bits(static_cast<uint64_t>(cref));
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, result, cref);
			}
			if (nrOfFailedTests > 24) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) std::cout << '.';
	}
	cout << "Total State Space: " << setw(10) << NR_VALUES * NR_VALUES << " Overflows: " << setw(10) << nrOfOverflows << " Underflows " << setw(10) << nrOfUnderflows << endl;
	*/
	return nrOfFailedTests;
}

template<size_t nbits, typename BlockType, sw::universal::BitEncoding encoding>
void TestMostSignificantBit() {
	using namespace sw::universal;
	blockfraction<nbits, BlockType, encoding> a;
	std::cout << to_binary(a) << ' ' << a.msb() << '\n';
	a.setbits(0x01ull);
	for (size_t i = 0; i < nbits; ++i) {
		std::cout << to_binary(a) << ' ' << a.msb() << '\n';
		a <<= 1;
	}
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 1) std::cout << argv[0] << std::endl; 
	
	int nrOfFailedTestCases = 0;

	std::string tag = "blockfraction division: ";

#if MANUAL_TESTING

	TestMostSignificantBit<27, uint8_t, BitEncoding::Ones>();
	TestMostSignificantBit<27, uint16_t, BitEncoding::Twos>();
	TestMostSignificantBit<33, uint32_t, BitEncoding::Twos>();

//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, uint8_t>(bReportIndividualTestCases), "blockfraction<4>", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, uint8_t>(bReportIndividualTestCases), "blockfraction<8>", "division");


#if STRESS_TESTING

#endif

#else
	bool bReportIndividualTestCases = true;

	cout << "blockfraction division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, uint8_t>(bReportIndividualTestCases), "blockfraction<4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<5, uint8_t>(bReportIndividualTestCases), "blockfraction<5,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6, uint8_t>(bReportIndividualTestCases), "blockfraction<6,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<7, uint8_t>(bReportIndividualTestCases), "blockfraction<7,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, uint8_t>(bReportIndividualTestCases), "blockfraction<8,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<9, uint8_t>(bReportIndividualTestCases), "blockfraction<9,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10, uint8_t>(bReportIndividualTestCases), "blockfraction<10,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<12, uint8_t>(bReportIndividualTestCases), "blockfraction<12,uint8_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<9, uint16_t>(bReportIndividualTestCases), "blockfraction<9,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<11, uint16_t>(bReportIndividualTestCases), "blockfraction<11,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<13, uint16_t>(bReportIndividualTestCases), "blockfraction<13,uint16_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<12, uint32_t>(bReportIndividualTestCases), "blockfraction<12,uint32_t>", "division");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<16, uint8_t>(bReportIndividualTestCases), "blockfraction<16,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<16, uint16_t>(bReportIndividualTestCases), "blockfraction<16,uint16_t>", "division");


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
