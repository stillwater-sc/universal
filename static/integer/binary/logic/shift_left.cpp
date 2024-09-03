// shift_left.cpp : test runner for arithmetic and logic shift of fixed-sized, arbitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw { namespace universal {

	// enumerate all shift left cases for an integer<nbits, BlockType> configuration
	template<unsigned nbits, typename BlockType = std::uint8_t, IntegerNumberType NumberType = IntegerNumberType::IntegerNumber>
	int VerifyLeftShift(bool reportTestCases) {
		using namespace sw::universal;
		using Integer = integer<nbits, BlockType>;

		if (reportTestCases) std::cout << type_tag(Integer()) << '\n';

		// take 1 and shift it left in all possible strides
		int nrOfFailedTests = 0;
		Integer a, result, ref;
		uint64_t shiftRef;
		for (unsigned i = 0; i < nbits + 1u; i++) {
			shiftRef = (~0ull << i);
			if (i == nbits) shiftRef = 0; // shift all bits out
			ref.setbits(shiftRef);

			a = -1;
			result = a << static_cast<int>(i);

			if (ref != result) {
				nrOfFailedTests++;
				if (reportTestCases) ReportArithmeticShiftError("FAIL", "<<", a, i, result, shiftRef);
			}
			else {
				if (reportTestCases) ReportArithmeticShiftSuccess("PASS", "<<", a, i, result, shiftRef);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		return nrOfFailedTests;
	}

} } // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite  = "Integer arithmetic/logic shift left verfication";
	std::string test_tag    = "shift left";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Integer = integer<12, uint8_t, IntegerNumberType::IntegerNumber>;
		Integer a(1);
		std::cout << to_binary(a) << " : " << a << '\n';
		a <<= 11;
		std::cout << to_binary(a) << " : " << a << '\n';
		a <<= 1;
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	{
		using Integer = integer<16, uint16_t>;
		constexpr Integer a(SpecificValue::maxpos), b(SpecificValue::maxneg);
		int i = int(b);
		std::cout << i << '\n';
		std::cout << b << '\n';
		Integer c;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(c) << '\n';
	}


	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 4, uint8_t>(reportTestCases), "integer< 4, uint8_t >", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	// VerifyLeftShift uses a (long long) as reference type, so we can only test up to 64bit integer<> types
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  8, uint8_t>(reportTestCases), "integer<  8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 12, uint8_t>(reportTestCases), "integer< 12,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 19, uint8_t>(reportTestCases), "integer< 19,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 33, uint8_t>(reportTestCases), "integer< 33,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 39, uint8_t>(reportTestCases), "integer< 39,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 63, uint8_t>(reportTestCases), "integer< 63,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  9, uint8_t>(reportTestCases), "integer<  9,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 13, uint8_t>(reportTestCases), "integer< 13,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 20, uint8_t>(reportTestCases), "integer< 20,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 31, uint8_t>(reportTestCases), "integer< 31,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 41, uint16_t>(reportTestCases), "integer< 41,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 53, uint16_t>(reportTestCases), "integer< 53,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 63, uint32_t>(reportTestCases), "integer< 63,uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  9, uint8_t>(reportTestCases), "integer<  9,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 13, uint8_t>(reportTestCases), "integer< 13,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 20, uint8_t>(reportTestCases), "integer< 20,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 31, uint8_t>(reportTestCases), "integer< 31,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 41, uint8_t>(reportTestCases), "integer< 41,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 53, uint8_t>(reportTestCases), "integer< 53,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 63, uint8_t>(reportTestCases), "integer< 63,uint32_t>", test_tag);
#endif

#if	REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  4, uint8_t>(reportTestCases), "integer<  4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  5, uint8_t>(reportTestCases), "integer<  5,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  6, uint8_t>(reportTestCases), "integer<  6,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  7, uint8_t>(reportTestCases), "integer<  7,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  8, uint8_t>(reportTestCases), "integer<  8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  9, uint8_t>(reportTestCases), "integer<  9,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 10, uint8_t>(reportTestCases), "integer< 10,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 11, uint8_t>(reportTestCases), "integer< 11,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 12, uint8_t>(reportTestCases), "integer< 12,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 13, uint8_t>(reportTestCases), "integer< 13,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 14, uint8_t>(reportTestCases), "integer< 14,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 15, uint8_t>(reportTestCases), "integer< 15,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 16, uint8_t>(reportTestCases), "integer< 16,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 17, uint8_t>(reportTestCases), "integer< 17,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 18, uint8_t>(reportTestCases), "integer< 18,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 19, uint8_t>(reportTestCases), "integer< 19,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 20, uint8_t>(reportTestCases), "integer< 20,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 21, uint8_t>(reportTestCases), "integer< 21,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 22, uint8_t>(reportTestCases), "integer< 22,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 23, uint8_t>(reportTestCases), "integer< 23,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 24, uint8_t>(reportTestCases), "integer< 24,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 25, uint8_t>(reportTestCases), "integer< 25,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 26, uint8_t>(reportTestCases), "integer< 26,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 27, uint8_t>(reportTestCases), "integer< 27,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 28, uint8_t>(reportTestCases), "integer< 28,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 29, uint8_t>(reportTestCases), "integer< 29,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 30, uint8_t>(reportTestCases), "integer< 30,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 31, uint8_t>(reportTestCases), "integer< 31,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 32, uint8_t>(reportTestCases), "integer< 32,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 33, uint8_t>(reportTestCases), "integer< 33,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 34, uint8_t>(reportTestCases), "integer< 34,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 35, uint8_t>(reportTestCases), "integer< 35,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 36, uint8_t>(reportTestCases), "integer< 36,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 37, uint8_t>(reportTestCases), "integer< 37,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 38, uint8_t>(reportTestCases), "integer< 38,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 39, uint8_t>(reportTestCases), "integer< 39,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 40, uint8_t>(reportTestCases), "integer< 40,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 41, uint8_t>(reportTestCases), "integer< 41,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 42, uint8_t>(reportTestCases), "integer< 42,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 43, uint8_t>(reportTestCases), "integer< 43,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 44, uint8_t>(reportTestCases), "integer< 44,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 45, uint8_t>(reportTestCases), "integer< 45,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 46, uint8_t>(reportTestCases), "integer< 46,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 47, uint8_t>(reportTestCases), "integer< 47,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 48, uint8_t>(reportTestCases), "integer< 48,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 49, uint8_t>(reportTestCases), "integer< 49,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 50, uint8_t>(reportTestCases), "integer< 50,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 51, uint8_t>(reportTestCases), "integer< 51,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 52, uint8_t>(reportTestCases), "integer< 52,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 53, uint8_t>(reportTestCases), "integer< 53,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 54, uint8_t>(reportTestCases), "integer< 54,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 55, uint8_t>(reportTestCases), "integer< 55,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 56, uint8_t>(reportTestCases), "integer< 56,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 57, uint8_t>(reportTestCases), "integer< 57,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 58, uint8_t>(reportTestCases), "integer< 58,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 59, uint8_t>(reportTestCases), "integer< 59,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 60, uint8_t>(reportTestCases), "integer< 60,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 61, uint8_t>(reportTestCases), "integer< 61,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 62, uint8_t>(reportTestCases), "integer< 62,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 63, uint8_t>(reportTestCases), "integer< 63,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 64, uint8_t>(reportTestCases), "integer< 64,uint8_t>", test_tag);
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
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
