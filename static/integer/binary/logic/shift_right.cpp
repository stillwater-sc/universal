// shift_right.cpp : test runner for arithmetic and logic shift of fixed-sized, arbitrary precision integers
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
#include <universal/number/integer/numeric_limits.hpp>
// is representable
#include <universal/math/functions/isrepresentable.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

// enumerate all shift right cases for an integer<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyArithmeticRightShift(bool reportTestCases) {
	using namespace sw::universal;
	using Integer = integer<nbits, BlockType>;

	if (reportTestCases) std::cout << type_tag(Integer()) << '\n';

	// take maxneg and shift it right in all possible strides
	int nrOfFailedTests = 0;
	Integer a, result;
	Integer mostNegative(SpecificValue::maxneg);
	int64_t shiftRef, resultRef;
	for (size_t i = 0; i < nbits + 1; i++) {
		a = mostNegative;
		int64_t denominator = 0;
		if (i == 64) {
			shiftRef = 0;
		}
		else if (i == 63) { // special case for int64_t shift as it is maxneg
			shiftRef = -1;
		}
		else { // i < 63
			denominator = (1ll << i);
			shiftRef = ((long long)a / denominator);
		}

		result = a >> long(i);
		resultRef = (long long)result;

		if (shiftRef != resultRef) {
			nrOfFailedTests++;
			if (reportTestCases) ReportArithmeticShiftError("FAIL", ">>", a, i, result, resultRef);
		}
		else {
			if (reportTestCases) ReportArithmeticShiftSuccess("PASS", ">>", a, i, result, resultRef);
		}
		if (nrOfFailedTests > 99) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

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

	std::string test_suite  = "Integer arithmetic/logic shift right verfication";
	std::string test_tag    = "shift right";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// TODO: verifcation routine doesn't support integers bigger > 64bits
	// nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift< 71, uint8_t>(reportTestCases), "integer< 71,uint8_t>", test_tag);

	using Integer = integer<16, uint16_t>;
	constexpr Integer a(SpecificValue::maxpos), b(SpecificValue::maxneg);
	int i = int(b);
	std::cout << i << '\n';
	std::cout << b << '\n';
	Integer c;
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(c) << '\n';

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 4, uint8_t>(reportTestCases), "integer< 4, uint8_t >", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<  8, uint8_t>(reportTestCases), "integer<  8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift< 12, uint8_t>(reportTestCases), "integer< 12,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift< 19, uint8_t>(reportTestCases), "integer< 19,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift< 33, uint8_t>(reportTestCases), "integer< 33,uint8_t>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	
#endif

#if REGRESSION_LEVEL_3
	
#endif

#if	REGRESSION_LEVEL_4
	// verification suite does not support integers and shifts bigger than 64
	//nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift< 71, uint8_t>(reportTestCases), "integer< 71,uint8_t>", test_tag);
	//nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRightShift<123, uint8_t>(reportTestCases), "integer<123,uint8_t>", test_tag);
	integer<71, uint8_t, IntegerNumberType::IntegerNumber> a{ -1 };
	a >>= 7;
	if (a != -1) {
		std::cerr << "integer<71> arithmetic right shift failed: " << to_hex(a) << " : " << a << " != -1\n";
		++nrOfFailedTestCases;
	}

	// arithmetic right shift of a native int32_t as reference
	int32_t b{ -1 };
	std::cout << to_hex(b, true) << " : " << b << '\n';
	b >>= 7;
	std::cout << to_hex(b, true) << " : " << b << '\n';

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
