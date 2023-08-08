//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation";

#if MANUAL_TESTING

#else

	std::cout << "IEEE-754 Floating-Point Quire experiments\n";
	
	std::cout << "\nTBD\n";

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
