// exceptions.cpp: lindex arithmetic exceptions under LINDEX_THROW_ARITHMETIC_EXCEPTION (#1695)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Closure (Lozier and Olver 1990) leaves division by zero as the one arithmetic error of
// level-index.  With the guard set, x / 0 throws lindex_divide_by_zero instead of giving NaR.
#include <universal/utility/directives.hpp>
#define LINDEX_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

template<typename L>
int VerifyDivideByZero(bool reportTestCases) {
	int fails = 0;
	auto expect_throw = [&](auto f, const char* what) {
		bool thrown = false;
		try { f(); }
		catch (const lindex_divide_by_zero&) { thrown = true; }
		if (!thrown) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(L()) << ' ' << what << " did not throw\n"; }
	};
	expect_throw([]() { L a(2.0), z(0); return a / z; }, "x / 0");
	expect_throw([]() { L z(0); return z.reciprocal(); }, "reciprocal(0)");
	expect_throw([]() { L a(-3.0); return a /= 0.0; }, "x /= 0.0");
	// the operations closure covers do not throw, and NaR still propagates without throwing
	try {
		L a(SpecificValue::maxpos), n(SpecificValue::nar), z(0);
		const L s = a * a + a;
		const L q = n / z;
		if (s != a || !q.isnar()) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(L()) << " saturation or NaR\n"; }
	}
	catch (...) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL: " << type_tag(L()) << " threw outside division by zero\n";
	}
	return fails;
}

}  // anonymous namespace

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

	std::string test_suite  = "lindex arithmetic exceptions (#1695)";
	std::string test_tag    = "exceptions";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero<lindex<16, 3>>(reportTestCases), "lindex<16,3>", "divide by zero");
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero<lindex<64, 3>>(reportTestCases), "lindex<64,3>", "divide by zero");
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
