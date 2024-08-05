// multiplication.cpp: test suite runner for multiplication of doubledouble floating-point values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

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

	std::string test_suite         = "doubledouble multiplication validation";
	std::string test_tag           = "doubledouble multiplication";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	dd a, b, c, d;

	constexpr unsigned labelWidth = 40;

	a.assign("0.1");
	ReportValue(a, "0.1", labelWidth, 32);
	b.assign("10");
	ReportValue(b, "10.0", labelWidth, 32);
	c = a * b;
	ReportValue(c, "1.0", labelWidth, 32);
	std::cout << '\n';

	std::string _third("0.333333333333333333333333333333333");
	c.assign(_third);
	ReportValue(c, _third, labelWidth, 32);

	d = c;
	for (int i = 0; i < 53; ++i) {
		std::string value = std::string("0.33333... * ") + std::to_string(i) + std::string(" * 0.1");
		ReportValue(d, value, labelWidth, 32);
		d *= a;
	}

	d = c;;
	for (int i = 0; i < 53; ++i) {
		std::string value = std::string("0.33333... * ") + std::to_string(i) + std::string(" * 10.0");
		ReportValue(d, value, labelWidth, 32);
		d *= b;
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	constexpr unsigned nrOfRandoms = 1000;
	std::stringstream s;
	s << test_tag << " " << nrOfRandoms << " random pairs";
	std::string description = s.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<dd>(reportTestCases, RandomsOp::OPCODE_MUL, nrOfRandoms),
		description, 
		test_tag
	); 

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
