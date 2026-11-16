// overflow_throw.cpp: with QUIRE_THROW_ARITHMETIC_EXCEPTION, a sum past the accumulator throws quire_overflow
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// As decided on #2094: when quire exceptions are enabled, the carry out of the accumulator's top bit throws
// quire_overflow, through a product added to the quire and through a quire added to a quire. Without them
// the quire records the overflow instead (overflow.cpp).
#include <universal/utility/directives.hpp>
#define QUIRE_THROW_ARITHMETIC_EXCEPTION 1
#include <cmath>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// f() must throw quire_overflow
template<typename F>
int ExpectOverflow(F&& f, const char* what, bool reportTestCases) {
	try {
		f();
	}
	catch (const sw::universal::quire_overflow&) {
		return 0;
	}
	catch (...) {
		if (reportTestCases) std::cerr << "FAIL: " << what << " threw something other than quire_overflow\n";
		return 1;
	}
	if (reportTestCases) std::cerr << "FAIL: " << what << " did not throw\n";
	return 1;
}

int VerifyThrows(bool reportTestCases) {
	using namespace sw::universal;
	using C = cfloat<4, 2, uint8_t, true, false, false>;
	using P = posit<8, 0>;
	int fails = 0;
	fails += ExpectOverflow([] {
		quire<C, 0> q;
		q += quire_mul(C(3), C(3));
		q += quire_mul(C(3), C(3));   // 18 is past the 6-bit accumulator
	}, "cfloat<4,2> 3*3 + 3*3", reportTestCases);
	fails += ExpectOverflow([] {
		quire<P, 0> q;
		q = std::ldexp(1.0, q.max_scale());
		const quire<P, 0> same(q);
		q += same;                    // 2^top + 2^top
	}, "posit<8,0> quire + quire", reportTestCases);
	// a sum that stays inside the accumulator does not throw
	try {
		quire<C, 0> q;
		q += quire_mul(C(1), C(1));
		q += quire_mul(C(1), C(1));
		if (!(quire_resolve(q) == C(2))) ++fails;
	}
	catch (...) {
		if (reportTestCases) std::cerr << "FAIL: an in-range sum threw\n";
		++fails;
	}
	return fails;
}

}  // namespace

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

	std::string test_suite  = "quire overflow exception";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyThrows(reportTestCases), "quire", "throws quire_overflow");
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
