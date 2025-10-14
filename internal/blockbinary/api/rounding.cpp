// rounding.cpp: functional tests for rounding using blockbinary numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <sstream>

// Configure the blockbinary template environment
// first: enable general or specialized array configurations
#define BLOCKBIN_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define BLOCKBIN_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/blockbinary_test_status.hpp>

// report helper to interpret rounding decision encoding
std::string roundingDecision(int roundingDirection) {
	std::stringstream ss;
	ss << (roundingDirection == 0 ? "tie" : (roundingDirection > 0 ? "up" : "down"));
	return ss.str();
}

int ValidateAny(bool reportTestCases) {
	int nrFailures = 0;
	sw::universal::blockbinary<18> a;

	a.setbits(0x32000); // 11'0010'0000'0000'0000
	if (a.any(8)) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(9)) {	++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(10)) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(11)) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(12)) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(13) != true) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(14) != true) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(16) != true) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }

	a.setbits(0x3244); // 00'0011'0010'0100'0100
	if (a.any(1)) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(4) != true) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	a.setbits(0x3240); // 00'0011'0010'0100'0000
	if (a.any(5)) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(6) != true) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }
	if (a.any(7) != true) { ++nrFailures; if (reportTestCases) std::cout << "fail\n"; }

	return nrFailures;
}

int ValidateSpecialRoundingCases(bool reportTestCases) {
	using namespace sw::universal;

	blockbinary<8> a, b, roundedResult;
	blockbinary<16> c;
	bool roundUp;

	int nrOfFailedTestCases = 0;

	// test cases at the boundary
//                   | lsb
//               '0001     round down                         1 -> 1
//                '0010    round down                         1 -> 1
//                 '0010   tie with lsb == 0, round down      0 -> 0
//                 '0100   round down                         1 -> 1
//                 '0101   round down                         1 -> 1
//                 '0110   tie with lsb == 1, round up        1 -> 2
	a = 1;
	roundUp = a.roundingMode(0);
	if (roundUp) ++nrOfFailedTestCases;
	a = 2;
	roundUp = a.roundingMode(1);
	if (roundUp) ++nrOfFailedTestCases;
	a = 2;
	roundUp = a.roundingMode(2);
	if (roundUp) ++nrOfFailedTestCases;
	a = 4;
	roundUp = a.roundingMode(2);
	if (roundUp) ++nrOfFailedTestCases;
	a = 5;
	roundUp = a.roundingMode(2);
	if (roundUp) ++nrOfFailedTestCases;
	a = 6;
	roundUp = a.roundingMode(2);
	if (!roundUp) ++nrOfFailedTestCases;

	// test cases at the boundary
	//                  | lsb
	//                 '0001   round down                         0 -> 0
	//                 '0010   round down                         0 -> 0
	//                 '0011   round down                         0 -> 0
	//                 '0100   tie with lsb == 0, round down      0 -> 0
	//                 '0101   round up                           0 -> 0
	//                 '0110   round up                           0 -> 1
	//                 '0111   round up                           0 -> 1
	//                 '1100   tie with lsb == 1, round up        1 -> 2
	a = 1;
	roundUp = a.roundingMode(3);
	if (roundUp) ++nrOfFailedTestCases;
	a = 2;
	roundUp = a.roundingMode(3);
	if (roundUp) ++nrOfFailedTestCases;
	a = 3;
	roundUp = a.roundingMode(3);
	if (roundUp) ++nrOfFailedTestCases;
	a = 4;
	roundUp = a.roundingMode(3);
	if (roundUp) ++nrOfFailedTestCases;
	a = 5;
	roundUp = a.roundingMode(3);
	if (!roundUp) ++nrOfFailedTestCases;
	a = 6;
	roundUp = a.roundingMode(3);
	if (!roundUp) ++nrOfFailedTestCases;
	a = 7;
	roundUp = a.roundingMode(3);
	if (!roundUp) ++nrOfFailedTestCases;
	a = 12;
	roundUp = a.roundingMode(3);
	if (!roundUp) ++nrOfFailedTestCases;

	std::cout << "First Nibble: " << (nrOfFailedTestCases > 0 ? "FAIL" : "PASS") << '\n';

	// test cases at the boundary
	//             | lsb
	//             1000'0000   round down                         1 -> 1
	//             0100'0000   tie with lsb == 0, round down      0 -> 0
	//             1100'0000   tie with lsb == 1, round up        1 -> 2
	//             0100'0001   round up                           0 -> 1
	//             0110'0000   round up                           0 -> 1
	//             0110'0001   round up                           0 -> 1

	a = 0x80;
	roundUp = a.roundingMode(7);
	if (roundUp) ++nrOfFailedTestCases;
	a = 0x40;
	roundUp = a.roundingMode(7);
	if (roundUp) ++nrOfFailedTestCases;
	a = 0xC0;
	roundUp = a.roundingMode(7);
	if (!roundUp) ++nrOfFailedTestCases;
	a = 0x41;
	roundUp = a.roundingMode(7);
	if (!roundUp) ++nrOfFailedTestCases;
	a = 0x60;
	roundUp = a.roundingMode(7);
	if (!roundUp) ++nrOfFailedTestCases;
	a = 0x61;
	roundUp = a.roundingMode(7);
	if (!roundUp) ++nrOfFailedTestCases;

	std::cout << "Second Nibble: " << (nrOfFailedTestCases > 0 ? "FAIL" : "PASS") << '\n';


	// test case  16 bits with lsb at 9
	//           | lsb
	//   0000'0000'0000'0000   round down                         0 -> 0
	//   0000'0000'1000'0000   tie with lsb == 0, round down    128 -> 0
	//   0000'0001'1000'0000   tie with lsb == 1, round up      384 -> 2
	//   0000'0000'1000'0001   round up                         129 -> 1
	//   0000'0000'1100'0000   round up                         192 -> 1
	//   0000'0000'1100'0001   round up                         193 -> 1
	//   0000'0000'0100'0001   round down                        65 -> 0

	// result is 128 -> rounds to 0
	a = 64;
	b = 2;
	c = urmul(a, b);
	if (reportTestCases) std::cout << "unrounded result is " << to_hex(c, true) << '\n';
	roundUp = c.roundingMode(8);
	if (reportTestCases) std::cout << (roundUp ? "round up" : "round down") << '\n';
	c >>= 8;
	roundedResult = c;
	if (reportTestCases) std::cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << '\n';
	if (roundUp) roundedResult += 1;
	if (reportTestCases) std::cout << "final rounded result: " << to_hex(roundedResult) << '\n';
	if (reportTestCases) std::cout << std::endl;
	if (roundedResult != blockbinary<8>(0ll)) ++nrOfFailedTestCases;

	// result is 384 -> rounds to 2
	a = 96;
	b = 4;
	c = urmul(a, b);
	if (reportTestCases) std::cout << "unrounded result is " << to_hex(c, true) << '\n';
	roundUp = c.roundingMode(8);
	if (reportTestCases) std::cout << (roundUp ? "round up" : "round down") << '\n';
	c >>= 8;
	roundedResult = c;
	if (reportTestCases) std::cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << '\n';
	if (roundUp) roundedResult += 1;
	if (reportTestCases) std::cout << "final rounded result: " << to_hex(roundedResult) << '\n';
	if (reportTestCases) std::cout << std::endl;
	if (roundedResult != blockbinary<8>(2ll)) ++nrOfFailedTestCases;

	// result is 129 -> rounds to 1
	a = 64;
	b = 2;
	c = urmul(a, b); c += 1;
	if (reportTestCases) std::cout << "unrounded result is " << to_hex(c, true) << '\n';
	roundUp = c.roundingMode(8);
	if (reportTestCases) std::cout << (roundUp ? "round up" : "round down") << '\n';
	c >>= 8;
	roundedResult = c;
	if (reportTestCases) std::cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << '\n';
	if (roundUp) roundedResult += 1;
	if (reportTestCases) std::cout << "final rounded result: " << to_hex(roundedResult) << '\n';
	if (reportTestCases) std::cout << std::endl;
	if (roundedResult != blockbinary<8>(1ll)) ++nrOfFailedTestCases;

	// result is 192 -> rounds to 1
	a = 96;
	b = 2;
	c = urmul(a, b);
	if (reportTestCases) std::cout << "unrounded result is " << to_hex(c, true) << '\n';
	roundUp = c.roundingMode(8);
	if (reportTestCases) std::cout << (roundUp ? "round up" : "round down") << '\n';
	c >>= 8;
	roundedResult = c;
	if (reportTestCases) std::cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << '\n';
	if (roundUp) roundedResult += 1;
	if (reportTestCases) std::cout << "final rounded result: " << to_hex(roundedResult) << '\n';
	if (reportTestCases) std::cout << std::endl;
	if (roundedResult != blockbinary<8>(1ll)) ++nrOfFailedTestCases;

	// result is 193 -> rounds to 1
	a = 96;
	b = 2;
	c = urmul(a, b); c += 1;
	if (reportTestCases) std::cout << "unrounded result is " << to_hex(c, true) << '\n';
	roundUp = c.roundingMode(8);
	if (reportTestCases) std::cout << (roundUp ? "round up" : "round down") << '\n';
	c >>= 8;
	roundedResult = c;
	if (reportTestCases) std::cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << '\n';
	if (roundUp) roundedResult += 1;
	if (reportTestCases) std::cout << "final rounded result: " << to_hex(roundedResult) << '\n';
	if (reportTestCases) std::cout << std::endl;
	if (roundedResult != blockbinary<8>(1ll)) ++nrOfFailedTestCases;

	// result is 65 -> rounds to 0
	a = 32;
	b = 2;
	c = urmul(a, b); c += 1;
	if (reportTestCases) std::cout << "unrounded result is " << to_hex(c, true) << '\n';
	roundUp = c.roundingMode(8);
	if (reportTestCases) std::cout << (roundUp ? "round up" : "round down") << '\n';
	c >>= 8;
	roundedResult = c;
	if (reportTestCases) std::cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << '\n';
	if (roundUp) roundedResult += 1;
	if (reportTestCases) std::cout << "final rounded result: " << to_hex(roundedResult) << '\n';
	if (reportTestCases) std::cout << std::endl;
	if (roundedResult != blockbinary<8>(0ll)) ++nrOfFailedTestCases;

	std::cout << "Second Byte: " << (nrOfFailedTestCases > 0 ? "FAIL" : "PASS") << '\n';

	return nrOfFailedTestCases;
}

template<size_t nbits>
int ValidateRounding(bool reportTestCases) {
	int nrTestFailures = 0;

	return nrTestFailures;
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
	
	std::string test_suite  = "blockbinary rounding validation";
	std::string test_tag    = "rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ValidateAny(reportTestCases);

	// basic algorithm
	//  010101...010101010101
	//               |  the source arithmetic needs to round at this point
	//                | guard bit
	//                 | rounding bit
	//                  ----- OR'ed to generate the sticky bit
	// rounding logic table
	//   lsb    guard   round   sticky   rounding decision
	//    0       0       x       x      round down
	//    0       1       0       0      tie, round to even -> LSB = 0, thus round down
	//    1       1       0       0      tie, round to even -> LSB = 1, thus round up
	//    x       1       0       1      round up
	//    x       1       1       0      round up
	//    x       1       1       1      round up

	//  111111...110101010101
	//               |  the source arithmetic needs to round at this point
	//                | guard bit
	//                 | rounding bit
	//                  ----- OR'ed to generate the sticky bit
	// rounding logic table
	//   lsb    guard   round   sticky   rounding decision
	//    0       0       x       x      round down
	//    0       1       0       0      tie, round to even -> LSB = 0, thus round down
	//    1       1       0       0      tie, round to even -> LSB = 1, thus round up
	//    x       1       0       1      round up
	//    x       1       1       0      round up
	//    x       1       1       1      round up

	nrOfFailedTestCases = ReportTestResult(ValidateSpecialRoundingCases(reportTestCases), test_tag, "special rounding cases");

	//nrOfFailedTestCases = ReportTestResult(ValidateRounding<4>(reportTestCases), tag, "urmul");
	

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(ValidateSpecialRoundingCases(reportTestCases), test_tag, "special rounding cases");

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
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
