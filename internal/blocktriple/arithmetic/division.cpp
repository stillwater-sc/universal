// division.cpp: functional tests for blocktriple number division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>

// temporary
//#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
//#include <universal/internal/bitblock/bitblock.hpp>

#include <universal/native/ieee754.hpp>
// uncomment to enable operator tracing
//#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_DIV
#include <universal/internal/blocktriple/blocktriple_debug.hpp>  // constexprClassParameters + tracing (#1334)
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/internal/blocksignificand/manipulators.hpp>   // (#1334)

// Enumerate all division cases of a blocktriple<fbits, DIV> configuration against an exact
// reference (#1745).
//
// The operands are prepared as cfloat::normalizeDivision prepares them: the significand 1.f
// shifted up by divshift, at the DIV radix R = divbits. div() then produces the UNROUNDED
// quotient: T = floor(A * 2^R / B) for the integer significands A and B, with bit 0 set when
// the division was inexact, at scale sa - sb (shifted left once, scale lowered, when the
// quotient is below 1; the value is the same). So the expected VALUE is
// (T | inexact) * 2^(sa - sb - R), and with at most 3 fbits + 6 significant bits it is exact
// in a double for fbits <= 12, as is the blocktriple's own conversion.
//
// This used to compare against the PRODUCT of the operands, with operands that skipped the
// divshift alignment, and it ran in MANUAL_TESTING mode, so 327680 failures never reached CI.
template<typename BlockTripleConfiguration>
int VerifyDivision(bool reportTestCases) {
	using namespace sw::universal;

	constexpr unsigned fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	constexpr sw::universal::BlockTripleOperator op = BlockTripleConfiguration::op;
	static_assert(op == BlockTripleOperator::DIV, "VerifyDivision requires a blocktriple with the DIV operator designation");
	static_assert(fbits <= 12, "the exact reference holds the quotient in 64 bits and a double");

	constexpr unsigned divshift = BlockTripleConfiguration::divshift;
	constexpr int      R        = BlockTripleConfiguration::radix;
	constexpr std::uint64_t NR_VALUES = (std::uint64_t(1) << fbits);
	constexpr std::uint64_t hiddenBit = NR_VALUES;

	int nrOfFailedTests = 0;
	BlockTripleConfiguration a, b, c;
	for (int scale = -2; scale < 3; ++scale) {
		for (std::uint64_t i = 0; i < NR_VALUES; ++i) {
			for (std::uint64_t j = 0; j < NR_VALUES; ++j) {
				const bool sa = ((i + j) & 1u) != 0;   // mix the signs
				const bool sb = ((i ^ j) & 2u) != 0;
				a.setnormal(); a.setsign(sa); a.setscale(scale); a.setbits((i + hiddenBit) << divshift);
				b.setnormal(); b.setsign(sb); b.setscale(0);     b.setbits((j + hiddenBit) << divshift);

				c.div(a, b);   // the unrounded quotient under test

				const std::uint64_t A = i + hiddenBit, B = j + hiddenBit;
				const std::uint64_t numerator = A << R;
				const std::uint64_t T = (numerator / B) | ((numerator % B) != 0 ? 1u : 0u);
				const double expected = (sa != sb ? -1.0 : 1.0) * std::ldexp(static_cast<double>(T), scale - R);
				const double result = double(c);
				if (result != expected) {
					++nrOfFailedTests;
					if (reportTestCases && nrOfFailedTests < 10) {
						std::cerr << "FAIL: " << to_triple(a) << " / " << to_triple(b) << " = " << to_triple(c)
						          << " : " << result << ", expected " << expected << '\n';
					}
				}
			}
		}
	}
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

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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
