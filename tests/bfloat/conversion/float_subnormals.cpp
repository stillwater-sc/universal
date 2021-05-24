// float_subnormals.cpp: test suite runner for conversion tests of float subnormals to bfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>


#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases = 0;

#else
	cout << "Arbitrary Real addition validation" << endl;

	bool bReportIndividualTestCases = false;
	std::string tag = "float subnormal conversion failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "bfloat<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "bfloat<8,4>", "addition");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_divide_by_zero& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
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
