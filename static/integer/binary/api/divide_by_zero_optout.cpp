// divide_by_zero_optout.cpp: with the exception opted out, an integer division or remainder by
// zero is 0
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// opt out through the older switch: INTEGER_THROW_DIVIDE_BY_ZERO follows it when not set itself (#1894)
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(INTEGER_THROW_DIVIDE_BY_ZERO == 0, "INTEGER_THROW_DIVIDE_BY_ZERO follows INTEGER_THROW_ARITHMETIC_EXCEPTION");

// the opted-out path is usable in constant evaluation, too
static_assert(sw::universal::integer<8, std::uint8_t>(7) / sw::universal::integer<8, std::uint8_t>(0) == 0, "7 / 0 is 0");
static_assert(sw::universal::integer<16, std::uint8_t>(7) / sw::universal::integer<16, std::uint8_t>(0) == 0, "7 / 0 is 0");
static_assert(sw::universal::integer<16, std::uint8_t>(7) % sw::universal::integer<16, std::uint8_t>(0) == 0, "7 % 0 is 0");

namespace sw { namespace universal {

// every entry point that divides: the operators, reduce(), remainder() and idiv()
template<typename Integer>
int VerifyDivideByZeroValue(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](const Integer& result, const char* label) {
		if (!result.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << result << " expected 0\n";
		}
	};
	Integer seven(7), zero(0);
	check(seven / zero, "7 / 0");
	check(seven % zero, "7 % 0");
	check(zero / zero, "0 / 0");
	Integer r(seven); r /= zero; check(r, "7 /= 0");
	r = seven; r %= zero; check(r, "7 %= 0");
	// reduce() and remainder() overwrite whatever their outputs held
	Integer q(5), rem(5);
	q.reduce(seven, zero, rem);
	check(q, "reduce(7, 0) quotient");
	check(rem, "reduce(7, 0) remainder");
	Integer c(5);
	remainder(c, seven, zero);
	check(c, "remainder(7, 0)");
	auto d = idiv(seven, zero);
	check(d.quot, "idiv(7, 0) quotient");
	check(d.rem, "idiv(7, 0) remainder");
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

	std::string test_suite  = "integer division by zero, exception opted out";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer<16, std::uint8_t> >(true), "integer<16,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// single limb of exactly nbits: the native-division paths
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer<  8, std::uint8_t > >(reportTestCases), "integer<8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer< 32, std::uint32_t> >(reportTestCases), "integer<32,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer< 16, std::uint16_t, IntegerNumberType::WholeNumber> >(reportTestCases), "integer<16,uint16_t,Whole>", test_tag);
	// several limbs: the long-division paths
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer< 16, std::uint8_t > >(reportTestCases), "integer<16,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer<128, std::uint32_t> >(reportTestCases), "integer<128,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< integer<256, std::uint64_t> >(reportTestCases), "integer<256,uint64_t>", test_tag);
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
