// multiplication.cpp: functional tests for blocktriple number multiplication
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>

// temporary
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/bitblock/bitblock.hpp>
// minimum set of include files to reflect source code dependencies                    
#include <universal/native/ieee754.hpp>
// uncomment to enable operator tracing
#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_MUL
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>

// enumerate all multiplication cases for an blocktriple<nbits,BlockType> configuration
template<typename BlockTripleConfiguration>
int VerifyMultiplication(bool bReportIndividualTestCases) {
	constexpr size_t fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	constexpr size_t mbits = BlockTripleConfiguration::mbits;
	using BlockType = typename BlockTripleConfiguration::BlockType;
	constexpr sw::universal::BlockTripleOperator op = BlockTripleConfiguration::op;

	// for the test we are going to enumerate the fbits state space
	constexpr size_t NR_VALUES = (size_t(1) << fbits);

	using namespace std;
	using namespace sw::universal;
	
	cout << endl;
	cout << "blocktriple<" <<fbits << ',' << op << ',' << typeid(BlockType).name() << '>' << endl;
	cout << "Fraction        bits : " << fbits << endl;
	cout << "Multiplication  bits : " << mbits << endl;

	/*
		blocktriple<fbits> has fbits fraction bits in the form h.<fbits>

		example: blocktriple<3> represents values 
		1.000
		1.001
		1.010
		...
		1.101
		1.110
		1.111

		The scale shifts these value relative to 1. So a scale of -3 shifts these bits to the right, a scale of +3 shifts them to the left

		when multiplying, we generate 2*fhbits result bits with radix at 2*fbits
		which we'll need to round using round-nearest-tie-to-even : lsb|guard|round|sticky.

		however, since we also need to generate unrounded for the fused dot product operation
		we are going to test the unrounded result.

		input argument ## ####h.fffff  : normalized to the 2*fhbit format but radix is at fbits
		output result  ##.fffff'fffff  : size is 2*fhbit, radix is at 2*fbits

		test is going to enumerate input argument 1.00000 through 1.11111
	 */

	int nrOfFailedTests = 0;




	blocktriple<fbits, BlockTripleOperator::MUL> a, b, c, refResult;
	constexpr size_t hiddenBit = (size_t(1) << fbits);
	a.setnormal();
	b.setnormal();
	c.setnormal();  // we are only enumerating normal values, special handling is not tested here

	for (int scale = -2; scale < 3; ++scale) {
		for (size_t i = 0; i < NR_VALUES; i++) {
			for (size_t j = 0; j < NR_VALUES; j++) {
				// set the a input test value
				a.setbits(i + hiddenBit);  // mix in the hidden bit in the blockfraction
				a.setscale(scale);
				// set the b input test value
				b.setbits(j + hiddenBit);
				b.setscale(0);

				cout << to_binary(a) << " * " << to_binary(b) << endl;
				cout << a << " * " << b << " = " << c << endl;

				c.mul(a, b); // generate the unrounded mul value under test
				
				double aref = double(a); // cast to double is reasonable constraint for exhaustive test
				double bref = double(b); // cast to double is reasonable constraint for exhaustive test
				double cref = aref * bref; // calculate the reference test value

				refResult = cref;  // assign

				if (cref != double(c)) {
					cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << endl;
					cout << a << " * " << b << " = " << c << endl;
					cout << aref << " * " << bref << " = " << cref << " vs " << refResult << endl;

					++nrOfFailedTests;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, c, refResult);
//					cout << "---------------------\n";
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, refResult);
				}
//				if (nrOfFailedTests > 24) return nrOfFailedTests;
			}
			//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
		}
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blocktriple
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename ArgumentType>
void GenerateTestCase(ArgumentType lhs, ArgumentType rhs) {
	using namespace sw::universal;
	blocktriple<nbits, BlockTripleOperator::MUL> a, b, result;  // MUL creates a blockfraction of mbits = 2*fhbits and set the initial radix at mbits

	// convert to blocktriple
	a = lhs;  // operator=() sets the radix to nbits, which is the number of fraction bits in the blocktriple representation for a format h.fffff
	b = rhs;
	result.mul(a, b); // unrounded multiply generates the bits and sets the radix at 2*fbits, thus creating an unrounded result in mbits

	// convert blocktriples back to argument type
	ArgumentType _a = ArgumentType(a);
	ArgumentType _b = ArgumentType(b);
	ArgumentType _c = _a * _b;

	ArgumentType ref = ArgumentType(result); // use the argument type as the marshalling reference type

	// check that the round-trip through the blocktriple yields the same value as direct conversion
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(5);
	std::cout << std::setw(nbits) << lhs << " * " << std::setw(nbits) << rhs << " = " << std::setw(nbits) << lhs * rhs << '\n';
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << ": " << result << " (reference: " << _c << ")   "; 
	std::cout << (_c == ref ? "PASS" : "FAIL") << '\n';
	std::cout << to_binary(_c) << '\n';
	std::cout << to_binary(ref) << '\n';
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
	
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple multiplication failed: ";

#if MANUAL_TESTING

	GenerateTestCase<2, float>(0.3125f, 1.75f);
	return 0;

	// 2^4 = 16 * 16 * 13 =  3328 : 568 fails
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 2, BlockTripleOperator::MUL, uint8_t> >(bReportIndividualTestCases), "blocktriple<2, BlockTripleOperator::MUL, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 4, BlockTripleOperator::MUL, uint8_t> >(bReportIndividualTestCases), "blocktriple<4, BlockTripleOperator::MUL, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, BlockTripleOperator::MUL, uint8_t> >(bReportIndividualTestCases), "blocktriple<8, BlockTripleOperator::MUL, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint8_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint16_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint32_t>", "multiplication");

#if STRESS_TESTING

#endif

	// manual test does not report failures
	nrOfFailedTestCases = 0;

#else

	cout << "blocktriple multiplication validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<4, uint8_t> >(bReportIndividualTestCases), "blocktriple<4,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<4, uint16_t> >(bReportIndividualTestCases), "blocktriple<4,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<4, uint32_t> >(bReportIndividualTestCases), "blocktriple<4,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<8, uint8_t> >(bReportIndividualTestCases), "blocktriple<8,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<8, uint16_t> >(bReportIndividualTestCases), "blocktriple<8,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<8, uint32_t> >(bReportIndividualTestCases), "blocktriple<8,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<9, uint8_t> >(bReportIndividualTestCases), "blocktriple<9,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<9, uint16_t> >(bReportIndividualTestCases), "blocktriple<9,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<9, uint32_t> >(bReportIndividualTestCases), "blocktriple<9,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, uint8_t> >(bReportIndividualTestCases), "blocktriple<10,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, uint16_t> >(bReportIndividualTestCases), "blocktriple<10,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, uint32_t> >(bReportIndividualTestCases), "blocktriple<10,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, uint8_t> >(bReportIndividualTestCases), "blocktriple<11,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, uint16_t> >(bReportIndividualTestCases), "blocktriple<11,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, uint32_t> >(bReportIndividualTestCases), "blocktriple<11,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint8_t> >(bReportIndividualTestCases), "blocktriple<12,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint16_t> >(bReportIndividualTestCases), "blocktriple<12,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint32_t> >(bReportIndividualTestCases), "blocktriple<12,uint32_t>", "multiplication");

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
