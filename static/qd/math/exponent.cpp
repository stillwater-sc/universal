// exponent.cpp: test suite runner for exponentiation function for quad-double (qd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

// generate specific test case
template<typename Ty>
void GenerateTestCase(Ty fa) {
	unsigned precision = 25;
	unsigned width = 30;
	Ty fref;
	sw::universal::qd a, ref, v;
	a = fa;
	fref = std::exp(fa);
	ref = fref;
	v = sw::universal::exp(a);
	auto oldPrec = std::cout.precision();
	std::cout << std::setprecision(precision);
	std::cout << " -> exp(" << fa << ") = " << std::setw(width) << fref << std::endl;
	std::cout << " -> exp( " << a << ")  = " << v << '\n' << to_binary(v) << '\n';
	std::cout << to_binary(ref) << "\n -> reference\n";
	std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(oldPrec);
}

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

	std::string test_suite  = "quad-double mathlib exponentiation function validation";
	std::string test_tag    = "exp/exp2/exp10/expm1";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase(4.0);

	auto oldPrec = std::cout.precision();
	for (int i = 0; i < 30; ++i) {
		std::string tag = "exp(" + std::to_string(i) + ")";
		double exponentRef = std::exp(double(i));
		qd exponent = exp(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	for (int i = 0; i < 30; ++i) {
		std::string tag = "exp2(" + std::to_string(i) + ")";
		double exponentRef = std::exp2(double(i));
		qd exponent = exp2(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	for (int i = 0; i < 30; ++i) {
		std::string tag = "exp10(" + std::to_string(i) + ")";
		double exponentRef = std::pow(10.0, double(i));
		qd exponent = exp10(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	for (int i = 0; i < 30; ++i) {
		std::string tag = "expm1(" + std::to_string(i) + ")";
		double exponentRef = std::expm1(double(i));
		qd exponent = expm1(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	std::cout << std::setprecision(oldPrec);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else


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
