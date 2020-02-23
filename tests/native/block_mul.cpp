// arithmetic_mul.cpp: functional tests for block multiplication
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <bitset>      // not used: just here to access the API

// minimum set of include files to reflect source code dependencies
#include <universal/native/blockbinary.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/blockbinary_helpers.hpp"

// enumerate all multiplication cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, typename StorageBlockType = uint8_t>
int VerifyMultiplication(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::unum;

	int nrOfFailedTests = 0;
	blockbinary<nbits, StorageBlockType> a, b, result, refResult;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		aref = i;
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			bref = j;
			cref = aref * bref;

			result = a * b;

			refResult.set_raw_bits(cref);
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in fixpnt.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t _a, int64_t _b) {
	using namespace sw::unum;
	blockbinary<nbits, StorageBlockType> a, b, result, reference;

	a.set_raw_bits(uint64_t(_a));
	b.set_raw_bits(uint64_t(_b));
	result = a + b;

	int64_t ref = _a * _b;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << std::hex;
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << std::dec;
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << " (reference: " << to_binary(int(ref)) << ")   " << std::endl;
	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference.set_raw_bits(ref);
	std::cout << (result == reference ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	std::string tag = "block multiplication: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<8>(12345, 54321);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, uint8_t>("Manual Testing", true), "blockbinary<4,uint8>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, uint8_t>("Manual Testing", true), "blockbinary<8,uint8>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, uint16_t>("Manual Testing", true), "blockbinary<8,uint16>", "multiplication");

	nrOfFailedTestCases = 0;

#if STRESS_TESTING


#endif

#else
	bool bReportIndividualTestCases = false;
	cout << "block multiplication validation" << endl;;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint32>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint8>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint16>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint32>", "multiplication");



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
