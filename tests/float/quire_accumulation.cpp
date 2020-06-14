//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files
#include "universal/posit/exceptions.hpp"
#include "universal/bitblock/bitblock.hpp"
#include "universal/value/value.hpp"
#include "universal/float/quire.hpp"

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::ieee;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation";

#if MANUAL_TESTING

#else

	cout << "IEEE Floating Point Quire experiments" << endl;
	
	cout << "\nTBD\n";

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
