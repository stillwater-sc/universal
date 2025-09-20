// division.cpp: functional tests for block binary number division
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/blockbinary_test_status.hpp>

// enumerate all division cases for a blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyDivision(bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	std::cout << "blockbinary<" << nbits << ',' << typeid(BlockType).name() << ">\n";

	constexpr bool bReportUnderflowCondition = false;
	constexpr bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockbinary<nbits, BlockType> a, b, result, refResult;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref = int64_t(a.to_sll()); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref = int64_t(b.to_sll()); // cast to long long is reasonable constraint for exhaustive test
			result = a / b;
		
			if (bref == 0) continue;
			cref = aref / bref;

			if (cref < -(1 << (nbits - 1))) {
				if constexpr (bReportUnderflowCondition) {
					std::cout << std::setw(5) << aref << " / " << std::setw(5) << bref << " = " << std::setw(5) << cref << " : ";
					std::cout << "underflow: " << std::setw(5) << cref << " < " << std::setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << std::setw(5) << result.to_sll() << " " << std::setw(5) << to_hex(result) << " vs " << to_binary(cref, 12) << '\n';
				}
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if constexpr (bReportOverflowCondition) {
					std::cout << std::setw(5) << aref << " / " << std::setw(5) << bref << " = " << std::setw(5) << cref << " : ";
					std::cout << "overflow: " << std::setw(5) << cref << " > " << std::setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << std::setw(5) << result.to_sll() << " " << std::setw(5) << to_hex(result) << " vs " << to_binary(cref, 12) << '\n';
				}
				++nrOfOverflows;
			}

			refResult.setbits(static_cast<uint64_t>(cref));
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", a, b, result, cref);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, result, cref);
			}
			if (nrOfFailedTests > 24) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << "Total State Space: " << std::setw(10) << NR_VALUES * NR_VALUES 
		<< " Overflows: " << std::setw(10) << nrOfOverflows << " Underflows " << std::setw(10) << nrOfUnderflows << '\n';
	return nrOfFailedTests;
}

template<size_t nbits, typename BlockType = uint8_t>
void TestMostSignificantBit() {
	using namespace sw::universal;
	blockbinary<nbits, BlockType> a;
	std::cout << to_binary(a) << ' ' << a.msb() << '\n';
	a = 1;
	for (size_t i = 0; i < nbits; ++i) {
		std::cout << to_binary(a) << ' ' << a.msb() << '\n';
		a <<= 1;
	}
}

// generate specific test case that you can trace with the trace conditions in blockbinary
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename BlockType = uint8_t>
void TestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::universal;
	blockbinary<nbits, BlockType> a, b, result, reference;

	a.setbits(uint64_t(lhs));
	b.setbits(uint64_t(rhs));
	result = a / b;

	long long _a, _b, _c;
	_a = (long long)a;
	_b = (long long)b;
	_c = _a / _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " / " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << '\n';
	//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference.setbits(static_cast<size_t>(_c));
	std::cout << (result == reference ? "PASS" : "FAIL") << "\n\n";
	std::cout << std::dec << std::setprecision(oldPrecision);
}

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

	std::string test_suite = "blockbinary division validation";	
	std::string test_tag = "blockbinary division: ";
	std::cout << test_suite << '\n';
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;


#if MANUAL_TESTING

//	TestMostSignificantBit<27, uint8_t>();
//	TestMostSignificantBit<27, uint16_t>();
//	TestMostSignificantBit<33, uint32_t>();

	TestCase<4>(0x1,0x8);  // 1 / -8 => 0

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, uint8_t>(bReportIndividualTestCases), "blockbinary<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 4, uint8_t>(bReportIndividualTestCases), "blockbinary< 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 5, uint8_t>(bReportIndividualTestCases), "blockbinary< 5,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 6, uint8_t>(bReportIndividualTestCases), "blockbinary< 6,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 7, uint8_t>(bReportIndividualTestCases), "blockbinary< 7,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, uint8_t>(bReportIndividualTestCases), "blockbinary< 8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 9, uint8_t>(bReportIndividualTestCases), "blockbinary< 9,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10, uint8_t>(bReportIndividualTestCases), "blockbinary<10,uint8_t>", test_tag);

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<12, uint8_t >(bReportIndividualTestCases), "blockbinary<12,uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 9, uint16_t>(bReportIndividualTestCases), "blockbinary<9,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<11, uint16_t>(bReportIndividualTestCases), "blockbinary<11,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<13, uint16_t>(bReportIndividualTestCases), "blockbinary<13,uint16_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<12, uint32_t>(bReportIndividualTestCases), "blockbinary<12,uint32_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<16, uint8_t >(bReportIndividualTestCases), "blockbinary<16,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", test_tag);
#endif  // REGRESSION_LEVEL_4

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
