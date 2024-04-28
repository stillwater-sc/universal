//  intervals.cpp : demonstration of valid intervals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// pull in the valid number system
#include <universal/number/valid/valid.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::cout << "Valid use cases\n";


#if MANUAL_TESTING


	std::cout << std::endl;

#else

	std::cout << "Valid interval validation" << std::endl;

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
