// remainder.cpp: functional tests for block binary number remainder
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
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/blockbinary_test_status.hpp>

// enumerate all multiplication cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyRemainder(bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	std::cout << "blockbinary<" << nbits << ',' << typeid(BlockType).name() << ">\n";

	bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockbinary<nbits, BlockType> a, b, result, refResult;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref = int64_t(a.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref = int64_t(b.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
			result = a % b;
		
			if (bref == 0) continue;
			cref = aref % bref;

			if (cref < -(1 << (nbits - 1))) {
				if (bReportOverflowCondition) std::cout << std::setw(5) << aref << " % " << std::setw(5) << bref << " = " << std::setw(5) << cref << " : ";
				if (bReportOverflowCondition) std::cout << "underflow: " << std::setw(5) << cref << " < " << std::setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << std::setw(5) << result.to_long_long() << " " << std::setw(5) << to_hex(result) << " vs " << to_binary(cref, false, 12) << '\n';
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if (bReportOverflowCondition) std::cout << std::setw(5) << aref << " % " << std::setw(5) << bref << " = " << std::setw(5) << cref << " : ";
				if (bReportOverflowCondition) std::cout << "overflow: " << std::setw(5) << cref << " > " << std::setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << std::setw(5) << result.to_long_long() << " " << std::setw(5) << to_hex(result) << " vs " << to_binary(cref, false, 12) << '\n';
				++nrOfOverflows;
			}

			refResult.setbits(static_cast<uint64_t>(cref));
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "%", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "%", a, b, result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << "Total State Space: " << std::setw(10) << NR_VALUES * NR_VALUES 
		<< " Overflows: " << std::setw(10) << nrOfOverflows << " Underflows " << std::setw(10) << nrOfUnderflows << '\n';
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blockbinary
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::universal;
	blockbinary<nbits, StorageBlockType> a, b, result, reference;

	a.setbits(uint64_t(lhs));
	b.setbits(uint64_t(rhs));
	result = a % b;

	long long _a, _b, _c;
	_a = (long long)a;
	_b = (long long)b;
	_c = _a % _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " % " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
	std::cout << to_binary(a) << " % " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << std::endl;
	//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
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

	std::string test_suite = "blockbinary remainder";
	std::string test_tag = "remainder";
	std::cout << test_suite << '\n';
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	GenerateTestCase<4>(0x8, 0x1); // -8 % 1 = 0

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<4, uint8_t>(bReportIndividualTestCases), "blockbinary<4>", test_tag);


#if STRESS_TESTING

#endif

#else

	std::cout << "blockbinary remainder validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 4, uint8_t>(bReportIndividualTestCases), "blockbinary< 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 5, uint8_t>(bReportIndividualTestCases), "blockbinary< 5,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 6, uint8_t>(bReportIndividualTestCases), "blockbinary< 6,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 7, uint8_t>(bReportIndividualTestCases), "blockbinary< 7,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 8, uint8_t>(bReportIndividualTestCases), "blockbinary< 8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 9, uint8_t>(bReportIndividualTestCases), "blockbinary< 9,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<10, uint8_t>(bReportIndividualTestCases), "blockbinary<10,uint8_t>", test_tag);

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint8_t >(bReportIndividualTestCases), "blockbinary<12,uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< 9, uint16_t>(bReportIndividualTestCases), "blockbinary< 9,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<11, uint16_t>(bReportIndividualTestCases), "blockbinary<11,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<13, uint16_t>(bReportIndividualTestCases), "blockbinary<13,uint16_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint32_t>(bReportIndividualTestCases), "blockbinary<12,uint32_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<16, uint8_t >(bReportIndividualTestCases), "blockbinary<16,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", test_tag);


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
