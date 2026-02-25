// integer_conversion.cpp: test suite for integer-to-areal conversions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Verify that convert_unsigned_integer and convert_signed_integer correctly
// handle integers beyond 2^53 (where the double-delegation path silently
// loses precision) and properly set the uncertainty bit (ubit) when
// low-order bits are truncated.
#include <universal/utility/directives.hpp>
// Configure the areal template environment
#define AREAL_FAST_SPECIALIZATION
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#define TRACE_CONVERSION 0

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// Regression testing guards
#define MANUAL_TESTING 0
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

/// Verify that small integers (powers of 2, boundary values) convert exactly.
/// These should work for both the double-delegation and native paths.
template<typename ArealType>
int VerifySmallIntegerConversion(bool reportTestCases) {
	int nrOfFailedTestCases = 0;

	// Test zero
	{
		ArealType a;
		a = 0ull;
		if (double(a) != 0.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 0ull -> " << double(a) << " expected 0\n";
		}
	}
	// Test 1
	{
		ArealType a;
		a = 1ull;
		if (double(a) != 1.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1ull -> " << double(a) << " expected 1\n";
		}
	}
	// Test powers of 2 within the normal range of the areal type
	// Stay below MAX_EXP to avoid boundary encoding issues
	constexpr int maxExp = ArealType::MAX_EXP;
	constexpr unsigned maxPow = (maxExp > 1 && maxExp < 53) ? static_cast<unsigned>(maxExp - 1) : 52u;
	for (unsigned p = 1; p <= maxPow; ++p) {
		uint64_t v = 1ull << p;
		ArealType a;
		a = v;
		double expected = static_cast<double>(v);
		if (double(a) != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 2^" << p << " -> " << double(a) << " expected " << expected << "\n";
		}
	}
	// Test small odd integers that fit within the type's precision
	// The type can represent integers exactly up to 2^(fbits+1)
	constexpr unsigned maxExactBits = ArealType::fbits + 1;
	constexpr uint64_t maxExactInt = (maxExactBits < 64) ? (1ull << maxExactBits) : 0xFFFFFFFFFFFFFFFFull;
	uint64_t oddLimit = (maxExactInt < 255) ? maxExactInt : 255;
	for (uint64_t v = 1; v <= oddLimit; v += 2) {
		ArealType a;
		a = v;
		double expected = static_cast<double>(v);
		if (double(a) != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << v << " -> " << double(a) << " expected " << expected << "\n";
		}
	}
	// Test signed small integers within representable range
	int64_t signedLimit = static_cast<int64_t>((oddLimit < 127) ? oddLimit : 127);
	for (int64_t v = -signedLimit; v <= signedLimit; ++v) {
		ArealType a;
		a = v;
		double expected = static_cast<double>(v);
		if (double(a) != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << v << " -> " << double(a) << " expected " << expected << "\n";
		}
	}
	return nrOfFailedTestCases;
}

/// Verify that integers > 2^53 convert correctly when fbits >= 53.
/// This specifically tests the native conversion path that avoids
/// precision loss through double.
template<typename ArealType>
int VerifyLargeUnsignedIntegerConversion(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	constexpr unsigned fbits = ArealType::fbits;

	// 2^53 + 1: this is NOT exactly representable in double
	// but should be exact in areal with fbits >= 53
	{
		constexpr uint64_t v = (1ull << 53) + 1;
		ArealType a;
		a = v;
		// Convert back: the round-trip should preserve the value
		// since fbits >= 53 and the integer has 53 fraction bits
		ArealType ref;
		ref.setbits(0); // clear
		// Construct expected: exponent = 53, fraction has bit 0 set
		// We verify by checking that the value differs from 2^53
		ArealType a_pow53;
		a_pow53 = (1ull << 53);
		if constexpr (fbits >= 53) {
			// a should differ from a_pow53 (the +1 must be preserved)
			if (a == a_pow53) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 2^53+1 collapsed to 2^53 (precision lost)\n";
			}
			// Check ubit is NOT set (exact representation)
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 2^53+1 ubit should be clear for fbits >= 53\n";
			}
		}
	}

	// 2^54 + 1: exponent=54, 54 fraction bits needed
	// For fbits == 53, the LSB is truncated -> ubit must be set
	{
		constexpr uint64_t v = (1ull << 54) + 1;
		ArealType a;
		a = v;
		if constexpr (fbits == 53) {
			// LSB truncated: ubit must be set
			if (!a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 2^54+1 ubit should be set for fbits == 53 (1 bit truncated)\n";
			}
		}
		if constexpr (fbits >= 54) {
			// Exact representation, ubit clear
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 2^54+1 ubit should be clear for fbits >= 54\n";
			}
		}
	}

	// 2^53 + 3: two least significant bits set
	{
		constexpr uint64_t v = (1ull << 53) + 3;
		ArealType a;
		a = v;
		if constexpr (fbits >= 53) {
			// Both bits should be preserved
			ArealType a_plus1;
			a_plus1 = (1ull << 53) + 1;
			ArealType a_plus2;
			a_plus2 = (1ull << 53) + 2;
			// a should differ from both a+1 and a+2
			if (a == a_plus1 || a == a_plus2) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 2^53+3 should be distinct from 2^53+1 and 2^53+2\n";
			}
			// ubit should be clear
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 2^53+3 ubit should be clear for fbits >= 53\n";
			}
		}
	}

	// Powers of 2 beyond 2^53 should always be exact (fraction is zero)
	for (unsigned p = 53; p <= 62; ++p) {
		uint64_t v = 1ull << p;
		ArealType a;
		a = v;
		// ubit must be clear: powers of 2 are always exact
		if (a.at(0)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 2^" << p << " ubit should be clear\n";
		}
	}

	// MAX uint64_t: 2^64 - 1 = all bits set, exponent=63, 63 fraction bits
	{
		constexpr uint64_t v = 0xFFFF'FFFF'FFFF'FFFFull;
		ArealType a;
		a = v;
		if constexpr (fbits >= 63) {
			// All 63 fraction bits should fit, ubit clear
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: UINT64_MAX ubit should be clear for fbits >= 63\n";
			}
		}
		else {
			// Truncation: ubit must be set (since all bits are 1, truncated bits are non-zero)
			if (!a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: UINT64_MAX ubit should be set for fbits < 63 (fbits=" << fbits << ")\n";
			}
		}
	}

	return nrOfFailedTestCases;
}

/// Verify signed integer conversion for large values and edge cases.
template<typename ArealType>
int VerifyLargeSignedIntegerConversion(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	constexpr unsigned fbits = ArealType::fbits;

	// -(2^53 + 1): negative value beyond double precision
	{
		constexpr int64_t v = -((1ll << 53) + 1);
		ArealType a;
		a = v;
		// sign bit must be set
		if (!a.at(ArealType::nbits - 1)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -(2^53+1) sign bit not set\n";
		}
		if constexpr (fbits >= 53) {
			// Should be exact, ubit clear
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: -(2^53+1) ubit should be clear for fbits >= 53\n";
			}
			// Should differ from -(2^53)
			ArealType a_neg;
			a_neg = -(1ll << 53);
			if (a == a_neg) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: -(2^53+1) collapsed to -(2^53)\n";
			}
		}
	}

	// INT64_MIN = -2^63: power of 2, fraction is zero, should be exact
	{
		constexpr int64_t v = std::numeric_limits<int64_t>::min(); // -2^63
		ArealType a;
		a = v;
		// sign bit must be set
		if (!a.at(ArealType::nbits - 1)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: INT64_MIN sign bit not set\n";
		}
		// power of 2: fraction = 0, always exact
		if (a.at(0)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: INT64_MIN ubit should be clear (power of 2)\n";
		}
	}

	// INT64_MIN + 1 = -(2^63 - 1): all fraction bits set, exponent=62
	{
		constexpr int64_t v = std::numeric_limits<int64_t>::min() + 1; // -(2^63 - 1)
		ArealType a;
		a = v;
		// sign bit must be set
		if (!a.at(ArealType::nbits - 1)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: INT64_MIN+1 sign bit not set\n";
		}
		if constexpr (fbits >= 62) {
			// All 62 fraction bits fit, ubit clear
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: INT64_MIN+1 ubit should be clear for fbits >= 62\n";
			}
		}
		else {
			// Truncation: all fraction bits are 1, so truncated bits are non-zero
			if (!a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: INT64_MIN+1 ubit should be set for fbits < 62 (fbits=" << fbits << ")\n";
			}
		}
	}

	// INT64_MAX = 2^63 - 1: exponent=62, 62 fraction bits all set
	{
		constexpr int64_t v = std::numeric_limits<int64_t>::max();
		ArealType a;
		a = v;
		// sign bit should NOT be set (positive)
		if (a.at(ArealType::nbits - 1)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: INT64_MAX sign bit should not be set\n";
		}
		if constexpr (fbits >= 62) {
			if (a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: INT64_MAX ubit should be clear for fbits >= 62\n";
			}
		}
		else {
			if (!a.at(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: INT64_MAX ubit should be set for fbits < 62 (fbits=" << fbits << ")\n";
			}
		}
	}

	return nrOfFailedTestCases;
}

/// Verify that values assigned through integer and double paths are
/// consistent for integers <= 2^53 (both paths should agree).
template<typename ArealType>
int VerifyIntegerDoubleConsistency(bool reportTestCases) {
	int nrOfFailedTestCases = 0;

	// For integers up to 2^53, both paths should produce identical bit patterns
	uint64_t testValues[] = {
		0, 1, 2, 3, 7, 8, 15, 16, 127, 128, 255, 256,
		1023, 1024, 65535, 65536,
		(1ull << 23) - 1, 1ull << 23, (1ull << 23) + 1,
		(1ull << 52) - 1, 1ull << 52, (1ull << 52) + 1,
		(1ull << 53) - 1, 1ull << 53,
	};

	for (uint64_t v : testValues) {
		ArealType from_int, from_double;
		from_int = v;
		from_double = static_cast<double>(v);
		// Compare bit patterns
		bool match = true;
		for (unsigned i = 0; i < ArealType::nbits; ++i) {
			if (from_int.at(i) != from_double.at(i)) {
				match = false;
				break;
			}
		}
		if (!match) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cerr << "FAIL: integer vs double path mismatch for " << v
					<< " int=" << sw::universal::to_binary(from_int)
					<< " dbl=" << sw::universal::to_binary(from_double) << "\n";
			}
		}
	}

	// Same for signed values
	int64_t signedTestValues[] = {
		-1, -2, -3, -7, -8, -128, -256, -65536,
		-(1ll << 52), -((1ll << 52) + 1),
		-(1ll << 53),
	};

	for (int64_t v : signedTestValues) {
		ArealType from_int, from_double;
		from_int = v;
		from_double = static_cast<double>(v);
		bool match = true;
		for (unsigned i = 0; i < ArealType::nbits; ++i) {
			if (from_int.at(i) != from_double.at(i)) {
				match = false;
				break;
			}
		}
		if (!match) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cerr << "FAIL: signed integer vs double path mismatch for " << v
					<< " int=" << sw::universal::to_binary(from_int)
					<< " dbl=" << sw::universal::to_binary(from_double) << "\n";
			}
		}
	}

	return nrOfFailedTestCases;
}

/*

  - VerifySmallIntegerConversion — powers of 2, small odd integers, and signed integers within the type's representable range; 
                                   tests both the double-delegation path (fbits < 53) and the native path (fbits >= 53)

  - VerifyLargeUnsignedIntegerConversion — values beyond 2^53 (2^53+1, 2^54+1, 2^53+3, large powers of 2, UINT64_MAX) 
                                           with verification that the ubit is correctly set when bits are truncated and clear when representation is exact

  - VerifyLargeSignedIntegerConversion — -(2^53+1), INT64_MIN, INT64_MIN+1, INT64_MAX with sign bit and ubit verification

  - VerifyIntegerDoubleConsistency — verifies bit-for-bit agreement between the integer assignment path and the 
                                     double assignment path for integers up to 2^53 (where both should produce identical results)

 */

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal integer conversion";
	std::string test_tag    = "integer conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		// Quick sanity: 2^53 + 1 through native path
		using Areal = areal<64, 8>; // fbits = 64 - 2 - 8 = 54 >= 53
		Areal a;
		a = (1ull << 53) + 1;
		Areal b;
		b = (1ull << 53);
		std::cout << "2^53+1: " << to_binary(a) << " ubit=" << a.at(0) << "\n";
		std::cout << "2^53  : " << to_binary(b) << " ubit=" << b.at(0) << "\n";
		std::cout << "equal? " << (a == b ? "YES (BUG)" : "NO (correct)") << "\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// ---- Small integer tests (both paths) ----
	// fbits < 53: exercises the double-delegation fallback path
	std::cout << "Small integer conversion (fbits < 53, double-delegation path)\n";
	nrOfFailedTestCases += ReportTestResult( VerifySmallIntegerConversion<areal<16, 5>>(reportTestCases), test_tag, "areal<16,5> small integers");
	nrOfFailedTestCases += ReportTestResult( VerifySmallIntegerConversion<areal<32, 8>>(reportTestCases), test_tag, "areal<32,8> small integers");

	// fbits >= 53: exercises the native conversion path
	std::cout << "Small integer conversion (fbits >= 53, native path)\n";
	nrOfFailedTestCases += ReportTestResult( VerifySmallIntegerConversion<areal<64, 8>>(reportTestCases), test_tag, "areal<64,8> small integers");

	// ---- Consistency: integer path vs double path for values <= 2^53 ----
	std::cout << "Integer vs double path consistency\n";
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerDoubleConsistency<areal<64, 8>>(reportTestCases), test_tag, "areal<64,8> int-double consistency");

	// ---- Large unsigned integers (native path, fbits >= 53) ----
	std::cout << "Large unsigned integer conversion (native path)\n";
	// fbits = 54: can represent 2^53+1 exactly, truncates at 2^55+1
	nrOfFailedTestCases += ReportTestResult( VerifyLargeUnsignedIntegerConversion<areal<64, 8>>(reportTestCases), test_tag, "areal<64,8> large unsigned");

	// ---- Large signed integers (native path, fbits >= 53) ----
	std::cout << "Large signed integer conversion (native path)\n";
	nrOfFailedTestCases += ReportTestResult( VerifyLargeSignedIntegerConversion<areal<64, 8>>(reportTestCases), test_tag, "areal<64,8> large signed");

#endif

#if REGRESSION_LEVEL_2

	// Test with different exponent sizes to exercise different fbits thresholds
	// areal<64, 2>: fbits = 60, can represent up to 2^60 fraction bits
	nrOfFailedTestCases += ReportTestResult( VerifyLargeUnsignedIntegerConversion<areal<64, 2>>(reportTestCases), test_tag, "areal<64,2> large unsigned");
	nrOfFailedTestCases += ReportTestResult( VerifyLargeSignedIntegerConversion<areal<64, 2>>(reportTestCases), test_tag, "areal<64,2> large signed");

	// areal<64, 4>: fbits = 58
	nrOfFailedTestCases += ReportTestResult( VerifyLargeUnsignedIntegerConversion<areal<64, 4>>(reportTestCases), test_tag, "areal<64,4> large unsigned");
	nrOfFailedTestCases += ReportTestResult( VerifyLargeSignedIntegerConversion<areal<64, 4>>(reportTestCases), test_tag, "areal<64,4> large signed");

	// areal<64, 11>: fbits = 51 (below threshold, uses double delegation)
	nrOfFailedTestCases += ReportTestResult( VerifySmallIntegerConversion<areal<64, 11>>(reportTestCases), test_tag, "areal<64,11> small integers");
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerDoubleConsistency<areal<64, 11>>(reportTestCases), test_tag, "areal<64,11> int-double consistency");

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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
