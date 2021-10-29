// prefix.cpp test suite runner for prefix operators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(VerifyPrefix<3, 0>(bReportIndividualTestCases), "posit<3,0>", "prefix ++posit");

	nrOfFailedTestCases += ReportTestResult(VerifyPrefix<4, 0>(bReportIndividualTestCases), "posit<4,0>", "prefix ++posit");
	nrOfFailedTestCases += ReportTestResult(VerifyPrefix<4, 1>(bReportIndividualTestCases), "posit<4,1>", "prefix ++posit");

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
