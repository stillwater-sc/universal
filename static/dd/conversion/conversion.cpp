// conversion.cpp: test suite runner for conversion operators for double-double (dd) floating-point
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

	std::string test_suite         = "doubledouble conversion validation";
	std::string test_tag           = "doubledouble conversion";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	uint64_t u64;
	int64_t i64;

	// check if we get all the bit of a 64-bit int
	u64 = 0xFFFF'FFFF'FFFF'FFFFull;
	i64 = 0x7FFF'FFFF'FFFF'FFFFll;

	{
		std::cout << to_binary(u64, false, 64) << " : " << u64 << '\n';
		u64 = -1;
		std::cout << to_binary(u64, false, 64) << " : " << u64 << '\n';
		uint64_t v{ u64 };
		double hi = static_cast<double>(v);
		uint64_t h = static_cast<uint64_t>(hi);
		std::cout << std::fixed << hi << '\n';
		std::cout << to_binary(h) << '\n';
		double lo = static_cast<double>(v - h);  // difference is always positive

		dd a(u64);
		ReportValue(a, "0xFFFF'FFFF'FFFF'FFFF", 35, 32);
		std::cout << to_pair(a) << '\n';
		uint64_t i = uint64_t(a);
		ReportValue(i, "0xFFFF'FFFF'FFFF'FFFF", 35, 32);
	}
	{
		dd a(i64);
		ReportValue(a, "0x7FFF'FFFF'FFFF'FFFF", 35, 32);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

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
