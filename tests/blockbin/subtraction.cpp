// subtraction.cpp: functional tests for block binary subtraction
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp> // for to_binary(int)
#include <universal/blockbin/blockbinary.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/blockbinary_helpers.hpp"


// enumerate all addition cases for an blockbinary configuration
template<size_t nbits, typename StorageBlockType = uint8_t>
int VerifySubtraction(std::string tag, bool bReportIndividualTestCases) {
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
			cref = aref - bref;

			result = a - b;

			refResult.set_raw_bits(cref);
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

// generate specific test case that you can trace with the trace conditions in blockbinary
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::unum;
	blockbinary<nbits, StorageBlockType> a, b, result, reference;

	a.set_raw_bits(uint64_t(lhs));
	b.set_raw_bits(uint64_t(rhs));
	result = a - b;

	long long _a, _b, _c;
	_a = (long long)a;
	_b = (long long)b;
	_c = _a - _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " / " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
	std::cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << std::endl;
	//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference.set_raw_bits(_c);
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
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular subtraction failed: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<12, uint8_t>(0, 1); 

	blockbinary<12, uint8_t> a, b;
	a = 0xfff;
	b = twosComplement(a);
	cout << to_hex(a) << ' ' << to_hex(b) << ' ' << to_hex(twosComplement(b)) << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint8_t>("Manual Testing", true), "uint8_t<4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint16_t>("Manual Testing", true), "uint16_t<4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint32_t>("Manual Testing", true), "uint32_t<4>", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, uint64_t>("Manual Testing", true), "uint64_t<4>", "subtraction");

	nrOfFailedTestCases = (bReportIndividualTestCases ? 0 : -1);

#if STRESS_TESTING

#endif

#else

	cout << "block subtraction validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint8_t>(tag, bReportIndividualTestCases), "uint8_t<8>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint16_t>(tag, bReportIndividualTestCases), "uint16_t<8>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, uint32_t>(tag, bReportIndividualTestCases), "uint32_t<8>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint8_t>(tag, bReportIndividualTestCases), "uint8_t<12>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint16_t>(tag, bReportIndividualTestCases), "uint16_t<12>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, uint32_t>(tag, bReportIndividualTestCases), "uint32_t<12>", "subtraction");

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
