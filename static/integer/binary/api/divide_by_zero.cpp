// divide_by_zero.cpp: by default an integer division or remainder by zero throws integer_divide_by_zero
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// no exception switch is set: this test checks the defaults (#1894)
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(INTEGER_THROW_DIVIDE_BY_ZERO == 1, "an integer division by zero throws by default");
static_assert(INTEGER_THROW_ARITHMETIC_EXCEPTION == 0, "the WholeNumber/NaturalNumber domain errors keep their default");

namespace sw { namespace universal {

// every entry point that divides: the operators, reduce(), remainder() and idiv()
template<typename Integer>
int VerifyDivideByZeroThrows(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto expectThrow = [&](auto op, const char* label) {
		try {
			op();
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " did not throw\n";
		}
		catch (const integer_divide_by_zero&) {}
	};
	Integer seven(7), zero(0);
	expectThrow([&] { Integer r = seven / zero; (void)r; }, "7 / 0");
	expectThrow([&] { Integer r = seven % zero; (void)r; }, "7 % 0");
	expectThrow([&] { Integer r = zero / zero; (void)r; }, "0 / 0");
	expectThrow([&] { Integer r(seven); r /= zero; }, "7 /= 0");
	expectThrow([&] { Integer r(seven); r %= zero; }, "7 %= 0");
	expectThrow([&] { Integer q, r; q.reduce(seven, zero, r); }, "reduce(7, 0)");
	expectThrow([&] { Integer c; remainder(c, seven, zero); }, "remainder(7, 0)");
	expectThrow([&] { auto d = idiv(seven, zero); (void)d; }, "idiv(7, 0)");
	// a nonzero divisor is untouched
	if (seven / Integer(2) != Integer(3) || seven % Integer(2) != Integer(1)) {
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

	std::string test_suite  = "integer division by zero, default";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer<16, std::uint8_t> >(true), "integer<16,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// single limb of exactly nbits: the native-division paths
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer<  8, std::uint8_t > >(reportTestCases), "integer<8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer< 32, std::uint32_t> >(reportTestCases), "integer<32,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer< 16, std::uint16_t, IntegerNumberType::WholeNumber> >(reportTestCases), "integer<16,uint16_t,Whole>", test_tag);
	// several limbs: the long-division paths
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer< 16, std::uint8_t > >(reportTestCases), "integer<16,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer<128, std::uint32_t> >(reportTestCases), "integer<128,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows< integer<256, std::uint64_t> >(reportTestCases), "integer<256,uint64_t>", test_tag);
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
catch (const sw::universal::integer_arithmetic_exception& err) {
	std::cerr << "Caught unexpected integer arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::integer_internal_exception& err) {
	std::cerr << "Caught unexpected integer internal exception: " << err.what() << std::endl;
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
