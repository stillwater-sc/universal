// api.cpp: class interface tests for arbitrary configuration fixed-point addition
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include "universal/fixpnt/fixed_point.hpp"
// fixed-point type manipulators such as pretty printers
#include "universal/fixpnt/fixpnt_manipulators.hpp"
//#include "universal/fixpnt/math_functions.hpp"
//#include "../utils/fixpnt_test_suite.hpp"

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	cout << "fixed-point class interface tests" << endl;

	// construction
	{
		// default construction using default arithmetic (Modular) and default BlockType (uint8_t)
		fixpnt<8, 4> a, b(-8.125f), c(7.875), d(-7.875); // replace with long double init  d(-7.875l);
		// b initialized to -8.125 in modular arithmetic becomes 7.875: -8.125 = b1000.0010 > maxneg -> becomes b0111.1110
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		cout << a << ' ' << b << ' ' << c << ' ' << d << endl;
	}

	{
		// construction with explicit arithmetic type and default BlockType (uint8_t)
		fixpnt<8, 4, Modular> a, b(-8.125), c(7.875), d(-7.875);
		// b initialized to -8.125 in modular arithmetic becomes 7.875: -8.125 = b1000.0010 > maxneg -> becomes b0111.1110
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
	}

	{
		// construction with explicit arithmetic type and default BlockType (uint8_t)
		fixpnt<8, 4, Saturation> a(-8.0), b(-8.125), c(7.875), d(-7.875);
		// b initialized to -8.125 in saturating arithmetic becomes -8
		if (0 != (c + d)) ++nrOfFailedTestCases;
		if (a != b) ++nrOfFailedTestCases;
		cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
	}

	{
		// construction with explicit arithmetic type and BlockType
		fixpnt<16, 4, Modular, uint16_t> a, b(-2048.125f), c(2047.875), d(-2047.875);
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
//		cout << to_binary(a, true) << ' ' << to_binary(b, true) << ' ' << to_binary(c, true) << ' ' << to_binary(d, true) << endl;
		cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
		cout << a << ' ' << b << ' ' << c << ' ' << d << endl;
	}

	if (nrOfFailedTestCases < 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "PASS" << endl;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
