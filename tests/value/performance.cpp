// performance.cpp: functional tests of the value type API
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "universal/bitblock/bitblock.hpp"
#include "universal/value/value.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// Performance benchmarks for value class
	cout << endl << "Performance benchmarks for value<> class" << endl;
	cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases" << endl;

#if MANUAL_TESTING
//	OperatorPerformance perfReport;
//	GeneratePerformanceReport<fbits>(perfReport);
//	ReportPerformance<nbits, es>(cout, "posit<8,0>", perfReport);

#else

	cout << "TBD" << endl;

#endif // MANUAL_TESTING

	if (nrOfFailedTestCases > 0) cout << "FAIL"; else cout << "PASS";

	cout.flush();
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
