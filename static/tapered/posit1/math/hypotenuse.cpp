// hypotenuse.cpp: test suite runner for the hypotenuse functions (hypot, hypotf, hypotl)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// when you define ALGORITHM_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define ALGORITHM_VERBOSE_OUTPUT
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	sw::universal::posit<nbits, es> a, b, pref, result;
	a = _a;
	b = _b;
	Ty ref = std::hypot(_a, _b);
	pref = ref;
	result = sw::universal::hypot(a, b);
	std::cout << std::setprecision(nbits - 2);
	std::cout << " hypot(" << _a << ", " << _b << ") = " << ref << '\n';
	std::cout << " hypot(" <<  a << ", " <<  b << ") = " << result << " : " << to_binary(result) << " (reference: " << to_binary(pref) << ")   ";
	std::cout << (pref == result ? "PASS" : "FAIL") << "\n\n";
	std::cout << std::setprecision(5);
}


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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit hypotenuse validation";
	std::string test_tag    = "hypotenuse";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase< 8, 2, float>(3.0f, 4.0f);
	GenerateTestCase<16, 2, float>(3.0f, 4.0f);

	nrOfFailedTestCases += ReportTestResult(VerifyHypot<posit<2, 0>(reportTestCases), "posit<2,0>", "hypot");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyHypot<posit<4, 0>>(reportTestCases), "posit<4,0>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot<posit<4, 1>>(reportTestCases), "posit<4,1>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot<posit<5, 2>>(reportTestCases), "posit<5,2>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot<posit<6, 2>>(reportTestCases), "posit<6,2>", "hypot");

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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
