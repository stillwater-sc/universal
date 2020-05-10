// urmul.cpp: functional tests for unrounded block binary multiplication
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

// enumerate all multiplication cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyUnroundedMultiplication(const std::string& tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace std;
	using namespace sw::unum;

	cout << endl;
	cout << "unrounded multiplication for blockbinary<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockbinary<nbits, BlockType> a, b;
	blockbinary<2*nbits, BlockType> signext_a, signext_b, signext_result, result_reference;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		signext_a = a;
		aref = int64_t(signext_a.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			signext_b = b;
			bref = int64_t(signext_b.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
			signext_result = signext_a * signext_b;
			cref = aref * bref;

			if (bReportOverflowCondition) cout << setw(5) << aref << " * " << setw(5) << bref << " = " << setw(5) << cref << " : ";
			if (cref < -(1 << (nbits - 1))) {
				if (bReportOverflowCondition) cout << "underflow: " << setw(5) << cref << " < " << setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << setw(5) << signext_result.to_long_long() << " " << setw(5) << to_hex(signext_result) << " vs " << to_binary(cref, 12) << endl;
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if (bReportOverflowCondition) cout << "overflow: " << setw(5) << cref << " > " << setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << setw(5) << signext_result.to_long_long() << " " << setw(5) << to_hex(signext_result) << " vs " << to_binary(cref, 12) << endl;
				++nrOfOverflows;
			}
			else {
				if (bReportOverflowCondition)cout << endl;
			}

			result_reference.set_raw_bits(cref); // in 2*nbits representation
			if (signext_result != result_reference) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", signext_a, signext_b, signext_result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", signext_a, signext_b, signext_result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) std::cout << '.';
	}
	cout << "Total State Space: " << setw(10) << NR_VALUES * NR_VALUES << " Overflows: " << setw(10) << nrOfOverflows << " Underflows " << setw(10) << nrOfUnderflows << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in fixpnt.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::unum;
	blockbinary<nbits, StorageBlockType> a, b, result, reference;

	a.set_raw_bits(uint64_t(lhs));
	b.set_raw_bits(uint64_t(rhs));
	long long _a, _b, _c;
	_a = (long long)a;
	_b = (long long)b;
	result = a * b;
	_c = (long long)result;


	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _a * _b << std::endl;
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _a * _b << ")   " << std::endl;
//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference.set_raw_bits(_a * _b);
	std::cout << (result == reference ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	std::string tag = "unrounded block multiplication: ";

#if MANUAL_TESTING

	blockbinary<4> a, b;
	a = -8;
	b = -8;
	blockbinary<8> c = urmul(a, b);
	cout << (long long)a << " * " << (long long)b << " = " << (long long)c << " : " << to_binary(c) << " <--- demonstration that 2*nbits is sufficient to represent all results\n";

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<4, uint8_t>("Manual Testing", true), "blockbinary<4,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint8_t>("Manual Testing", true), "blockbinary<8,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint16_t>("Manual Testing", true), "blockbinary<8,uint16>", "multiplication");

	nrOfFailedTestCases = 0;

#if STRESS_TESTING


#endif

#else
	bool bReportIndividualTestCases = false;
	cout << "unrounded block multiplication validation" << endl;;

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<4, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<4, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<4, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<9, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<9,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<9, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<9,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<9, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<9,uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<10, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<10,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<10, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<10,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<10, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<10,uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<11, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<11,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<11, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<11,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<11, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<11,uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<12, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<12, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<12, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint32>", "multiplication");



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
