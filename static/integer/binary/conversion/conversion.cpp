//  conversion.cpp : test suite runner for conversion of abitrary fixed precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
// configure the integer arithmetic class
// we need to enable exceptions to validate divide by zero and overflow conditions
// however, we also need to make this work with exceptions turned off: TODO
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/integer_test_suite.hpp>

#include <universal/native/integers.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw { namespace universal {

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
int VerifyToIntegerConversion(bool reportTestCases) {
	using Integer = integer<nbits, BlockType, NumberType>;

	int nrOfFailedTests = 0;

	Integer a;
	a.setbits(0xAAAA'AAAA'AAAA'AAAA);
	if (reportTestCases) std::cout << type_tag(a) << '\n';

	uint8_t  ua8  = uint8_t(a);
	uint16_t ua16 = uint16_t(a);
	uint32_t ua32 = uint32_t(a);
	uint64_t ua64 = uint64_t(a);
	std::cout << "ua8   : " << to_binary(ua8, true, sizeof(ua8) * 8) << std::setw(95) << (long long)(ua8) << '\n';
	std::cout << "ua16  : " << to_binary(ua16, true, sizeof(ua16) * 8) << std::setw(85) << (long long)(ua16) << '\n';
	std::cout << "ua32  : " << to_binary(ua32, true, sizeof(ua32) * 8) << std::setw(65) << (long long)(ua32) << '\n';
	std::cout << "ua64  : " << to_binary(ua64, true, sizeof(ua64) * 8) << std::setw(25) << (long long)(ua64) << '\n';

	int8_t ia8   = int8_t(a);
	int16_t ia16 = int16_t(a);
	int32_t ia32 = int32_t(a);
	int64_t ia64 = int64_t(a);
	std::cout << "ia8   : " << to_binary(ia8, true, sizeof(ia8) * 8) << std::setw(95) << (long long)(ia8) << '\n';
	std::cout << "ia16  : " << to_binary(ia16, true, sizeof(ia16) * 8) << std::setw(85) << (long long)(ia16) << '\n';
	std::cout << "ia32  : " << to_binary(ia32, true, sizeof(ia32) * 8) << std::setw(65) << (long long)(ia32) << '\n';
	std::cout << "ia64  : " << to_binary(ia64, true, sizeof(ia64) * 8) << std::setw(25) << (long long)(ia64) << '\n';

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

	std::string test_suite  = "Integer conversion verfication";
	std::string test_tag    = "integer<> conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		int8_t ia8 = static_cast<int8_t>(0xAA);
		uint8_t ua8 = static_cast<uint8_t>(ia8);
		uint8_t ua16 = static_cast<uint16_t>(ia8);
		uint64_t ua64 = 0xAAAA'AAAA'AAAA'AAAA;
		std::cout << to_binary(ia8, 8, true) << " : " << long(ia8) << '\n';
		std::cout << to_binary(ua8, 8, true) << " : " << long(ua8) << '\n';
		std::cout << to_binary(ua16, 16, true) << " : " << long(ua16) << '\n';
		std::cout << to_binary(ua64, 64, true) << " : " << (long long)(ua64) << '\n';

		VerifyToIntegerConversion< 8, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<12, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<16, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<24, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<32, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<48, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<64, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion<79, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases);
		VerifyToIntegerConversion< 8, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<12, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<16, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<24, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<32, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<48, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<64, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
		VerifyToIntegerConversion<79, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases);
	}

	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer< 8, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<64, uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer< 8, uint8_t, WholeNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<16, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<16, uint8_t, WholeNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<32, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<32, uint8_t, WholeNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint8_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<64, uint8_t, WholeNumber>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint8_t, IntegerNumberType::NaturalNumber>(reportTestCases), "integer< 8, uint8_t, NaturalNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<16, uint8_t, IntegerNumberType::NaturalNumber>(reportTestCases), "integer<16, uint8_t, NaturalNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<32, uint8_t, IntegerNumberType::NaturalNumber>(reportTestCases), "integer<32, uint8_t, NaturalNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint8_t, IntegerNumberType::NaturalNumber>(reportTestCases), "integer<64, uint8_t, NaturalNumber>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases), "integer< 8, uint8_t, IntegerNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<16, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases), "integer<16, uint8_t, IntegerNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<32, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases), "integer<32, uint8_t, IntegerNumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases), "integer<64, uint8_t, IntegerNumber>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint16_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer< 8, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<16, uint16_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<16, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<32, uint16_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<32, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint16_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<64, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint32_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer< 8, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<16, uint32_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<16, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<32, uint32_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<32, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint32_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<64, uint32_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion< 8, uint64_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer< 8, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<16, uint64_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<16, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<32, uint64_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<32, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToIntegerConversion<64, uint64_t, IntegerNumberType::WholeNumber>(reportTestCases), "integer<64, uint64_t>", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
