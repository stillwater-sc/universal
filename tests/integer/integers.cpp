//  integers.cpp : test suite for abitrary precision integers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "universal/integer/integer.hpp"

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;


	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING
	using int8 = integer<8>;
	using int64 = integer<64>;
	using int128 = integer<128>;

	int8 a;
	int64 k;
	int128 m;
	cout << "Nr of bytes\n";
	cout << typeid(a).name() << "  size in bytes " << a.nrBytes << endl;
	cout << typeid(k).name() << "  size in bytes " << k.nrBytes << endl;
	cout << typeid(m).name() << "  size in bytes " << m.nrBytes << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// allocation is the only functionality of integer<N> at this time

	// TODO: implement parsing, assigment, conversion, arithmetic

#ifdef STRESS_TESTING


#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
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
