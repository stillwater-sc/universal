// integer_conversion.cpp: test suite runner for integer conversions to/from classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION 0
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a regression hierarchy
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	constexpr bool hasSubnormals   = true;
	constexpr bool hasMaxExpValues = true;
	constexpr bool isSaturating    = false;

	std::string test_suite = "cfloat integer conversion validation";
	std::string test_tag   = "integer conversion";
	bool reportTestCases   = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		// Ad-hoc reproduction of the three original bugs
		using E4M3 = cfloat<8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		E4M3 a;
		// Bug 1: sticky bit (101 should round to 104, not 96)
		a = 101;
		std::cout << "E4M3(101) = " << double(a) << " " << to_binary(a) << " (expected 104)\n";
		// Bug 2: rounding overflow stale bits (31 should round to 32, not 48)
		a = 31;
		std::cout << "E4M3(31)  = " << double(a) << " " << to_binary(a) << " (expected 32)\n";
		// Bug 3: no overflow guard (1000 should be inf)
		a = 1000;
		std::cout << "E4M3(1000)= " << double(a) << " " << to_binary(a) << " isinf=" << a.isinf() << "\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else //!MANUAL_TESTING

#if REGRESSION_LEVEL_1
	// ---------------------------------------------------------------
	// Integer -> cfloat: exhaustive via VerifyInteger2CfloatConversion
	// Uses RefType (nbits+1, es) to enumerate midpoints and test
	// rounding for every integer in the representable range.
	// ---------------------------------------------------------------

	// 8-bit cfloats: all es configurations with at least 1 fraction bit
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 2> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 3> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 4> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 5> int2cfloat");

	// 10-bit and 12-bit cfloats
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<10, 3, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<10, 3> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<10, 4, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<10, 4> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<12, 4, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<12, 4> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<12, 5, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<12, 5> int2cfloat");

	// Saturating variants (no hasMaxExpValues so maxpos is finite)
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 4, uint8_t, hasSubnormals, false, true> >(reportTestCases),
		test_tag, "cfloat< 8, 4, sat> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 5, uint8_t, hasSubnormals, false, true> >(reportTestCases),
		test_tag, "cfloat< 8, 5, sat> int2cfloat");

	// ---------------------------------------------------------------
	// cfloat -> integer: exhaustive via VerifyCfloat2IntegerConversion
	// Enumerates all 2^nbits encodings, verifies int(cfloat) ==
	// int(double(cfloat)).
	// ---------------------------------------------------------------

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 2> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 3> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 4> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 5> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat<10, 3, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<10, 3> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat<10, 4, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<10, 4> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat<12, 4, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<12, 4> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat<12, 5, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<12, 5> cfloat2int");

#endif

#if REGRESSION_LEVEL_2
	// 16-bit cfloats: larger integer ranges
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<16, 5, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<16, 5> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<16, 7, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<16, 7> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<16, 8, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<16, 8> int2cfloat");

	// 16-bit cfloat -> integer (full 64K encoding enumeration)
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat<16, 5, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<16, 5> cfloat2int");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloat2IntegerConversion< cfloat<16, 8, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<16, 8> cfloat2int");

#endif

#if REGRESSION_LEVEL_3
	// No-subnormal variants
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat< 8, 4, uint8_t, false, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat< 8, 4, nosub> int2cfloat");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInteger2CfloatConversion< cfloat<12, 5, uint16_t, false, hasMaxExpValues, isSaturating> >(reportTestCases),
		test_tag, "cfloat<12, 5, nosub> int2cfloat");

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
