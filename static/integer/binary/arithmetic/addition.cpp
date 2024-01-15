//  addition.cpp : test runner for addition operator on fixed-size abitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
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

#include <typeinfo>
template<typename Scalar>
void GenerateAddTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
	z = x + y;
	std::cout << typeid(Scalar).name() << ": " << x << " + " << y << " = " << z << std::endl;
}


// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateAddTest<short>(2, 16, s);
	sw::universal::integer<16> z = 0;
	GenerateAddTest<sw::universal::integer<16> >(2, 16, z);
}

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::universal::reportRepresentability(i, j);
		}
	}
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

std::string convert_to_string(const std::vector<char>& v) {
	std::stringstream ss;
	for (std::vector<char>::const_reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "Integer Arithmetic Addition verfication";
	std::string test_tag    = "integer<> addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

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
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 4, uint8_t>(reportTestCases), "integer< 4, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 6, uint8_t>(reportTestCases), "integer< 6, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 8, uint8_t>(reportTestCases), "integer< 8, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 9, uint8_t >(reportTestCases), "integer< 9, uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< 9, uint16_t>(reportTestCases), "integer< 9, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint8_t >(reportTestCases), "integer<11, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint16_t>(reportTestCases), "integer<11, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint8_t >(reportTestCases), "integer<12, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint16_t>(reportTestCases), "integer<12, uint16_t>", test_tag);
#endif

#if	REGRESSION_LEVEL_4
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortAddition<uint8_t >(reportTestCases), "integer<16, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyShortAddition<uint16_t>(reportTestCases), "integer<16, uint16_t>", test_tag);
	// this is a 'standard' comparision against a native int64_t
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<16, uint16_t>(reportTestCases), "integer<16, uint16_t>", test_tag);
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
