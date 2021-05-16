// addition.cpp: functional tests for blocktriple number addition
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// temporary
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/bitblock/bitblock.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/native/ieee754.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>

#ifdef DEPRECATED
#define NUMBER_COLUMN_WIDTH 20

template<typename InputType, typename ResultType, typename RefType>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const InputType& lhs, const InputType& rhs, const ResultType& result, const RefType& ref) {
	using namespace sw::universal;
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << to_binary(result) << " vs " << to_binary(ref, true)
		<< std::setprecision(old_precision)
		<< std::endl;
}
#endif

// enumerate all addition cases for an blocktriple<nbits,BlockType> configuration
template<typename BlockTripleConfiguration>
int VerifyAddition(bool bReportIndividualTestCases) {
	constexpr size_t fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	constexpr size_t abits = BlockTripleConfiguration::abits;
	using BlockType = typename BlockTripleConfiguration::BlockType;

	constexpr size_t NR_VALUES = (size_t(1) << fbits);
	using namespace std;
	using namespace sw::universal;
	
	cout << endl;
	cout << "blocktriple<" <<fbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocktriple<abits> a, b;
	blocktriple<abits+1> c, refResult;
	a.setnormal();
	b.setnormal();
	double aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setBits(i + NR_VALUES);  // the + NR_VALUES is to set the hidden bit in the blockfraction
		aref = double(a); // cast to double is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setBits(j + NR_VALUES);
			bref = double(b); // cast to double is reasonable constraint for exhaustive test
			cref = aref + bref;
			c.add(a, b);
			refResult = cref;

			if (c != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blocktriple
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename ArgumentType>
void GenerateTestCase(ArgumentType lhs, ArgumentType rhs) {
	using namespace sw::universal;
	blocktriple<nbits> a, b;
	blocktriple<nbits+1> result, reference;

	// convert to blocktriple
	a = lhs;
	b = rhs;
	result.add(a, b);

	// convert blocktriples back to argument type
	ArgumentType _a, _b, _c;
	_a = ArgumentType(a);
	_b = ArgumentType(b);
	_c = _a + _b;

	// check that the round-trip through the blocktriple yields the same value as direct conversion
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << lhs << " + " << std::setw(nbits) << rhs << " = " << std::setw(nbits) << lhs + rhs << '\n';
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << '\n';
	reference = _c;
	std::cout << (result == reference ? "PASS" : "FAIL") << '\n' << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);
	
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular addition failed: ";

#if MANUAL_TESTING

	internal::bitblock<23> bb;
	bb = internal::convert_to_bitblock<23>(-1);
	cout << bb << endl;

	// generate individual testcases to hand trace/debug
	GenerateTestCase<18, float>(12345.0, 54321.0); // result is 66,666, and needs 18 bits to be represented by 2's complement
	GenerateTestCase<18, float>(66666, -54321); // result is 12,345

	blocktriple<18> aa = 12345.0f;
	cout << "aa : " << aa << endl;
	aa = 12345;
	cout << "aa : " << aa << endl;

	blocktriple<18> a, b;
	blocktriple<19> c;
	a =  66666.0f;
	b = -66666.0f;
	c.add(a, b);
	a.sign();
	cout << (a.sign() ? "neg" : "pos") << endl;
	cout << (c.sign() ? "neg" : "pos") << endl;
	cout << a << endl;	
	cout << c << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple< 8, uint8_t> >(bReportIndividualTestCases), "blocktriple<8, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, uint8_t> >(bReportIndividualTestCases), "blocktriple<12, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, uint16_t> >(bReportIndividualTestCases), "blocktriple<12, uint16_t>", "addition");

#if STRESS_TESTING

#endif

#else

	cout << "block addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<4, uint8_t> >(bReportIndividualTestCases), "blocktriple<4,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<4, uint16_t> >(bReportIndividualTestCases), "blocktriple<4,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<4, uint32_t> >(bReportIndividualTestCases), "blocktriple<4,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<8, uint8_t> >(bReportIndividualTestCases), "blocktriple<8,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<8, uint16_t> >(bReportIndividualTestCases), "blocktriple<8,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<8, uint32_t> >(bReportIndividualTestCases), "blocktriple<8,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<9, uint8_t> >(bReportIndividualTestCases), "blocktriple<9,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<9, uint16_t> >(bReportIndividualTestCases), "blocktriple<9,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<9, uint32_t> >(bReportIndividualTestCases), "blocktriple<9,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, uint8_t> >(bReportIndividualTestCases), "blocktriple<10,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, uint16_t> >(bReportIndividualTestCases), "blocktriple<10,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, uint32_t> >(bReportIndividualTestCases), "blocktriple<10,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, uint8_t> >(bReportIndividualTestCases), "blocktriple<11,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, uint16_t> >(bReportIndividualTestCases), "blocktriple<11,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, uint32_t> >(bReportIndividualTestCases), "blocktriple<11,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, uint8_t> >(bReportIndividualTestCases), "blocktriple<12,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, uint16_t> >(bReportIndividualTestCases), "blocktriple<12,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, uint32_t> >(bReportIndividualTestCases), "blocktriple<12,uint32_t>", "addition");

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
