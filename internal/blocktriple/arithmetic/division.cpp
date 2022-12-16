// division.cpp: functional tests for blocktriple number division
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>

// temporary
//#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
//#include <universal/internal/bitblock/bitblock.hpp>

#include <universal/native/ieee754.hpp>
// uncomment to enable operator tracing
//#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_DIV
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

// enumerate all division cases for an blocktriple<nbits,BlockType> configuration
// TODO: fix test failures in VerifyMultiplication<BlockTripleConfiguration>
template<typename BlockTripleConfiguration>
int VerifyDivision(bool reportTestCases) {
	using namespace sw::universal;
	
	constexpr size_t fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	using BlockType = typename BlockTripleConfiguration::BlockType;
	constexpr sw::universal::BlockTripleOperator op = BlockTripleConfiguration::op;

	if constexpr (op != BlockTripleOperator::DIV) {
		std::cerr << "VerifyDivision requires a blocktriple with the DIV operator designation\n";
		return 1;
	}
	constexpr size_t divbits = BlockTripleConfiguration::divbits;
	std::cout << '\n';
	std::cout << "blocktriple<" <<fbits << ',' << op << ',' << typeid(BlockType).name() << '>' << '\n';
	std::cout << "Fraction        bits : " << fbits << '\n';
	std::cout << "Division        bits : " << divbits << '\n';

	/*
		blocktriple<fbits> has fbits fraction bits in the form h.<fbits>
		Digit-recurrence produces one bit at each iteration and moves the radix point.
		
		we generate 2*fhbits result bits with radix at 2*fbits
		which we'll need to round using round-nearest-tie-to-even : lsb|guard|round|sticky.

		h.fffff / h.fffff in long division: h5.f4 f3 f2 f1 f0
		    dividend 0h.fffff 00000
			divider	 0h.fffff 00000  h5 . 
                        hffff f0000  f4
                         hfff ff000  f3
				          hff fff00  f2  
				           hf ffff0  f1  
				            h fffff  f0
			           +---------------+
			  oh.fffff'fffff     o == overflow, h == hidden, . is result radix point

		To prepare for the iterative substraction, we are normalizing the input operand to
		the result fixed-point of size 2*fhbits.
		That is:
		input argument ## ####h.fffff  : normalized to the 2*fhbit format but radix is at fbits
		output result  oh.fffff'fffff  : size is 2*fhbit, radix is at 2*fbits

		we might also want to generate unrounded result
		we are going to test the unrounded result.

		test is going to enumerate input argument 1.00000 through 1.11111 as we only work with normalized formats
	 */

	// for the test we are going to enumerate the fbits state space
	constexpr size_t NR_VALUES = (size_t(1) << fbits);

	int nrOfFailedTests = 0;

	blocktriple<fbits, BlockTripleOperator::DIV, BlockType> a, b, c;
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

				c.div(a, b); // generate the unrounded div value under test

//				std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << '\n';
//				std::cout << a << " * " << b << " = " << c << '\n';
								
				double aref = double(a);
				double bref = double(b);
				double cref = aref * bref; // calculate the reference test value

				// map the result into the unrounded representation
				blocktriple<2*fbits + 1, BlockTripleOperator::REP> reference;
				reference = cref;
				double btref = double(reference);  // map the double result to the unrounded blocktriple representation

				if (btref != double(c)) {
//					std::cout << "test case   : " << a << " * " << b << " = " << c << '\n';
//					std::cout << "conversion  : " << aref << " * " << bref << " = " << cref << " vs " << btref << '\n';
//					std::cout << "blocktriple : " << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " : " << c << '\n';

					++nrOfFailedTests;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, double(c), btref);
//					std::cout << "---------------------\n";
				}
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, reference);
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
// for most bugs they are traceable with _trace_conversion and _trace_div
template<size_t fbits, typename ArgumentType>
void TestCase(ArgumentType lhs, ArgumentType rhs) {
	using namespace sw::universal;
	blocktriple<fbits, BlockTripleOperator::DIV> a, b, result;  // DIV creates a blockfraction of divbits = 2*fhbits and sets the initial radix at divbits

	// convert to blocktriple
	a = lhs;
	std::cout << to_triple(a) << " : " << a << "(lhs = " << lhs << ')' << '\n';
	b = rhs;
	std::cout << to_triple(b) << " : " << b << "(rhs = " << rhs << ')' << '\n';
	result.div(a, b); // unrounded multiply generates the bits and sets the radix at 2*fbits, thus creating an unrounded result in mbits
	std::cout << to_triple(result) << " : " << result << "    <-------------------------" << '\n';

	// convert blocktriples back to argument type
	ArgumentType _a = ArgumentType(a);
	ArgumentType _b = ArgumentType(b);
	ArgumentType _c = _a / _b;

	ArgumentType ref = ArgumentType(result); // use the argument type as the marshalling reference type

	// map the result into the unrounded representation
	constexpr size_t divbits = 2 * (fbits);
	blocktriple<divbits, BlockTripleOperator::REP> reference;
	reference = _c;

	ArgumentType btref = ArgumentType(reference);

	// check that the round-trip through the blocktriple yields the same value as direct conversion
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(fbits);
	std::cout << "original float : " << lhs << " / " << rhs << " = " << lhs * rhs << '\n';
	std::cout << "blocktriple    : " << a << " / " << b << " = " << result << " vs reference " << reference << '\n';
	std::cout << "result         : " << to_binary(result) << '\n';
	std::cout << "reference      : " << to_binary(reference) << '\n';
	std::cout << "blocktriple    : " << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(result) << ": " << result << " (reference: " << _c << ")   ";
	std::cout << (btref == ref ? "PASS" : "FAIL") << '\n';
	std::cout << "converted float: " << _a << " / " << _b << " = " << _c << '\n';
	std::cout << to_binary(_c) << '\n';
	std::cout << to_binary(ref) << '\n';
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// TODO: this regression test is not correct yet
// 
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
	
	std::string test_suite  = "blocktriple division validation";
	std::string test_tag    = "blocktriple division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	TestCase<4>(1.0f, 1.0f);

	// TBD: design and implement a proper blocktriple test suite for DIV
	// 
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 2, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple< 2, BlockTripleOperator::DIV, uint8_t >", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 4, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple< 4, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 8, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple< 8, BlockTripleOperator::DIV, uint8_t >", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 8, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple< 8, BlockTripleOperator::DIV, uint16_t>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<12, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple<12, BlockTripleOperator::DIV, uint8_t >", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<12, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple<12, BlockTripleOperator::DIV, uint16_t>", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<12, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple<12, BlockTripleOperator::DIV, uint32_t>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 4, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple< 4, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 4, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple< 4, BlockTripleOperator::DIV, uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 4, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple< 4, BlockTripleOperator::DIV, uint32_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 8, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple< 8, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 8, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple< 8, BlockTripleOperator::DIV, uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 8, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple< 8, BlockTripleOperator::DIV, uint32_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 9, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple< 9, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 9, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple< 9, BlockTripleOperator::DIV, uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple< 9, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple< 9, BlockTripleOperator::DIV, uint32_t>", "division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<10, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple<10, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<10, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple<10, BlockTripleOperator::DIV, uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<10, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple<10, BlockTripleOperator::DIV, uint32_t>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<11, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple<11, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<11, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple<11, BlockTripleOperator::DIV, uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<11, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple<11, BlockTripleOperator::DIV, uint32_t>", "division");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<12, BlockTripleOperator::DIV, uint8_t > >(reportTestCases), "blocktriple<12, BlockTripleOperator::DIV, uint8_t >", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<12, BlockTripleOperator::DIV, uint16_t> >(reportTestCases), "blocktriple<12, BlockTripleOperator::DIV, uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< blocktriple<12, BlockTripleOperator::DIV, uint32_t> >(reportTestCases), "blocktriple<12, BlockTripleOperator::DIV, uint32_t>", "division");
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
