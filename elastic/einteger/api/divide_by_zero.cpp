// divide_by_zero.cpp: by default an einteger division or remainder by zero throws einteger_divide_by_zero
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// no exception switch is set: this test checks the defaults (#1894)
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(EINTEGER_THROW_DIVIDE_BY_ZERO == 1, "a einteger division by zero throws by default");
static_assert(EINTEGER_THROW_ARITHMETIC_EXCEPTION == 0, "the other einteger arithmetic errors keep their default");

namespace sw { namespace universal {

template<typename EInteger>
int VerifyDivideByZeroThrows(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto expectThrow = [&](auto op, const char* label) {
		try {
			op();
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " did not throw\n";
		}
		catch (const einteger_divide_by_zero&) {}
	};
	EInteger seven(7), zero(0), big;
	big.assign("123456789012345678901234567890");  // several limbs
	expectThrow([&] { EInteger r = seven / zero; (void)r; }, "7 / 0");
	expectThrow([&] { EInteger r = seven % zero; (void)r; }, "7 % 0");
	expectThrow([&] { EInteger r = zero / zero; (void)r; }, "0 / 0");
	expectThrow([&] { EInteger r = big / zero; (void)r; }, "big / 0");
	expectThrow([&] { EInteger r(seven); r /= zero; }, "7 /= 0");
	expectThrow([&] { EInteger r(seven); r %= zero; }, "7 %= 0");
	expectThrow([&] { EInteger q, r; q.reduce(seven, zero, r); }, "reduce(7, 0)");
	// a zero held in more than one limb, which setblock() can build
	EInteger widezero; widezero.setblock(1, 0);
	expectThrow([&] { EInteger r = seven / widezero; (void)r; }, "7 / [0,0]");
	expectThrow([&] { EInteger r = seven % widezero; (void)r; }, "7 % [0,0]");
	if (seven / EInteger(2) != EInteger(3) || seven % EInteger(2) != EInteger(1)) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 7 / 2 or 7 % 2\n";
	}
	return nrOfFailedTests;
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

	std::string test_suite  = "einteger division by zero, default";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< einteger<std::uint32_t> >(true), "einteger", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< einteger<std::uint8_t> >(reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< einteger<std::uint16_t> >(reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< einteger<std::uint32_t> >(reportTestCases), "einteger<uint32_t>", test_tag);
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
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Caught unexpected einteger arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Caught unexpected einteger internal exception: " << err.what() << std::endl;
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
