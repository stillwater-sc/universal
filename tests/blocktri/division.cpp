// division.cpp: validation tests for block triple number division
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

#ifdef LATER
// minimum set of include files to reflect source code dependencies
#include <universal/blockbinary/blocktriple.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/blocktriple_helpers.hpp"

template<size_t ebits, size_t fbits, typename BlockType = uint8_t>
std::string to_binary(const sw::universal::blocktriple<ebits, fbits, BlockType>& a) {
	std::stringstream ss;
	return ss.str();
}

// enumerate all multiplication cases for a blocktriple<ebits,fbits,BlockType> configuration
template<size_t ebits, size_t fbits, typename BlockType = uint8_t>
int VerifyDivision(const std::string& tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << fbits);
	using namespace std;
	using namespace sw::universal;

	cout << endl;
	cout << "blocktriple<" << ebits << ',' << fbits << ',' << typeid(BlockType).name() << '>' << endl;

	bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blocktriple<ebits,fbits, BlockType> a, b, result, refResult;
	long double aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		aref = a.to_long_double(); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			bref = b.to_long_double(); // cast to long long is reasonable constraint for exhaustive test
			result = a / b;
		
			if (bref == 0) continue;
			cref = aref / bref;

			refResult = cref;
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
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blocktriple
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t ebits, size_t fbits, typename BlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::universal;
	blocktriple<ebits,fbits, BlockType> a, b, result, reference;

	a.set_raw_bits(uint64_t(lhs));
	b.set_raw_bits(uint64_t(rhs));
	result = a / b;

	long double _a, _b, _c;
	_a = (long double)a;
	_b = (long double)b;
	_c = _a / _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(fbits - 2);
	std::cout << std::setw(fbits) << _a << " / " << std::setw(fbits) << _b << " = " << std::setw(fbits) << _c << std::endl;
	std::cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << std::endl;
	//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference =_c;
	std::cout << (result == reference ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple division: ";

#if MANUAL_TESTING

	GenerateTestCase<8,4>(0x8,0x1);  // -8 / 1 => -8

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 8, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 8, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8>", "division");


#if STRESS_TESTING

#endif

#else

	cout << "blocktriple division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 4, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 5, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 5,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 6, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 6,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 7, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 7,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 8, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 8,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 9, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 9,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 10, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 10,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 12, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 12,uint8_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 9, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 9,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 11, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 11,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 13, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 13,uint16_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 12, uint32_t>(tag, bReportIndividualTestCases), "blocktriple<8, 12,uint32_t>", "division");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 16, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 16,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 16, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 16,uint16_t>", "division");


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
#else
int main(int argc, char* argv[]) {
	return 0;
}
#endif