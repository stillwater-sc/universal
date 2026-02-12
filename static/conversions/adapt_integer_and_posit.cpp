// adapt_integer_and_posit.cpp : test suite for posit<->integer adapter conversions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable conversion between posits and integers
#include <universal/adapters/adapt_integer_and_posit.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
// configure the posit arithmetic class
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	////////////////////////////////////////////////////////////////////////
	// Tests for convert_p2i (posit to integer conversion)

	// Test convert_p2i with scale < 0 (fractional posit values map to 0)
	template<unsigned pbits, unsigned pes, unsigned ibits>
	int VerifyP2I_ScaleLessThanZero(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Integer result;

		// Test 0.5 (scale = -1)
		{
			Posit p(0.5);
			convert_p2i(p, result);
			if (result != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(0.5) = " << result << " (expected 0)\n";
			}
		}

		// Test 0.25 (scale = -2)
		{
			Posit p(0.25);
			convert_p2i(p, result);
			if (result != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(0.25) = " << result << " (expected 0)\n";
			}
		}

		// Test -0.5 (scale = -1, negative)
		{
			Posit p(-0.5);
			convert_p2i(p, result);
			if (result != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(-0.5) = " << result << " (expected 0)\n";
			}
		}

		// Test minpos (smallest positive value)
		{
			Posit p;
			p.minpos();
			convert_p2i(p, result);
			if (result != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(minpos) = " << result << " (expected 0)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test convert_p2i with scale == 0 (values between 1 and 2 map to 1)
	template<unsigned pbits, unsigned pes, unsigned ibits>
	int VerifyP2I_ScaleEqualZero(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Integer result;

		// Test 1.0 (scale = 0)
		{
			Posit p(1.0);
			convert_p2i(p, result);
			if (result != 1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(1.0) = " << result << " (expected 1)\n";
			}
		}

		// Test 1.5 (scale = 0, with fraction)
		{
			Posit p(1.5);
			convert_p2i(p, result);
			// 1.5 has scale 0, so result should be 1 (truncation)
			if (result != 1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(1.5) = " << result << " (expected 1)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test convert_p2i with scale > 0 (larger integer values)
	template<unsigned pbits, unsigned pes, unsigned ibits>
	int VerifyP2I_ScaleGreaterThanZero(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Integer result;

		// Test 2.0 (scale = 1)
		{
			Posit p(2.0);
			convert_p2i(p, result);
			if (result != 2) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(2.0) = " << result << " (expected 2)\n";
			}
		}

		// Test 4.0 (scale = 2)
		{
			Posit p(4.0);
			convert_p2i(p, result);
			if (result != 4) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(4.0) = " << result << " (expected 4)\n";
			}
		}

		// Test 8.0 (scale = 3)
		{
			Posit p(8.0);
			convert_p2i(p, result);
			if (result != 8) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(8.0) = " << result << " (expected 8)\n";
			}
		}

		// Test 3.0 (scale = 1, with fraction 0.5)
		{
			Posit p(3.0);
			convert_p2i(p, result);
			if (result != 3) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(3.0) = " << result << " (expected 3)\n";
			}
		}

		// Test 7.0 (scale = 2, with fraction)
		{
			Posit p(7.0);
			convert_p2i(p, result);
			if (result != 7) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(7.0) = " << result << " (expected 7)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test convert_p2i with negative posit values
	template<unsigned pbits, unsigned pes, unsigned ibits>
	int VerifyP2I_NegativeValues(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Integer result;

		// Test -1.0
		{
			Posit p(-1.0);
			convert_p2i(p, result);
			if (result != -1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(-1.0) = " << result << " (expected -1)\n";
			}
		}

		// Test -2.0
		{
			Posit p(-2.0);
			convert_p2i(p, result);
			if (result != -2) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(-2.0) = " << result << " (expected -2)\n";
			}
		}

		// Test -4.0
		{
			Posit p(-4.0);
			convert_p2i(p, result);
			if (result != -4) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(-4.0) = " << result << " (expected -4)\n";
			}
		}

		// Test -7.0
		{
			Posit p(-7.0);
			convert_p2i(p, result);
			if (result != -7) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_p2i(-7.0) = " << result << " (expected -7)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Tests for convert_i2p (integer to posit conversion)

	// Test convert_i2p with zero
	template<unsigned ibits, unsigned pbits, unsigned pes>
	int VerifyI2P_Zero(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Posit result;
		Integer zero(0);

		convert_i2p(zero, result);
		if (!result.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: convert_i2p(0) = " << result << " (expected 0)\n";
		}

		return nrOfFailedTests;
	}

	// Test convert_i2p with positive integers
	template<unsigned ibits, unsigned pbits, unsigned pes>
	int VerifyI2P_PositiveValues(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Posit result;

		// Test 1
		{
			Integer i(1);
			convert_i2p(i, result);
			if (double(result) != 1.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(1) = " << result << " (expected 1)\n";
			}
		}

		// Test 2
		{
			Integer i(2);
			convert_i2p(i, result);
			if (double(result) != 2.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(2) = " << result << " (expected 2)\n";
			}
		}

		// Test 4
		{
			Integer i(4);
			convert_i2p(i, result);
			if (double(result) != 4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(4) = " << result << " (expected 4)\n";
			}
		}

		// Test 7
		{
			Integer i(7);
			convert_i2p(i, result);
			if (double(result) != 7.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(7) = " << result << " (expected 7)\n";
			}
		}

		// Test 10
		{
			Integer i(10);
			convert_i2p(i, result);
			if (double(result) != 10.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(10) = " << result << " (expected 10)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test convert_i2p with negative integers
	template<unsigned ibits, unsigned pbits, unsigned pes>
	int VerifyI2P_NegativeValues(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Posit result;

		// Test -1
		{
			Integer i(-1);
			convert_i2p(i, result);
			if (double(result) != -1.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(-1) = " << result << " (expected -1)\n";
			}
		}

		// Test -2
		{
			Integer i(-2);
			convert_i2p(i, result);
			if (double(result) != -2.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(-2) = " << result << " (expected -2)\n";
			}
		}

		// Test -4
		{
			Integer i(-4);
			convert_i2p(i, result);
			if (double(result) != -4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(-4) = " << result << " (expected -4)\n";
			}
		}

		// Test -7
		{
			Integer i(-7);
			convert_i2p(i, result);
			if (double(result) != -7.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(-7) = " << result << " (expected -7)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test convert_i2p with powers of 2
	template<unsigned ibits, unsigned pbits, unsigned pes>
	int VerifyI2P_PowersOfTwo(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		Posit result;

		// Test powers of 2: 1, 2, 4, 8, 16, 32
		for (int exp = 0; exp <= 5; ++exp) {
			Integer i(1 << exp);
			convert_i2p(i, result);
			double expected = static_cast<double>(1 << exp);
			if (double(result) != expected) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert_i2p(2^" << exp << ") = " << result << " (expected " << expected << ")\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Round-trip tests

	// Test that integer -> posit -> integer preserves value (for representable integers)
	// Note: Only powers of 2 and small integers are exactly representable in small posits
	template<unsigned ibits, unsigned pbits, unsigned pes>
	int VerifyI2P2I_RoundTrip(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		// Test small integers that should round-trip exactly
		// For small posits like posit<8,0>, only powers of 2 and nearby values are exact
		// Use conservative set: 0, ±1, ±2, ±4, ±8 (powers of 2 that all posits can represent)
		int testValues[] = { 0, 1, -1, 2, -2, 4, -4, 8, -8 };

		for (int val : testValues) {
			Integer original(val);
			Posit intermediate;
			Integer result;

			convert_i2p(original, intermediate);
			convert_p2i(intermediate, result);

			if (original != result) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: round-trip i2p2i(" << val << "): original=" << original << " result=" << result << "\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test that posit -> integer -> posit preserves value (for integer posit values)
	template<unsigned pbits, unsigned pes, unsigned ibits>
	int VerifyP2I2P_RoundTrip(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<pbits, pes>;
		using Integer = integer<ibits, uint32_t, IntegerNumberType::IntegerNumber>;

		// Test integer-valued posits
		double testValues[] = { 1.0, -1.0, 2.0, -2.0, 4.0, -4.0, 8.0, -8.0 };

		for (double val : testValues) {
			Posit original(val);
			Integer intermediate;
			Posit result;

			convert_p2i(original, intermediate);
			convert_i2p(intermediate, result);

			if (double(original) != double(result)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: round-trip p2i2p(" << val << "): original=" << original << " result=" << result << "\n";
			}
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

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

	std::string test_suite  = "adapt_integer_and_posit verification";
	std::string test_tag    = "posit<->integer conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;

	// Manual testing with specific configurations
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleLessThanZero<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "scale<0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleEqualZero<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "scale==0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleGreaterThanZero<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "scale>0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_NegativeValues<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "negative");

	nrOfFailedTestCases += ReportTestResult(VerifyI2P_Zero<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "zero");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PositiveValues<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "positive");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_NegativeValues<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "negative");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PowersOfTwo<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "powers of 2");

	nrOfFailedTestCases += ReportTestResult(VerifyI2P2I_RoundTrip<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>->int<16>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I2P_RoundTrip<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>->posit<16,1>", "round-trip");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// convert_p2i tests
	std::cout << "\nconvert_p2i (posit to integer) tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleLessThanZero<8, 0, 8>(reportTestCases), "posit<8,0>->int<8>", "scale<0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleLessThanZero<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "scale<0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleEqualZero<8, 0, 8>(reportTestCases), "posit<8,0>->int<8>", "scale==0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleEqualZero<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "scale==0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleGreaterThanZero<8, 0, 8>(reportTestCases), "posit<8,0>->int<8>", "scale>0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleGreaterThanZero<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "scale>0");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_NegativeValues<8, 0, 8>(reportTestCases), "posit<8,0>->int<8>", "negative");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_NegativeValues<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>", "negative");

	// convert_i2p tests
	std::cout << "\nconvert_i2p (integer to posit) tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_Zero<8, 8, 0>(reportTestCases), "int<8>->posit<8,0>", "zero");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_Zero<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "zero");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PositiveValues<8, 8, 0>(reportTestCases), "int<8>->posit<8,0>", "positive");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PositiveValues<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "positive");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_NegativeValues<8, 8, 0>(reportTestCases), "int<8>->posit<8,0>", "negative");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_NegativeValues<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "negative");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PowersOfTwo<8, 8, 0>(reportTestCases), "int<8>->posit<8,0>", "powers of 2");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PowersOfTwo<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>", "powers of 2");

	// Round-trip tests
	std::cout << "\nRound-trip tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyI2P2I_RoundTrip<8, 8, 0>(reportTestCases), "int<8>->posit<8,0>->int<8>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P2I_RoundTrip<16, 16, 1>(reportTestCases), "int<16>->posit<16,1>->int<16>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I2P_RoundTrip<8, 0, 8>(reportTestCases), "posit<8,0>->int<8>->posit<8,0>", "round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyP2I2P_RoundTrip<16, 1, 16>(reportTestCases), "posit<16,1>->int<16>->posit<16,1>", "round-trip");
#endif

#if REGRESSION_LEVEL_2
	// Larger configurations
	nrOfFailedTestCases += ReportTestResult(VerifyP2I_ScaleGreaterThanZero<32, 2, 32>(reportTestCases), "posit<32,2>->int<32>", "scale>0");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P_PositiveValues<32, 32, 2>(reportTestCases), "int<32>->posit<32,2>", "positive");
	nrOfFailedTestCases += ReportTestResult(VerifyI2P2I_RoundTrip<32, 32, 2>(reportTestCases), "int<32>->posit<32,2>->int<32>", "round-trip");
#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
