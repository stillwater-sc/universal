// prefix.cpp functional tests for prefix operators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "universal/posit/posit.hpp"
#include "universal/posit/manipulators.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/posit_test_helpers.hpp"

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<3, 0>("Increment failed", bReportIndividualTestCases), "posit<3,0>", "prefix ++posit");

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 0>("Increment failed", bReportIndividualTestCases), "posit<4,0>", "prefix ++posit");
	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "prefix ++posit");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
