// assignment.cpp : test suite runner for native type literal assignments for posit2
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>

namespace sw { namespace universal {

	template<size_t nbits, size_t es, typename Ty>
	int VerifyAssignment(bool reportTestCases) {
		const size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTestCases = 0;

		// use only valid posit values
		// posit_raw -> to value in Ty -> assign to posit -> compare posits
		sw::universal::posit<nbits, es> p, assigned;
		for (size_t i = 0; i < NR_POSITS; i++) {
			p.setbits(i);
			if (p.isnar() && std::numeric_limits<Ty>::is_exact) continue; // can't assign NaR for integer types
			Ty value = (Ty)(p);
			assigned = value;
			if (p != assigned) {
				nrOfFailedTestCases++;
				if (reportTestCases) ReportAssignmentError("FAIL", "=", p, assigned, value);
			}
			else {
				//if (reportTestCases) ReportAssignmentSuccess("PASS", "=", p, assigned, value);
			}
		}
		return nrOfFailedTestCases;
	}

} } // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit2 assignment validation";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 1, float>(reportTestCases), test_tag, "posit<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 2, float>(reportTestCases), test_tag, "posit<6,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<3, 0, float>(reportTestCases), test_tag, "posit<3,0>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 0, float>(reportTestCases), test_tag, "posit<4,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 1, float>(reportTestCases), test_tag, "posit<4,1>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 0, float>(reportTestCases), test_tag, "posit<5,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 1, float>(reportTestCases), test_tag, "posit<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 2, float>(reportTestCases), test_tag, "posit<5,2>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 0, float>(reportTestCases), test_tag, "posit<6,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 1, float>(reportTestCases), test_tag, "posit<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 2, float>(reportTestCases), test_tag, "posit<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 3, float>(reportTestCases), test_tag, "posit<6,3>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 0, float>(reportTestCases), test_tag, "posit<7,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 1, float>(reportTestCases), test_tag, "posit<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 2, float>(reportTestCases), test_tag, "posit<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 3, float>(reportTestCases), test_tag, "posit<7,3>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 0, float>(reportTestCases), test_tag, "posit<8,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 1, float>(reportTestCases), test_tag, "posit<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 2, float>(reportTestCases), test_tag, "posit<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 3, float>(reportTestCases), test_tag, "posit<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 4, float>(reportTestCases), test_tag, "posit<8,4>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 0, float>(reportTestCases), test_tag, "posit<9,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 1, float>(reportTestCases), test_tag, "posit<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 2, float>(reportTestCases), test_tag, "posit<9,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 3, float>(reportTestCases), test_tag, "posit<9,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 4, float>(reportTestCases), test_tag, "posit<9,4>");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
