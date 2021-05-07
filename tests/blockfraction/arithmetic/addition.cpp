// addition.cpp: functional tests for blockfraction addition
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/native/ieee754.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
// #include <universal/verification/test_reporters.hpp>

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

// enumerate all addition cases for an blockfraction<nbits,BlockType> configuration
template<typename BlockFractionConfiguration>
int VerifyAddition(bool bReportIndividualTestCases) {
	constexpr size_t fhbits = BlockFractionConfiguration::fhbits;  // includes hidden bit
	constexpr size_t abits = BlockFractionConfiguration::abits;
	using BlockType = typename BlockFractionConfiguration::BlockType;

	constexpr size_t NR_VALUES = (size_t(1) << fhbits);
	using namespace std;
	using namespace sw::universal;
	
	cout << endl;
	cout << "blockfraction<" <<fhbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blockfraction<abits> a, b;
	blockfraction<abits+1> result, refResult;
	double aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		aref = double(a); // cast to double is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			bref = double(b); // cast to double is reasonable constraint for exhaustive test
			cref = aref + bref;
			module_add(a, b, result);
			refResult = cref;

			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blockfraction
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename BlockTriple, typename BlockType = uint8_t>
void GenerateTestCase(const BlockTriple& lhs, const BlockTriple& rhs) {
	using namespace sw::universal;
	constexpr size_t fhbits = BlockTriple::fhbits;
	blockfraction<fhbits, BlockType> a, b;
	blockfraction<fhbits+1, BlockType> result, reference;

	a = lhs;
	b = rhs;
	uradd(result, a, b);

	double _a, _b, _c;
	_a = double(a);
	_b = double(b);
	_c = _a + _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(fhbits - 2);
	std::cout << std::setw(fhbits) << lhs << " + " << std::setw(fhbits) << rhs << " = " << std::setw(fhbits) << lhs + rhs << '\n';
	std::cout << std::setw(fhbits) << _a << " + " << std::setw(fhbits) << _b << " = " << std::setw(fhbits) << _c << '\n';
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
	
//	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "blockfraction addition failed: ";

#if MANUAL_TESTING

	using Real = blocktriple<23>;
	Real triple = 1.0f;
	cout << triple << endl;

	blockfraction<23, uint32_t> a, b;

	// generate individual testcases to hand trace/debug
//	GenerateTestCase<Real>(triple, triple);
//	GenerateTestCase<Real>(1.5, 1.5);


//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint8_t> >(bReportIndividualTestCases), "blockfraction<8, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint8_t> >(bReportIndividualTestCases), "blockfraction<12, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint16_t> >(bReportIndividualTestCases), "blockfraction<12, uint16_t>", "addition");

#if STRESS_TESTING

#endif

#else

	cout << "blockfraction addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<4, uint8_t> >(bReportIndividualTestCases), "blockfraction<4,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<4, uint16_t> >(bReportIndividualTestCases), "blockfraction<4,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<4, uint32_t> >(bReportIndividualTestCases), "blockfraction<4,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint8_t> >(bReportIndividualTestCases), "blockfraction<8,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint16_t> >(bReportIndividualTestCases), "blockfraction<8,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<8, uint32_t> >(bReportIndividualTestCases), "blockfraction<8,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<9, uint8_t> >(bReportIndividualTestCases), "blockfraction<9,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<9, uint16_t> >(bReportIndividualTestCases), "blockfraction<9,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<9, uint32_t> >(bReportIndividualTestCases), "blockfraction<9,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<10, uint8_t> >(bReportIndividualTestCases), "blockfraction<10,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<10, uint16_t> >(bReportIndividualTestCases), "blockfraction<10,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<10, uint32_t> >(bReportIndividualTestCases), "blockfraction<10,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<11, uint8_t> >(bReportIndividualTestCases), "blockfraction<11,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<11, uint16_t> >(bReportIndividualTestCases), "blockfraction<11,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<11, uint32_t> >(bReportIndividualTestCases), "blockfraction<11,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint8_t> >(bReportIndividualTestCases), "blockfraction<12,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint16_t> >(bReportIndividualTestCases), "blockfraction<12,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blockfraction<12, uint32_t> >(bReportIndividualTestCases), "blockfraction<12,uint32_t>", "addition");

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
