// shift.cpp: functional tests for block binary number shifts
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include "universal/blockbin/blockbinary.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/blockbinary_helpers.hpp"

//#include <bitset>

void ShiftExamples() {
	using namespace std;
	using namespace sw::unum;

	blockbinary<37, uint8_t> a;
	blockbinary<37, uint16_t> b;
	blockbinary<37, uint32_t> c;
	//	blockbinary<37, int64_t> d;

	a.set_raw_bits(0xAAAAAAAAAA);
	b.set_raw_bits(0x5555555555);
	c.set_raw_bits(0xAAAAAAAAAA);
	//	d.set_raw_bits(0x5555555555);

	cout << to_binary(a, true) << endl;
	cout << to_binary(b, true) << endl;
	cout << to_binary(c, true) << endl;
	cout << to_hex(a, true) << endl;
	cout << to_hex(b, true) << endl;
	cout << to_hex(c, true) << endl;
	//	cout << to_hex(d, true) << endl;

	cout << "shifting\n";
	a.set_raw_bits(0x155555555);
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a <<= 1;
	cout << to_binary(a, true) << endl;
	a >>= 4;
	cout << to_binary(a, true) << endl;
	a >>= 9;
	cout << to_binary(a, true) << endl;

	b.set_raw_bits(0x155555555);
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b <<= 1;
	cout << to_binary(b, true) << endl;
	b >>= 4;
	cout << to_binary(b, true) << endl;
	b >>= 17;
	cout << to_binary(b, true) << endl;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "block shifts: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	ShiftExamples();

	blockbinary<37, uint8_t> a;
	blockbinary<37, uint16_t> b;


	a.set_raw_bits(0xAAAAAAAAAA);
	b.set_raw_bits(0x5555555555);

	a.set_raw_bits(0x1FFFFFFFF);
	b.set_raw_bits(0x1FFFFFFFF);

	cout << to_binary(a, true) << endl;
	a <<= 9;
	cout << to_binary(a, true) << endl;

	cout << to_binary(b, true) << endl;
	b <<= 17;
	cout << to_binary(b, true) << endl;

	a.set_raw_bits(0xAA00FF00FF);
	cout << to_binary(a, true) << endl;
	a <<= 8;
	cout << to_binary(a, true) << endl;
	a <<= 16;
	cout << to_binary(a, true) << endl;
	a.set_raw_bits(0xAA00FF00FF);
	cout << to_binary(a, true) << endl;
	a <<= 16;
	cout << to_binary(a, true) << endl;
	a <<= 8;
	cout << to_binary(a, true) << endl;
	a.set_raw_bits(0x0000FF00FF);
	cout << to_binary(a, true) << endl;
	a <<= 16;
	cout << to_binary(a, true) << endl;
	a.set_raw_bits(0x0000FF00FF);
	cout << to_binary(a, true) << endl;
	a <<= 8;
	cout << to_binary(a, true) << endl;


	//nrOfFailedTestCases += ReportTestResult(VerifyShiftLeft<4>("Manual Testing", true), "array<4,1>", "addition");
	//nrOfFailedTestCases += ReportTestResult(VerifyShiftRight<4>("Manual Testing", true), "array<4,1>", "addition");


#if STRESS_TESTING

#endif

#else

	cout << "block shifts validation" << endl;



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
