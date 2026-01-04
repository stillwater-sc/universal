// addition.cpp: functional tests for blocktriple number addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#define BLOCKTRIPLE_TRACE_ADD
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

// enumerate all addition cases for an blocktriple<nbits,BlockType> configuration
template<typename BlockTripleConfiguration>
int VerifyAddition(bool reportTestCases) {
	constexpr size_t fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	constexpr size_t abits = BlockTripleConfiguration::abits;
	using BlockType = typename BlockTripleConfiguration::BlockType;

	// for the test we are going to enumerate the fbits state space
	// and then shift the values into place into the declared ALU inputs.
	// forall i in NR_VALUES
	//    setBits(i + shiftLeft + hiddenBit);
	constexpr size_t NR_VALUES = (size_t(1) << fbits);

	using namespace sw::universal;
	
	std::cout << "blocktriple<" << fbits << ',' << typeid(BlockType).name() << ">/\n";
	std::cout << "Fractions bits : " << fbits << '\n';
	std::cout << "Addition  bits : " << abits << '\n';

	/*
		blocktriple<fbits> has fbits fraction bits in the form 00h.<fbits>
		We need this form of 3 bits above the radix point to capture overflow
		to the max negative number represented in 2's complement.

		Furthermore, the alignment of arguments during add/sub we need 3 additional bits 
		of information to correctly round, represented by guard, round, and sticky.
		The sticky bit would consolidate all 'tail' bits that get shifted 'out'
		after alignment of the smaller operand.

		example: blocktriple<3> represents values 
		00h.000
		00h.001
		00h.010
		...
		00h.101
		00h.110
		00h.111

		The scale shifts these value relative to 1. So a scale of -3 shifts these bits to the right, a scale of +3 shifts them to the left

	 */

	int nrOfFailedTests = 0;

	/*
	 * the structure of a blocktriple is always 00h.ffff, that is,
	 * we have an explicit normal bit.
	 * 
	 * When we are adding two blocktriples we need to at least 3 integer bits 
	 * to support 2's complement and overflow. Thus a blocktriple with a
	 * significant of 1.ffff would need to be in the form 00h.ffff.
	 * Secondly, as the smallest argument will go through alignment,
	 * there is a collection of rounding bits required to capture all
	 * the necessary information bits after alignment. This is a function
	 * of es. Empirically, 2*fhbits rounding bits have yielded correct rounding
	 * results: -> 00h.ffff becomes 00h.ffff 00000 00000
	 * 
	 * The blockfraction class captures these
	 * rounding bits     rbits = 2 * fhbits
	 * accumulation bits abits = fbits + rbits
	 * accu output             = 3 + abits
	 */

	blocktriple<abits, BlockTripleOperator::ADD> a, b, c, refResult;
	constexpr size_t hiddenBit = (size_t(1) << (abits-1));
	a.setnormal();
	b.setnormal();
	c.setnormal();  // we are only enumerating normal values, special handling is not tested here

	// NOTE: the add operator will change the arguments during alignment
	// specifically, the add operator will shift the fraction and adjust the scale
	// This requires the input values of the blocktriples to be set in the inner loop,
	// as the test input values will not remain invariant as they are manipulated by add();
	double aref, bref, cref;
	for (int scale = -6; scale < 7; ++scale) {
		for (size_t i = 0; i < NR_VALUES; i++) {
			for (size_t j = 0; j < NR_VALUES; j++) {
				// set the a input test value
				a.setbits(i*8 + hiddenBit);  // mix in the hidden bit in the blockfraction
				a.setscale(scale);
				// set the b input test value
				b.setbits(j*8 + hiddenBit);
				b.setscale(0);

//				cout << to_binary(a) << " + " << to_binary(b) << endl;
//				cout << a << " + " << b << " = " << c << endl;

				// if you generate the reference double before the alignment
				// you end up with bits in the double that you don't have
				// in the blocktriple. The scale of the block triple
				// will shift bits into the double that get potentially
				// removed from the blocktriple addition: 
				//   a catastrophic rounding failure due to the smaller fraction 
				//   in the blocktriple as compared to a double.
				// If we move the aref|bref|cref calculation till after
				// the add, then the aref|bref will be aligned and thus
				// the bits on which we make a rounding decision will be
				// closer. However, they can still be different, and I 
				// don't know yet if this is a solvable problem.
				// If it is a problem, then we are going to have problems
				// with cfloats and posits as well.

				// Question: how did we solve it with the bit-level implementation of posits?
				// There we have a value that is unrounded, and than assigned
				// at which point the rounding logic is applied to ALL the bits.

				c.add(a, b); // generate the add value under test
				
				aref = double(a); // cast to double is reasonable constraint for exhaustive test
				bref = double(b); // cast to double is reasonable constraint for exhaustive test
				cref = aref + bref; // calculate the reference test value

				refResult = cref; // sample the reference test value

				if (double(c) != double(refResult)) {  // it is possible for c to be in overflow format, i.e. 01#.ffff, and thus we need to compare c and refResult in their 'value' space, which we accomplish by the double() conversion
//					cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << endl;
//					cout << a << " + " << b << " = " << c << endl;
//					cout << aref << " + " << bref << " = " << cref << " vs " << refResult << endl;

					++nrOfFailedTests;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, refResult);
//					std::cout << "---------------------\n";
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, refResult);
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
void TestCase(ArgumentType lhs, ArgumentType rhs) {
	using namespace sw::universal;
	blocktriple<fbits, BlockTripleOperator::ADD> a, b, result, reference;

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
	std::cout << std::setprecision(fbits);
	std::cout << lhs << " + " << rhs << " = " << lhs + rhs << '\n';
	std::cout << _a << " + " << _b << " = " << _c << '\n';
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << '\n';
	reference = _c;
	std::cout << (result == reference ? "PASS" : "FAIL") << '\n' << std::endl;
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
	
	std::string test_suite  = "blocktriple addition validation";
	std::string test_tag    = "blocktriple addition";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	TestCase<4>(1.0f, 1.0f);

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple< 1, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<1, BlockTripleOperator::ADD, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple< 4, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<4, BlockTripleOperator::ADD, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple< 8, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<8, BlockTripleOperator::ADD, uint8_t>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<10, BlockTripleOperator::ADD, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<12, BlockTripleOperator::ADD, uint8_t>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<12, BlockTripleOperator::ADD, uint16_t>", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<4, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<4,BlockTripleOperator::ADD, uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<4, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<4,BlockTripleOperator::ADD, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<4, BlockTripleOperator::ADD, uint32_t> >(reportTestCases), "blocktriple<4,BlockTripleOperator::ADD, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<8, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<8,BlockTripleOperator::ADD, uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<8, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<8,BlockTripleOperator::ADD, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<8, BlockTripleOperator::ADD, uint32_t> >(reportTestCases), "blocktriple<8,BlockTripleOperator::ADD, uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<9, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<9,BlockTripleOperator::ADD, uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<9, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<9,BlockTripleOperator::ADD, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<9, BlockTripleOperator::ADD, uint32_t> >(reportTestCases), "blocktriple<9,BlockTripleOperator::ADD, uint32_t>", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, BlockTripleOperator::ADD, uint8_t> >(reportTestCases), "blocktriple<10,BlockTripleOperator::ADD, uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<10,BlockTripleOperator::ADD, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<10, BlockTripleOperator::ADD, uint32_t> >(reportTestCases), "blocktriple<10,BlockTripleOperator::ADD, uint32_t>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, BlockTripleOperator::ADD, uint8_t > >(reportTestCases), "blocktriple<11,BlockTripleOperator::ADD, uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<11,BlockTripleOperator::ADD, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, BlockTripleOperator::ADD, uint32_t> >(reportTestCases), "blocktriple<11,BlockTripleOperator::ADD, uint32_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<11, BlockTripleOperator::ADD, uint64_t> >(reportTestCases), "blocktriple<11,BlockTripleOperator::ADD, uint64_t>", "addition");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, BlockTripleOperator::ADD, uint8_t > >(reportTestCases), "blocktriple<12,BlockTripleOperator::ADD, uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, BlockTripleOperator::ADD, uint16_t> >(reportTestCases), "blocktriple<12,BlockTripleOperator::ADD, uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, BlockTripleOperator::ADD, uint32_t> >(reportTestCases), "blocktriple<12,BlockTripleOperator::ADD, uint32_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< blocktriple<12, BlockTripleOperator::ADD, uint64_t> >(reportTestCases), "blocktriple<12,BlockTripleOperator::ADD, uint64_t>", "addition");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
