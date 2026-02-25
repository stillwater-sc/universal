// truncate.cpp: test suite runner for truncation functions trunc, round, floor, and ceil
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite_mathlib.hpp>

namespace sw { namespace universal {

template<typename TestType>
int VerifyFloor(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;

	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		TestType l1 = floor(a);
		// generate the reference
		float f = float(a);         // we can stay with floats as the state space NR_VALUES is always going to be small to be practical (nbits < 16)
		float ff = std::floor(f);
		TestType l2 = ff;
		if (l1 != l2) {
			if (a.isnan() || l1.isnan()) continue;
			std::cout << to_binary(a) << " : " << a << '\n';
			std::cout << "floor(" << f << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("floor", "floor", a, l1, l2);
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyCeil(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;

	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		TestType l1 = ceil(a);
		// generate the reference
		float f = float(a);
		float cf = std::ceil(f);
		TestType l2 = cf;
		if (l1 != l2) {
			if (a.isnan() || l1.isnan()) continue;
			std::cout << to_binary(a) << " : " << a << '\n';
			std::cout << "ceil(" << f << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("ceil", "ceil", a, l1, l2);
		}
	}
	return nrOfFailedTestCases;
}

} }   // namespace sw::universal

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

	std::string test_suite  = "lnsfloat<> mathlib truncation validation";
	std::string test_tag    = "truncation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases = ReportTestResult(VerifyFloor< lns<8, 2, uint8_t> >(reportTestCases), "floor", "lns<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCeil < lns<8, 2, uint8_t> >(reportTestCases), "ceil ", "lns<8,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures in manual testing mode
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(VerifyFloor< lns<8, 2, uint8_t> >(reportTestCases), "floor", "lns<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCeil < lns<8, 2, uint8_t> >(reportTestCases), "ceil ", "lns<8,2>");

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
