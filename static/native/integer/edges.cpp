// edges.cpp: find_msb overloads, fastipow overflow, to_binary/to_hex widths (#2059)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep in #2039 found:
//   - find_msb(long) scanned 32 bits, so find_msb(1L << 40) was 0 on LP64 (int64_t is long there);
//   - fastipow squared in int64_t, signed-overflow UB for 3^41, and (-2)^63 = INT64_MIN came out 0;
//   - to_binary(x, marker, nbits) shifted a 1ull mask by nbits - 1, UB past 64 bits.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <limits>
#include <string>
#include <universal/native/integers.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	int report(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
		return 1;
	}

	int VerifyFindMsb(bool reportTestCases) {
		int fails = 0;
		for (unsigned k = 0; k < 63; ++k) {
			const long long v = 1ll << k;
			fails += report(find_msb(v) == k + 1, "find_msb(long long 2^" + std::to_string(k) + ")", reportTestCases);
			fails += report(find_msb(static_cast<std::uint64_t>(v)) == k + 1, "find_msb(uint64_t 2^" + std::to_string(k) + ")", reportTestCases);
			fails += report(find_msb(static_cast<std::int64_t>(v)) == k + 1, "find_msb(int64_t 2^" + std::to_string(k) + ")", reportTestCases);
			if (k < 8 * sizeof(long) - 1) {
				fails += report(find_msb(static_cast<long>(v)) == k + 1, "find_msb(long 2^" + std::to_string(k) + ")", reportTestCases);
			}
		}
		fails += report(find_msb(0L) == 0 && find_msb(0LL) == 0, "find_msb(0) is not 0", reportTestCases);
		fails += report(find_msb(static_cast<long>(-1)) == 8 * sizeof(long), "find_msb(long -1) is not the width of long", reportTestCases);
		fails += report(find_msb(static_cast<signed char>(-1)) == 8 && find_msb(static_cast<signed char>(0x40)) == 7, "find_msb(signed char)", reportTestCases);
		fails += report(find_msb(static_cast<short>(0x4000)) == 15, "find_msb(short 0x4000)", reportTestCases);
		return fails;
	}

	int VerifyFastipow(bool reportTestCases) {
		int fails = 0;
		// 3^39 fits, 3^40 = 12157665459056928801 does not fit int64_t: overflow is 0
		fails += report(fastipow(3, 39) == 4052555153018976267ll, "3^39", reportTestCases);
		fails += report(fastipow(3, 40) == 0 && fastipow(3, 41) == 0 && fastipow(3, 255) == 0, "3^40, 3^41, 3^255 do not overflow to 0", reportTestCases);
		fails += report(fastipow(-2, 63) == std::numeric_limits<std::int64_t>::min(), "(-2)^63 is not INT64_MIN", reportTestCases);
		fails += report(fastipow(2, 63) == 0, "2^63 does not overflow to 0", reportTestCases);
		fails += report(fastipow(2, 62) == (std::int64_t(1) << 62), "2^62", reportTestCases);
		fails += report(fastipow(-3, 3) == -27 && fastipow(-3, 4) == 81, "(-3)^3 and (-3)^4", reportTestCases);
		fails += report(fastipow(1, 255) == 1 && fastipow(-1, 255) == -1 && fastipow(-1, 254) == 1, "(+-1)^n", reportTestCases);
		fails += report(fastipow(0, 0) == 1 && fastipow(0, 7) == 0 && fastipow(7, 0) == 1, "0^0, 0^7, 7^0", reportTestCases);
		fails += report(fastipow(std::numeric_limits<std::int64_t>::min(), 1) == std::numeric_limits<std::int64_t>::min(), "INT64_MIN^1", reportTestCases);
		fails += report(fastipow(std::numeric_limits<std::int64_t>::min(), 2) == 0, "INT64_MIN^2 does not overflow to 0", reportTestCases);
		return fails;
	}

	int VerifyStrings(bool reportTestCases) {
		int fails = 0;
		fails += report(to_binary(std::uint8_t(5), false) == "0b00000101", "to_binary(uint8_t 5)", reportTestCases);
		fails += report(to_binary(std::int8_t(-1), false, 12) == "0b111111111111", "to_binary(int8_t -1, 12 bits) is not sign extended", reportTestCases);
		fails += report(to_binary(std::uint64_t(1), false, 70) == "0b" + std::string(69, '0') + "1", "to_binary(uint64_t 1, 70 bits)", reportTestCases);
		fails += report(to_binary(std::int64_t(-2), false, 66) == "0b" + std::string(65, '1') + "0", "to_binary(int64_t -2, 66 bits) is not sign extended", reportTestCases);
		fails += report(to_binary(std::uint16_t(0xA5), false, 4) == "0b0101", "to_binary(0xA5, 4 bits)", reportTestCases);
		fails += report(to_hex(std::uint32_t(0xDEADBEEF)) == "0xDEADBEEF", "to_hex(0xDEADBEEF)", reportTestCases);
		fails += report(to_hex(std::int16_t(-1)) == "0xFFFF", "to_hex(int16_t -1)", reportTestCases);
		fails += report(to_hex(std::uint64_t(0x0123456789ABCDEFull)) == "0x0123456789ABCDEF", "to_hex(uint64_t)", reportTestCases);
		return fails;
	}

}  // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "native integer helpers at their edges (#2059)";
	std::string test_tag    = "integer edges";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb(reportTestCases), test_tag, "find_msb");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb(reportTestCases), test_tag, "find_msb");
	nrOfFailedTestCases += ReportTestResult(VerifyFastipow(reportTestCases), test_tag, "fastipow");
	nrOfFailedTestCases += ReportTestResult(VerifyStrings(reportTestCases), test_tag, "to_binary/to_hex");
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
