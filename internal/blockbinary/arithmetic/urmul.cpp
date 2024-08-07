// urmul.cpp: functional tests for unrounded block binary multiplication
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/blockbinary_test_status.hpp>

// enumerate all multiplication cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyUnroundedMultiplication(bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	std::cout << "unrounded multiplication for blockbinary<" << nbits << ',' << typeid(BlockType).name() << ">\n";

	bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockbinary<nbits, BlockType> a, b;
	blockbinary<2*nbits, BlockType> signext_a, signext_b, signext_result, result_reference;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		signext_a = a;
		aref = int64_t(signext_a.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			signext_b = b;
			bref = int64_t(signext_b.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
			signext_result = signext_a * signext_b;
			cref = aref * bref;

			if (bReportOverflowCondition) std::cout << std::setw(5) << aref << " * " << std::setw(5) << bref << " = " << std::setw(5) << cref << " : ";
			if (cref < -(1 << (nbits - 1))) {
				if (bReportOverflowCondition) std::cout << "underflow: " << std::setw(5) << cref << " < " << std::setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << std::setw(5) << signext_result.to_long_long() << " " << std::setw(5) << to_hex(signext_result) << " vs " << to_binary(cref, false, 12) << '\n';
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if (bReportOverflowCondition) std::cout << "overflow: " << std::setw(5) << cref << " > " << std::setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << std::setw(5) << signext_result.to_long_long() << " " << std::setw(5) << to_hex(signext_result) << " vs " << to_binary(cref, false, 12) << '\n';
				++nrOfOverflows;
			}
			else {
				if (bReportOverflowCondition) std::cout << '\n';
			}

			result_reference.setbits(static_cast<uint64_t>(cref)); // in 2*nbits representation
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
	std::cout << "Total State Space: " << std::setw(10) << NR_VALUES * NR_VALUES 
		<< " Overflows: " << std::setw(10) << nrOfOverflows << " Underflows " << std::setw(10) << nrOfUnderflows << '\n';
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in fixpnt.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::universal;
	blockbinary<nbits, StorageBlockType> a, b, result, reference;

	a.setbits(uint64_t(lhs));
	b.setbits(uint64_t(rhs));
	long long _a, _b, _c;
	_a = (long long)a;
	_b = (long long)b;
	_c = _a * _b;
	result = a * b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << std::endl;
	reference.setbits(_c);
	std::cout << (result == reference ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;
	
	std::string test_suite = "unrounded blockbinary multiplication";
	std::string test_tag = "unrounded multiplication";
	std::cout << test_suite << '\n';
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	blockbinary<4> a, b;
	a = -8;
	b = -8;
	blockbinary<8> c = urmul(a, b);
	cout << (long long)a << " * " << (long long)b << " = " << (long long)c << " : " << to_binary(c) << " <--- demonstration that 2*nbits is sufficient to represent all results\n";

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<4, uint8_t>(true), "blockbinary<4,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint8_t>(true), "blockbinary<8,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<8, uint16_t>(true), "blockbinary<8,uint16>", test_tag);

	nrOfFailedTestCases = 0;

#if STRESS_TESTING


#endif

#else

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 4, uint8_t >(bReportIndividualTestCases), "blockbinary< 8,uint8 >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 4, uint16_t>(bReportIndividualTestCases), "blockbinary< 8,uint16>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 4, uint32_t>(bReportIndividualTestCases), "blockbinary< 8,uint32>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 8, uint8_t >(bReportIndividualTestCases), "blockbinary< 8,uint8 >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 8, uint16_t>(bReportIndividualTestCases), "blockbinary< 8,uint16>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 8, uint32_t>(bReportIndividualTestCases), "blockbinary< 8,uint32>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 9, uint8_t >(bReportIndividualTestCases), "blockbinary< 9,uint8 >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 9, uint16_t>(bReportIndividualTestCases), "blockbinary< 9,uint16>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication< 9, uint32_t>(bReportIndividualTestCases), "blockbinary< 9,uint32>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<10, uint8_t >(bReportIndividualTestCases), "blockbinary<10,uint8 >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<10, uint16_t>(bReportIndividualTestCases), "blockbinary<10,uint16>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<10, uint32_t>(bReportIndividualTestCases), "blockbinary<10,uint32>", test_tag);


#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<11, uint8_t>(bReportIndividualTestCases), "blockbinary<11,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<11, uint16_t>(bReportIndividualTestCases), "blockbinary<11,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<11, uint32_t>(bReportIndividualTestCases), "blockbinary<11,uint32>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<12, uint16_t>(bReportIndividualTestCases), "blockbinary<12,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyUnroundedMultiplication<12, uint32_t>(bReportIndividualTestCases), "blockbinary<12,uint32>", test_tag);

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
