// rounding.cpp: functional tests for rounding consistency in areal (arbitrary real) numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure areal environment
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// Verify round-trip conversion: areal -> double -> areal
// This tests that exact values convert correctly
template<typename ArealType>
int VerifyExactRoundTrip(bool reportTestCases) {
	constexpr size_t nbits = ArealType::nbits;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	ArealType a, result;

	// Only test exact values (even bit patterns - ubit=0)
	for (size_t i = 0; i < NR_VALUES; i += 2) {
		a.setbits(i);

		// Skip special values
		if (a.isnan() || a.isinf()) continue;

		double da = double(a);
		result = da;

		// For exact values, the round-trip should preserve the value
		// But result may have ubit=1 if precision is lost (shouldn't happen for exact round-trip)
		if (double(result) != da) {
			nrOfFailedTests++;
			if (reportTestCases) {
				std::cerr << "FAIL: round-trip " << to_binary(a) << " -> " << da
				          << " -> " << to_binary(result) << " = " << double(result) << std::endl;
			}
		}
	}
	return nrOfFailedTests;
}

// Verify that arithmetic results match direct assignment semantics
// For areal: both paths should truncate to floor and set ubit if precision lost
template<typename ArealType>
int VerifyArithmeticConversionConsistency(bool reportTestCases) {
	constexpr size_t nbits = ArealType::nbits;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	ArealType a, b, sum, direct;

	// Only test exact values (even bit patterns - ubit=0)
	for (size_t i = 0; i < NR_VALUES; i += 2) {
		a.setbits(i);
		if (a.isnan() || a.isinf()) continue;

		for (size_t j = 0; j < NR_VALUES; j += 2) {
			b.setbits(j);
			if (b.isnan() || b.isinf()) continue;

			// Compute sum via arithmetic
			sum = a + b;

			// Compute expected result via direct assignment
			double ref = double(a) + double(b);
			direct = ref;

			// The numerical values should match (ubit may differ)
			double dsum = double(sum);
			double ddirect = double(direct);

			if (dsum != ddirect && !(std::isnan(dsum) && std::isnan(ddirect))) {
				nrOfFailedTests++;
				if (reportTestCases) {
					std::cerr << "FAIL: " << to_binary(a) << " + " << to_binary(b)
					          << " = " << to_binary(sum) << " (" << dsum << ")"
					          << " expected " << to_binary(direct) << " (" << ddirect << ")"
					          << std::endl;
				}
			}
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal rounding verification";
	std::string test_tag    = "areal rounding";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Test specific cases
	{
		areal<5, 2, uint8_t> a, b, c, direct;
		a = 0.5;
		b = 3.0;
		c = a + b;
		direct = 3.5;

		std::cout << "0.5 + 3.0:\n";
		std::cout << "  arithmetic: " << to_binary(c) << " = " << double(c) << " ubit=" << c.ubit() << std::endl;
		std::cout << "  direct:     " << to_binary(direct) << " = " << double(direct) << " ubit=" << direct.ubit() << std::endl;
		std::cout << "  match: " << (double(c) == double(direct) ? "PASS" : "FAIL") << std::endl;
	}

	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<5, 2, uint8_t>>(true), "areal<5,2>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<5, 2, uint8_t>>(true), "areal<5,2>", "arithmetic consistency");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// Test round-trip conversion for various configurations
	// areal<nbits, es> requires nbits > es + 2
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<4, 1, uint8_t>>(reportTestCases), "areal< 4,1>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<5, 1, uint8_t>>(reportTestCases), "areal< 5,1>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<5, 2, uint8_t>>(reportTestCases), "areal< 5,2>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<6, 2, uint8_t>>(reportTestCases), "areal< 6,2>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<6, 3, uint8_t>>(reportTestCases), "areal< 6,3>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<7, 3, uint8_t>>(reportTestCases), "areal< 7,3>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<7, 4, uint8_t>>(reportTestCases), "areal< 7,4>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<8, 2, uint8_t>>(reportTestCases), "areal< 8,2>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<8, 4, uint8_t>>(reportTestCases), "areal< 8,4>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<8, 5, uint8_t>>(reportTestCases), "areal< 8,5>", "round-trip");

	// Test arithmetic conversion consistency
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<4, 1, uint8_t>>(reportTestCases), "areal< 4,1>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<5, 1, uint8_t>>(reportTestCases), "areal< 5,1>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<5, 2, uint8_t>>(reportTestCases), "areal< 5,2>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<6, 2, uint8_t>>(reportTestCases), "areal< 6,2>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<6, 3, uint8_t>>(reportTestCases), "areal< 6,3>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<7, 3, uint8_t>>(reportTestCases), "areal< 7,3>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<7, 4, uint8_t>>(reportTestCases), "areal< 7,4>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<8, 2, uint8_t>>(reportTestCases), "areal< 8,2>", "arith-consistency");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<9, 3, uint8_t>>(reportTestCases), "areal< 9,3>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<9, 5, uint8_t>>(reportTestCases), "areal< 9,5>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<10, 4, uint8_t>>(reportTestCases), "areal<10,4>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<10, 6, uint8_t>>(reportTestCases), "areal<10,6>", "round-trip");

	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<9, 3, uint8_t>>(reportTestCases), "areal< 9,3>", "arith-consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticConversionConsistency<areal<9, 5, uint8_t>>(reportTestCases), "areal< 9,5>", "arith-consistency");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<11, 4, uint8_t>>(reportTestCases), "areal<11,4>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<12, 5, uint8_t>>(reportTestCases), "areal<12,5>", "round-trip");
#endif

#if REGRESSION_LEVEL_4
	// Larger configurations - exhaustive testing
	nrOfFailedTestCases += ReportTestResult(VerifyExactRoundTrip<areal<14, 6, uint8_t>>(reportTestCases), "areal<14,6>", "round-trip");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
