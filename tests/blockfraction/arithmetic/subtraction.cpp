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
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp> // ReportBinaryArithmeticError

template<typename TestType, typename ResultType, typename RefType>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const TestType& lhs, const TestType& rhs, const ResultType& result, const RefType& ref) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
//		<< " " << to_binary(result) << " vs " << to_binary(ref)
		<< std::setprecision(old_precision)
		<< std::endl;
}

// enumerate all addition cases for an blockfraction configuration
template<size_t nbits, typename StorageBlockType = uint8_t>
int VerifySubtraction(bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	int nrOfFailedTests = 0;
	blockfraction<nbits, StorageBlockType> a, b;
	blockfraction<nbits+1, StorageBlockType> result, refResult;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		aref = static_cast<int64_t>(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			bref = static_cast<int64_t>(j);
			cref = aref - bref;

			result.sub(a, b);

			refResult.set_raw_bits(static_cast<uint64_t>(cref));
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blockfraction
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::universal;
	blockfraction<nbits, StorageBlockType> a, b;
	blockfraction<nbits+1, StorageBlockType> result, reference;

	a.set_raw_bits(uint64_t(lhs));
	b.set_raw_bits(uint64_t(rhs));
	result.sub(a, b);

	double _a, _b, _c;
	_a = double(a);
	_b = double(b);
	_c = _a - _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " / " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
	std::cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << std::endl;
	//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference.set_raw_bits(static_cast<uint64_t>(_c));
	std::cout << (result == reference ? "PASS" : "FAIL") << std::endl << std::endl;
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
#define MANUAL_TESTING 1
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
	GenerateTestCase<12, uint8_t>(0, 1); 

	blockfraction<12, uint8_t> a, b;
	a.set_raw_bits(0xfff);
	b = twosComplement(a);
	cout << to_hex(a) << ' ' << to_hex(b) << ' ' << to_hex(twosComplement(b)) << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint8_t>(true), "uint8_t<4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint16_t>(true), "uint16_t<4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint32_t>(true), "uint32_t<4>", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint64_t>(true), "uint64_t<4>", "subtraction");

	nrOfFailedTestCases = (bReportIndividualTestCases ? 0 : -1);

#if STRESS_TESTING

#endif

#else

	cout << "block subtraction validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint8_t>(bReportIndividualTestCases), "uint8_t<8>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint16_t>(bReportIndividualTestCases), "uint16_t<8>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint32_t>(bReportIndividualTestCases), "uint32_t<8>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint8_t>(bReportIndividualTestCases), "uint8_t<12>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint16_t>(bReportIndividualTestCases), "uint16_t<12>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint32_t>(bReportIndividualTestCases), "uint32_t<12>", "subtraction");

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
