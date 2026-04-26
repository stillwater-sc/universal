// subtraction.cpp : test suite runner for subtraction operator on fixed-sized, arbitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//#include <typeinfo>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

template<typename Scalar>
void GenerateSubTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
	z = x - y;
	std::cout << typeid(Scalar).name() << ": " << x << " - " << y << " = " << z << std::endl;
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

	std::string test_suite  = "Integer Arithmetic Subtraction verfication";
	std::string test_tag    = "integer<> subtraction";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	integer<12> a, b, c;
	a = 1234;
	b = 1235;
	GenerateSubTest(a, b, c);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, uint8_t >(reportTestCases), "integer< 5, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, uint8_t >(reportTestCases), "integer< 7, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 9, uint8_t >(reportTestCases), "integer< 9, uint8_t >", "subtraction");

	// Issue #758: WholeNumber and NaturalNumber operator-= now performs
	// magnitude subtraction (was previously an unimplemented placeholder).
	// VerifySubtraction is IntegerNumber-only via its template; spot-check
	// the unsigned-domain modes here.
	{
		std::cout << "+----- WholeNumber / NaturalNumber subtraction (issue #758)\n";
		using W32 = integer<32, std::uint32_t, IntegerNumberType::WholeNumber>;
		using N32 = integer<32, std::uint32_t, IntegerNumberType::NaturalNumber>;
		using N64u8 = integer<64, std::uint8_t, IntegerNumberType::NaturalNumber>;  // multi-limb

		auto check = [&](const char* name, long long expected, auto actual) {
			long long got = static_cast<long long>(actual);
			if (got != expected) {
				++nrOfFailedTestCases;
				std::cout << "FAIL " << name << "  expected=" << expected << "  got=" << got << '\n';
			}
		};

		// Single-limb
		{ W32 a(42); a -= W32(7);  check("W32 42 - 7",  35, a); }
		{ W32 a(1000); a -= W32(1); check("W32 1000 - 1", 999, a); }
		{ N32 a(42); a -= N32(7);  check("N32 42 - 7",  35, a); }
		{ N32 a(7);  a -= N32(7);  check("N32 7 - 7 (allowed zero)", 0, a); }

		// Multi-limb cross-limb borrow
		{ N64u8 a(256); a -= N64u8(1);     check("N64u8 256 - 1 (cross-limb borrow)", 255, a); }
		{ N64u8 a(70000); a -= N64u8(1);   check("N64u8 70000 - 1 (multi-limb)", 69999, a); }
		{ N64u8 a(70000); a -= N64u8(50000); check("N64u8 70000 - 50000", 20000, a); }
	}
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, uint8_t >(reportTestCases), "integer< 7, uint8_t >", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, uint16_t>(reportTestCases), "integer< 7, uint16_t>", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<10, uint8_t >(reportTestCases), "integer<10, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<10, uint16_t>(reportTestCases), "integer<10, uint16_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<11, uint8_t >(reportTestCases), "integer<11, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<13, uint16_t>(reportTestCases), "integer<13, uint16_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_4
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
//	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction<uint8_t >(reportTestCases), "integer<16, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction<uint16_t>(reportTestCases), "integer<16, uint16_t>", "subtraction");
	// this is a 'standard' comparision against a native int64_t
//	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<16, uint8_t>(reportTestCases), "integer<16, uint8_t>", "subtraction");
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
