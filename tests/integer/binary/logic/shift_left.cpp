// shift_left.cpp : test runner for arithmetic and logic shift of abitrary precision fixed-size integers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

// enumerate all shift left cases for an integer<nbits, BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyLeftShift(bool reportTestCases) {
	using namespace sw::universal;
	using Integer = integer<nbits, BlockType>;

	if (reportTestCases) std::cout << type_tag(Integer()) << '\n';

	// take 1 and shift it left in all possible strides
	int nrOfFailedTests = 0;
	Integer a, result;
	int64_t shiftRef, resultRef;
	for (size_t i = 0; i < nbits + 1; i++) {
		shiftRef = (-1ll << i);
		if (i == nbits) shiftRef = 0; // shift all bits out

		a = -1;
		result = a << long(i);
		resultRef = (long long)result;

		if (shiftRef != resultRef) {
			nrOfFailedTests++;
			if (reportTestCases) ReportArithmeticShiftError("FAIL", "<<", a, i, result, resultRef);
		}
		else {
			if (reportTestCases) ReportArithmeticShiftSuccess("PASS", "<<", a, i, result, resultRef);
		}
		if (nrOfFailedTests > 100) return nrOfFailedTests;
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

	std::string test_suite  = "Integer arithmetic/logic shift left verfication";
	std::string test_tag    = "shift left";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

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
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift<  8, uint8_t>(reportTestCases), "integer<  8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 12, uint8_t>(reportTestCases), "integer< 12,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 19, uint8_t>(reportTestCases), "integer< 19,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLeftShift< 33, uint8_t>(reportTestCases), "integer< 33,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	
#endif

#if REGRESSION_LEVEL_3
	
#endif

#if	REGRESSION_LEVEL_4
	
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
