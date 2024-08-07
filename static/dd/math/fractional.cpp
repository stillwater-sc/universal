// fractional.cpp: test suite runner for fractional functions for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <numbers>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>

// generate specific test case
template<typename Ty>
void GenerateTestCase(Ty fa, Ty fb) {
	unsigned precision = 25;
	unsigned width = 30;
	Ty fref;
	sw::universal::dd a, b, ref, v;
	a = fa;
	b = fb;
	fref = std::remainder(fa, fb);
	ref = fref;
	v = sw::universal::remainder(a, b);
	auto oldPrec = std::cout.precision();
	std::cout << std::setprecision(precision);
	std::cout << " -> remainder(" << fa << "," << fb << ") = " << std::setw(width) << fref << std::endl;
	std::cout << " -> remainder( " << a << "," << b << ")  = " << v << '\n' << to_binary(v) << '\n';
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
	using std::fmod;

	std::string test_suite  = "doubledouble mathlib fractional function validation";
	std::string test_tag    = "fractional";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	double a{ 1.5 }, b{ 1.25 };
	dd da(a), db(b);

	std::cout << "fmod( " << a << ", " << b << ") = " << fmod(a, b) << '\n';
	std::cout << "fmod( " << da << ", " << db << ") = " << fmod(da, db) << '\n';

	std::cout << "remainder( " << a << ", " << b << ") = " << remainder(a, b) << '\n';
	std::cout << "remainder( " << da << ", " << db << ") = " << remainder(da, db) << '\n';

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
