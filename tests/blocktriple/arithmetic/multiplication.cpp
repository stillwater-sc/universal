// multiplication.cpp: functional tests for blocktriple number multiplication
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
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
	using namespace sw::universal;
	
	constexpr size_t fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	using BlockType = typename BlockTripleConfiguration::BlockType;
	constexpr sw::universal::BlockTripleOperator op = BlockTripleConfiguration::op;

	if constexpr (op != BlockTripleOperator::MUL) {
		std::cerr << "VerifyMultiplication requires a blocktriple with the MUL operator designation\n";
		return 1;
	}
//	constexpr size_t mbits = BlockTripleConfiguration::mbits;
//	cout << endl;
//	cout << "blocktriple<" <<fbits << ',' << op << ',' << typeid(BlockType).name() << '>' << endl;
//	cout << "Fraction        bits : " << fbits << endl;
//	cout << "Multiplication  bits : " << mbits << endl;

	/*
		blocktriple<fbits> has fbits fraction bits in the form h.<fbits>
		Multiplication doubles the bits in the result and moves the radix point.
		
		we generate 2*fhbits result bits with radix at 2*fbits
		which we'll need to round using round-nearest-tie-to-even : lsb|guard|round|sticky.

		h.fffff * h.fffff in long multiplication: h5.f4 f3 f2 f1 f0
		             h.fffff
					 h.fffff  f0
                    hf.ffff0  f1
                   hff.fff00  f2
				  hfff.ff000  f3
				 hffff.f0000  f4
				hfffff.00000  h5
			+---------------+
			  oh.fffff'fffff     o == overflow, h == hidden, . is result radix point

		To prepare for the multiplication, we are normalizing the input operand to
		the result fixed-point of size 2*fhbits.
		That is:
		input argument ## ####h.fffff  : normalized to the 2*fhbit format but radix is at fbits
		output result  ##.fffff'fffff  : size is 2*fhbit, radix is at 2*fbits

		we also need to generate unrounded for the fused dot product operation
		we are going to test the unrounded result.

		test is going to enumerate input argument 1.00000 through 1.11111
	 */

	// for the test we are going to enumerate the fbits state space
	constexpr size_t NR_VALUES = (size_t(1) << fbits);

	int nrOfFailedTests = 0;

	blocktriple<fbits, BlockTripleOperator::MUL, BlockType> a, b, c;
	constexpr size_t hiddenBit = (size_t(1) << fbits);
	a.setnormal();
	b.setnormal();
	c.setnormal();  // we are only enumerating normal values, special handling is not tested here

	// test design
	// a * b, both fbits fraction bits
	// (+, scale, 01.00000) * (+, 0, 01.00000)
	// (+, scale, 01.00000) * (+, 0, 01.00001)
	for (int scale = -2; scale < 3; ++scale) {
		for (size_t i = 0; i < NR_VALUES; i++) {
			for (size_t j = 0; j < NR_VALUES; j++) {
				// set the a input test value
				a.setbits(i + hiddenBit);  // mix in the hidden bit in the blockfraction
				a.setscale(scale);
				a.setradix(fbits);
				// set the b input test value
				b.setbits(j + hiddenBit);
				b.setscale(0);
				b.setradix(fbits);

				c.mul(a, b); // generate the unrounded mul value under test

//				std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << '\n';
//				std::cout << a << " * " << b << " = " << c << '\n';
								
				double aref = double(a);
				double bref = double(b);
				double cref = aref * bref; // calculate the reference test value

				// map the result into the unrounded representation
				blocktriple<2*fbits, BlockTripleOperator::REPRESENTATION> reference;
				reference = cref;
				double btref = double(reference);  // map the double result to the unrounded blocktriple representation

				if (btref != double(c)) {
					std::cout << "test case   : " << a << " * " << b << " = " << c << '\n';
					std::cout << "conversion  : " << aref << " * " << bref << " = " << cref << " vs " << btref << '\n';
					std::cout << "blocktriple : " << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << '\n';

					++nrOfFailedTests;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, double(c), btref);
//					std::cout << "---------------------\n";
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, reference);
				}
//				if (nrOfFailedTests > 24) return nrOfFailedTests;
			}
			//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
		}
	}
//	std::cout << '\n';
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blocktriple
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t fbits, typename ArgumentType>
void GenerateTestCase(ArgumentType lhs, ArgumentType rhs) {
	using namespace sw::universal;
	blocktriple<fbits, BlockTripleOperator::MUL> a, b, result;  // MUL creates a blockfraction of mbits = 2*fhbits and set the initial radix at mbits

	// convert to blocktriple
	a = lhs;  // operator=() sets the radix to nbits, which is the number of fraction bits in the blocktriple representation for a format h.fffff
	b = rhs;
	result.mul(a, b); // unrounded multiply generates the bits and sets the radix at 2*fbits, thus creating an unrounded result in mbits

	// convert blocktriples back to argument type
	ArgumentType _a = ArgumentType(a);
	ArgumentType _b = ArgumentType(b);
	ArgumentType _c = _a * _b;

	ArgumentType ref = ArgumentType(result); // use the argument type as the marshalling reference type

	// map the result into the unrounded representation
	constexpr size_t mbits = 2 * (fbits);
	blocktriple<mbits, BlockTripleOperator::REPRESENTATION> reference;
	reference = _c;

	ArgumentType btref = ArgumentType(reference);

	// check that the round-trip through the blocktriple yields the same value as direct conversion
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(5);
	std::cout << "original float : " << lhs << " * " << rhs << " = " << lhs * rhs << '\n';
	std::cout << "blocktriple    : " << a << " * " << b << " = " << result << " vs reference " << reference << '\n';
	std::cout << "result         : " << to_binary(result) << '\n';
	std::cout << "reference      : " << to_binary(reference) << '\n';
	std::cout << "blocktriple    : " << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << ": " << result << " (reference: " << _c << ")   ";
	std::cout << (btref == ref ? "PASS" : "FAIL") << '\n';
	std::cout << "converted float: " << _a << " * " << _b << " = " << _c << '\n';
	std::cout << to_binary(_c) << '\n';
	std::cout << to_binary(ref) << '\n';
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);
	
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple multiplication failed: ";

#if MANUAL_TESTING

	GenerateTestCase<2, float>(0.375f, 1.5f);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 2, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple< 2, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 4, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple< 4, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple< 8, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple< 8, BlockTripleOperator::MUL, uint16_t>", "multiplication");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint32_t>", "multiplication");

#endif

	// manual test does not report failures
	nrOfFailedTestCases = 0;

#else

	std::cout << "blocktriple multiplication validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 4, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple< 4, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 4, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple< 4, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 4, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple< 4, BlockTripleOperator::MUL, uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple< 8, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple< 8, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple< 8, BlockTripleOperator::MUL, uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 9, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple< 9, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 9, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple< 9, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 9, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple< 9, BlockTripleOperator::MUL, uint32_t>", "multiplication");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple<10, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple<10, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple<10, BlockTripleOperator::MUL, uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple<11, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple<11, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple<11, BlockTripleOperator::MUL, uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint8_t > >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint8_t >", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint16_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, BlockTripleOperator::MUL, uint32_t> >(bReportIndividualTestCases), "blocktriple<12, BlockTripleOperator::MUL, uint32_t>", "multiplication");


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
