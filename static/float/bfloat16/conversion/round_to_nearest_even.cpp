// round_to_nearest_even.cpp: regression tests for RNE rounding when converting
//                            a native IEEE-754 float/double into a bfloat16 (#1133)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Before #1133, bfloat16 conversion truncated the low 16 bits of the float
// representation (round-toward-zero). Google TPUs and Intel use round-to-
// nearest, ties-to-even (RNE). These tests pin the RNE behavior, including the
// exact-halfway tie-to-even boundary and preservation of inf/nan/zero.

#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {

// build a float from a raw 32-bit IEEE-754 pattern
float f32(std::uint32_t bits) { return sw::bit_cast<float>(bits); }

}  // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite   = "bfloat16 round-to-nearest-even conversion (#1133)";
	bool reportTestCases     = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// helper: convert a value, compare the resulting float to an expected float
	auto expect = [&](double input, float expected, const char* label) {
		bfloat16 b(input);
		float got = float(b);
		if (got != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "  FAIL " << label << ": " << input
				          << " -> " << got << " (" << to_binary(b) << ")"
				          << " expected " << expected << '\n';
			}
		}
	};

	// ----- the exact case from issue #1133 -----
	{
		int start = nrOfFailedTestCases;
		// 0.2691408770292272 -> float bits 0x3E89CCD5; dropped bits 0xCCD5 > half,
		// so RNE rounds UP to 0x3E8A (0.26953125). Truncation gave 0.267578125.
		expect(0.2691408770292272, 0.26953125f, "issue-1133 round up");
		// and assert it is NOT the truncated value
		if (float(bfloat16(0.2691408770292272)) == 0.267578125f) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 issue-1133 case\n";
	}

	// ----- more-than-half rounds up, less-than-half stays -----
	{
		int start = nrOfFailedTestCases;
		// base 1.0 == 0x3F800000; next bfloat16 up is 0x3F81 == 1.0078125
		expect(f32(0x3F80C000u), 1.0078125f, "guard+sticky rounds up");   // dropped 0xC000 > half
		expect(f32(0x3F804000u), 1.0f,        "below half truncates");    // dropped 0x4000 < half
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 up/down rounding\n";
	}

	// ----- exact-half ties round to even -----
	{
		int start = nrOfFailedTestCases;
		// dropped bits exactly 0x8000, retained lsb == 0 (even) -> stays 1.0
		expect(f32(0x3F808000u), 1.0f,        "tie to even (lsb=0) stays");
		// dropped bits exactly 0x8000, retained lsb == 1 (odd)  -> rounds up to 0x3F82 == 1.015625
		expect(f32(0x3F818000u), 1.015625f,   "tie to even (lsb=1) rounds up");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 tie-to-even\n";
	}

	// ----- special values survive rounding -----
	{
		int start = nrOfFailedTestCases;
		if (!bfloat16(f32(0x7F800000u)).isinf()) ++nrOfFailedTestCases;            // +inf
		if (!bfloat16(f32(0xFF800000u)).isinf()) ++nrOfFailedTestCases;            // -inf
		if (!bfloat16(f32(0x7FC00000u)).isnan()) ++nrOfFailedTestCases;            // qnan
		// NaN whose payload is only in the low 16 bits must NOT collapse to inf
		if (!bfloat16(f32(0x7F800001u)).isnan()) ++nrOfFailedTestCases;
		if (!bfloat16(0.0).iszero())             ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 special-value preservation\n";
	}

	// ----- round_style advertises round_to_nearest -----
	{
		int start = nrOfFailedTestCases;
		if (std::numeric_limits<bfloat16>::round_style != std::round_to_nearest) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 round_style\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
