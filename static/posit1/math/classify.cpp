// classify.cpp: test suite runner for classification functions of the Reals specialized for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
	using std::isnormal;
	using std::isfinite;
	using std::isinf;
	using std::isnan;

	std::string test_suite  = "posit classification function validation";
	std::string test_tag    = "classification failed: ";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#define MY_DBL_MIN          2.2250738585072014e-308 // min positive value

	constexpr unsigned nbits = 32;
	constexpr unsigned es = 2;
	posit<nbits, es> pnar; pnar.setnar();
	posit<nbits, es> pinf; pinf.setnar();
	posit<nbits, es> pzero(0);
	posit<nbits, es> pminpos(SpecificValue::minpos);
	posit<nbits, es> pdblmin(MY_DBL_MIN);
	posit<nbits, es> pone(1);

	std::cout << std::boolalpha
		<< "isnormal(NaN) = " << isnormal(NAN) << '\n'
		<< "isnormal(Inf) = " << isnormal(INFINITY) << '\n'
		<< "isnormal(0.0) = " << isnormal(0.0) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << isnormal(MY_DBL_MIN / 2.0) << '\n'
		<< "isnormal(1.0) = " << isnormal(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnormal(NaR) = " << isnormal(pnar) << '\n'
		<< "isnormal(Inf) = " << isnormal(pinf) << '\n'
		<< "isnormal(0.0) = " << isnormal(pzero) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << isnormal(pdblmin / 2.0) << '\n'
		<< "isnormal(1.0) = " << isnormal(pone) << '\n';

	std::cout << std::boolalpha
		<< "isfinite(NaN) = " << isfinite(NAN) << '\n'
		<< "isfinite(Inf) = " << isfinite(INFINITY) << '\n'
		<< "isfinite(0.0) = " << isfinite(0.0) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << isfinite(MY_DBL_MIN / 2.0) << '\n'
		<< "isfinite(1.0) = " << isfinite(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isfinite(NaR) = " << isfinite(pnar) << '\n'
		<< "isfinite(Inf) = " << isfinite(pinf) << '\n'
		<< "isfinite(0.0) = " << isfinite(pzero) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << isfinite(pdblmin / 2.0) << '\n'
		<< "isfinite(1.0) = " << isfinite(pone) << '\n';

	std::cout << std::boolalpha
		<< "isinf(NaN) = " << isinf(NAN) << '\n'
		<< "isinf(Inf) = " << isinf(INFINITY) << '\n'
		<< "isinf(0.0) = " << isinf(0.0) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << isinf(MY_DBL_MIN / 2.0) << '\n'
		<< "isinf(1.0) = " << isinf(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isinf(NaR) = " << isinf(pnar) << '\n'
		<< "isinf(Inf) = " << isinf(pinf) << '\n'
		<< "isinf(0.0) = " << isinf(pzero) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << isinf(pdblmin / 2.0) << '\n'
		<< "isinf(1.0) = " << isinf(pone) << '\n';

	std::cout << std::boolalpha
		<< "isnan(NaN) = " << isnan(NAN) << '\n'
		<< "isnan(Inf) = " << isnan(INFINITY) << '\n'
		<< "isnan(0.0) = " << isnan(0.0) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << isnan(MY_DBL_MIN / 2.0) << '\n'
		<< "isnan(1.0) = " << isnan(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnan(NaR) = " << isnan(pnar) << '\n'
		<< "isnan(Inf) = " << isnan(pinf) << '\n'
		<< "isnan(0.0) = " << isnan(pzero) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << isnan(pdblmin / 2.0) << '\n'
		<< "isnan(1.0) = " << isnan(pone) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else


#if REGRESSION_LEVEL_1

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
