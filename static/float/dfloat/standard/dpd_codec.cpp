// dpd_codec.cpp: exhaustive verification of DPD (Densely Packed Decimal) encoding
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite = "DPD (Densely Packed Decimal) codec verification";
	std::string test_tag = "DPD codec";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// Test 1: Verify all 1000 encode/decode round-trips
	std::cout << "+---------    Exhaustive DPD encode/decode round-trip test (0-999)\n";
	{
		for (unsigned i = 0; i < 1000; ++i) {
			uint16_t encoded = dpd_encode(i);
			unsigned decoded = dpd_decode(encoded);
			if (decoded != i) {
				std::cerr << "FAIL: dpd_encode(" << i << ") = 0x"
				          << std::hex << encoded << std::dec
				          << ", dpd_decode = " << decoded << " (expected " << i << ")\n";
				++nrOfFailedTestCases;
			}
		}
		if (nrOfFailedTestCases == 0) {
			std::cout << "  All 1000 encode/decode round-trips PASS\n";
		}
	}

	// Test 2: Verify that all 1024 declets decode to valid values (0-999)
	std::cout << "+---------    All 1024 declets decode to valid 3-digit values\n";
	{
		int invalid_count = 0;
		for (unsigned i = 0; i < 1024; ++i) {
			unsigned decoded = dpd_decode(static_cast<uint16_t>(i));
			if (decoded > 999) {
				++invalid_count;
				++nrOfFailedTestCases;
			}
		}
		std::cout << "  Invalid declets: " << invalid_count << " (24 expected for non-canonical encodings)\n";
	}

	// Test 3: Spot checks for known DPD encodings
	std::cout << "+---------    Spot checks for known DPD values\n";
	{
		// 0 -> should encode to 0b0000000000
		uint16_t e0 = dpd_encode(0);
		if (e0 != 0) {
			std::cerr << "FAIL: dpd_encode(0) = " << e0 << " (expected 0)\n";
			++nrOfFailedTestCases;
		}

		// 5 -> all small digits: 0,0,5 -> 000 000 0 101 = 0x005
		uint16_t e5 = dpd_encode(5);
		unsigned d5 = dpd_decode(e5);
		if (d5 != 5) {
			std::cerr << "FAIL: round-trip for 5: got " << d5 << '\n';
			++nrOfFailedTestCases;
		}

		// 999 -> all large digits (9,9,9)
		uint16_t e999 = dpd_encode(999);
		unsigned d999 = dpd_decode(e999);
		if (d999 != 999) {
			std::cerr << "FAIL: round-trip for 999: got " << d999 << '\n';
			++nrOfFailedTestCases;
		}

		// 123 -> all small: 1,2,3
		uint16_t e123 = dpd_encode(123);
		unsigned d123 = dpd_decode(e123);
		if (d123 != 123) {
			std::cerr << "FAIL: round-trip for 123: got " << d123 << '\n';
			++nrOfFailedTestCases;
		}

		// 890 -> d0=8 (large), d1=9 (large), d2=0 (small)
		uint16_t e890 = dpd_encode(890);
		unsigned d890 = dpd_decode(e890);
		if (d890 != 890) {
			std::cerr << "FAIL: round-trip for 890: got " << d890 << '\n';
			++nrOfFailedTestCases;
		}

		std::cout << "  Spot checks: " << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << '\n';
	}

	// Test 4: DPD significand encode/decode for decimal32 (7 digits, 6 trailing = 2 declets)
	std::cout << "+---------    DPD significand encode/decode for decimal32 config\n";
	{
		// Test with significand = 1234567 (7 digits)
		// MSD = 1, trailing 6 digits = 234567
		// 234567 = 234*1000 + 567
		uint64_t sig = 1234567;
		uint64_t encoded = dpd_encode_significand(sig, 7);
		uint64_t decoded = dpd_decode_significand(encoded, 7);
		if (decoded != 234567) {
			std::cerr << "FAIL: significand encode/decode for 1234567: trailing decoded = "
			          << decoded << " (expected 234567)\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "  Significand 1234567 trailing encode/decode: PASS\n";
		}
	}

	// Test 5: DPD dfloat round-trip through value construction
	std::cout << "+---------    DPD dfloat value round-trip\n";
	{
		using DpdFloat = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;
		DpdFloat a(42.0);
		double d = double(a);
		std::cout << "  DPD dfloat<7,6>(42.0) = " << d << " : " << to_binary(a) << '\n';

		DpdFloat b(0.1);
		double db = double(b);
		std::cout << "  DPD dfloat<7,6>(0.1) = " << db << " : " << to_binary(b) << '\n';
	}

#endif

#if REGRESSION_LEVEL_2
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
