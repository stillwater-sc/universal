// rounding.cpp: test suite runner for blocktriple rounding decisions
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
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

/*
precondition for rounding is a 1's complement bit pattern, and no denorm
That is:  patterns of the form 
   0b001.ffff
   0b010.ffff
   0b011.ffff
  and excluding
   0b000.ffff
   0b1##.ffff
 */

	// Verify blocktriple rounding behavior
	template<size_t fbits, sw::universal::BlockTripleOperator op, typename bt>
	int VerifyBlocktripleRounding() {
		using namespace sw::universal;
		using BlockTriple = blocktriple<fbits, op, bt>;
		BlockTriple a, nut;
		std::cout << ' ' << type_tag(nut) << " with radix point at " << nut.radix << ' ';
		int nrOfFailures = 0;

		size_t fractionbits = fbits; // default is ADD
		if constexpr (op == BlockTripleOperator::MUL) {
			fractionbits = 2*fbits; // override to 2*fbits
		}
		size_t START = (1ull << fractionbits);
		size_t NR_VALUES = (1ull << (fractionbits + 2));
		for (size_t i = START; i < NR_VALUES; ++i) {
			if (i == 0) a.setzero(); else a.setnormal();
			a.setbits(i);
			// for add/sub ops    0b0ii.fffff with only a single bit of rounding. 
			// TODO is that always true? if you have dynamic range, don't you have 2*fhbits of bits to examine?
			// for mul op         0bii.fffff`fffff with fbits of rounding
			//auto retval = a.roundingDecision();
			//std::cout << to_triple(a) << (retval.first ? " rounds up\n" : " rounds down\n");

			bool correct = true;
			if (!correct) {
				++nrOfFailures;
				std::cout << "FAIL: " << std::setw(10) << i << " : " << to_binary(a) << " != ";
				std::cout << to_binary(nut) << '\n';
			}
			else {
				//std::cout << "PASS: " << std::setw(10) << i << " : " << to_binary(a) << " == " << to_binary(nut) << '\n';
			}
		}
		std::cout << (nrOfFailures ? "FAIL\n" : "PASS\n");
		return nrOfFailures;
	}


}}  // namespace sw::universal

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

	std::string test_suite  = "blocktriple rounding validation";
	std::string test_tag    = "bt_rounding";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

//	nrOfFailedTestCases += VerifyBlocktripleRounding<5, BlockTripleOperator::REPRESENTATION, uint8_t>();
	nrOfFailedTestCases += VerifyBlocktripleRounding<5, BlockTripleOperator::ADD, uint8_t>();
	nrOfFailedTestCases += VerifyBlocktripleRounding<5, BlockTripleOperator::MUL, uint8_t>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyBlocktripleRounding<5, BlockTripleOperator::ADD, uint8_t>();
	nrOfFailedTestCases += VerifyBlocktripleRounding<5, BlockTripleOperator::MUL, uint8_t>();
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
