// performance.cpp: functional tests of the value type API
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/bitblock/bitblock.hpp>
#include <universal/value/value.hpp>
#include <universal/performance/number_system.hpp>

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
	{
		constexpr size_t fbits = 22;
		value<fbits> number{ 1 };
//		OperatorPerformance perfReport;
//		GeneratePerformanceReport(number, perfReport);
//		auto report = ReportPerformance(number, perfReport);
//		cout << report << endl;
	}


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
