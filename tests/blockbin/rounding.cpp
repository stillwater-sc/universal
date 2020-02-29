// rounding.cpp: functional tests for rounding using blockbinary numbers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>

// Configure the blockbinary template environment
// first: enable general or specialized array configurations
#define BLOCKBIN_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define BLOCKBIN_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include "universal/blockbin/blockbinary.hpp"
#include "../utils/test_helpers.hpp"
#include "../utils/blockbinary_helpers.hpp"

// report helper to interpret rounding decision encoding
std::string roundingDecision(int roundingDirection) {
	std::stringstream ss;
	ss << (roundingDirection == 0 ? "tie" : (roundingDirection > 0 ? "up" : "down"));
	return ss.str();
}

int ValidateAny(bool bReportIndividualTestCases) {
	int nrFailures = 0;
	sw::unum::blockbinary<18> a;

	a.set_raw_bits(0x32000); // 11'0010'0000'0000'0000
	if (a.any(8)) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(9)) {	++nrFailures; std::cout << "fail\n"; }
	if (a.any(10)) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(11)) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(12)) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(13) != true) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(14) != true) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(16) != true) { ++nrFailures; std::cout << "fail\n"; }

	a.set_raw_bits(0x3244); // 00'0011'0010'0100'0100
	if (a.any(1)) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(4) != true) { ++nrFailures; std::cout << "fail\n"; }
	a.set_raw_bits(0x3240); // 00'0011'0010'0100'0000
	if (a.any(5)) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(6) != true) { ++nrFailures; std::cout << "fail\n"; }
	if (a.any(7) != true) { ++nrFailures; std::cout << "fail\n"; }

	return nrFailures;
}

template<size_t nbits>
int ValidateRounding(bool bReportIndividualTestCases) {
	int nrTestFailures = 0;

	return nrTestFailures;
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

	std::string tag = "rounding:";

#if MANUAL_TESTING

	nrOfFailedTestCases += ValidateAny(bReportIndividualTestCases);

	blockbinary<8> a, b, roundedResult;
	blockbinary<16> c;
	bool roundUp;

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
	cout << "unrounded result is " << to_hex(c, true) << endl;
	roundUp = c.roundingMode(8); 
	cout << (roundUp ? "round up" : "round down") << endl;
	c >>= 8;
	roundedResult = c;
	cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << endl;
	if (roundUp) roundedResult += 1;
	cout << "final rounded result: " << to_hex(roundedResult) << endl;
	cout << endl;

	// result is 384 -> rounds to 2
	a = 96;
	b = 4;
	c = urmul(a, b);
	cout << "unrounded result is " << to_hex(c, true) << endl;
	roundUp = c.roundingMode(8);
	cout << (roundUp ? "round up" : "round down") << endl;
	c >>= 8;
	roundedResult = c;
	cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << endl;
	if (roundUp) roundedResult += 1;
	cout << "final rounded result: " << to_hex(roundedResult) << endl;
	cout << endl;

	// result is 129 -> rounds to 1
	a = 64;
	b = 2;
	c = urmul(a, b); c += 1;
	cout << "unrounded result is " << to_hex(c, true) << endl;
	roundUp = c.roundingMode(8);
	cout << (roundUp ? "round up" : "round down") << endl;
	c >>= 8;
	roundedResult = c;
	cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << endl;
	if (roundUp) roundedResult += 1;
	cout << "final rounded result: " << to_hex(roundedResult) << endl;
	cout << endl;

	// result is 192 -> rounds to 1
	a = 96;
	b = 2;
	c = urmul(a, b);
	cout << "unrounded result is " << to_hex(c, true) << endl;
	roundUp = c.roundingMode(8);
	cout << (roundUp ? "round up" : "round down") << endl;
	c >>= 8;
	roundedResult = c;
	cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << endl;
	if (roundUp) roundedResult += 1;
	cout << "final rounded result: " << to_hex(roundedResult) << endl;
	cout << endl;

	// result is 193 -> rounds to 1
	a = 96;
	b = 2;
	c = urmul(a, b); c += 1;
	cout << "unrounded result is " << to_hex(c, true) << endl;
	roundUp = c.roundingMode(8);
	cout << (roundUp ? "round up" : "round down") << endl;
	c >>= 8;
	roundedResult = c;
	cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << endl;
	if (roundUp) roundedResult += 1;
	cout << "final rounded result: " << to_hex(roundedResult) << endl;
	cout << endl;

	// result is 65 -> rounds to 0
	a = 32;
	b = 2;
	c = urmul(a, b); c += 1;
	cout << "unrounded result is " << to_hex(c, true) << endl;
	roundUp = c.roundingMode(8);
	cout << (roundUp ? "round up" : "round down") << endl;
	c >>= 8;
	roundedResult = c;
	cout << "shifted unrounded result: " << to_hex(c) << " result in orginal system: " << to_hex(roundedResult) << endl;
	if (roundUp) roundedResult += 1;
	cout << "final rounded result: " << to_hex(roundedResult) << endl;
	cout << endl;

	nrOfFailedTestCases = ReportTestResult(ValidateRounding<4>(bReportIndividualTestCases), tag, "urmul");
	
#if STRESS_TESTING

	// manual exhaustive test

#endif

#else
	cout << "Fixed-point modular assignment validation" << endl;


//	nrOfFailedTestCases += ReportTestResult(VerifyModularAddition<8, 0>(tag, bReportIndividualTestCases), "fixpnt<8,0>", "addition");

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
