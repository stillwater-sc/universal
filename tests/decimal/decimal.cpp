//  decimal.cpp : test suite for abitrary precision decimal integers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include "../../decimal/decimal.hpp"

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Decimal Arithmetic tests failed";

#if MANUAL_TESTING
	decimal d1, d2, d3;

	//cin >> d2;
	//cout << d2 << endl;
	
	std::string val = "1234567890";
	if (!d1.parse(val)) {
		cerr << "failed to parse the decimal value -" << val << "-\n";
	}
	cout << d1 << endl;

	val = "-123";
	if (!d2.parse(val)) {
		cerr << "failed to parse the decimal value -" << val << "-\n";
	}
	cout << d2 << endl;

	val = "+123";
	if (!d3.parse(val)) {
		cerr << "failed to parse the decimal value -" << val << "-\n";
	}
	cout << d3 << endl;



#else
	std::cout << "Decimal Arithmetic verfication" << std::endl;

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
