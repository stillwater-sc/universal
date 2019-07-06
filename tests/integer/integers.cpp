//  integers.cpp : test suite for abitrary precision integers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include "../../integer/integer.hpp"

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING
	using int8 = integer<8>;
	using int64 = integer<64>;
	using int128 = integer<128>;

	int8 a, b;
	a = 1;
	b = 2;
	int8 c(3);
	//c = a + b;
//	cout << "sum: " << c << endl;

	int64 k;
	int128 m;
	cout << "Nr of bytes\n";
	cout << typeid(a).name() << "  size in bytes " << a.nrBytes << endl;
	cout << typeid(k).name() << "  size in bytes " << k.nrBytes << endl;
	cout << typeid(m).name() << "  size in bytes " << m.nrBytes << endl;

#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
