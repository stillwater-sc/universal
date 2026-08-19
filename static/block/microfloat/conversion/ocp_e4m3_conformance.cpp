// ocp_e4m3_conformance.cpp: OCP 8-bit floating point (OFP8) E4M3 conformance
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// microfloat's E4M3 *encoding* was already exact -- all 256 patterns decode to
// the right value -- but conversion from a wider type diverged from the OCP
// specification in four places, and every existing suite stayed green because
// none of them looked above maxpos (universal#1302):
//
//     input          was          OCP OFP8 E4M3
//     500.0          0x7e (448)   0x7f (NaN)
//     +inf / -inf    0x7e / 0xfe  0x7f / 0xff
//     -NaN           0x7f         0xff
//     e5m2 58000.0   0x7c (inf)   0x7b (57344)
//
// This suite pins both halves.  The decode reference is a 256-entry table
// generated straight from the specification's field layout -- sign, 4-bit
// exponent biased by 7, 3-bit mantissa, subnormals at exponent code 0, NaN at
// S.1111.111, no infinity -- and the conversion reference is a brute-force
// search over that same table for the nearest representable value with
// ties-to-even.  Neither consults microfloat to decide what the answer is.
//
// The two conversion policies are both tested, because both are wanted:
// e4m3 (= e4m3fn) is the OCP format that ml_dtypes, JAX and PyTorch implement,
// and e4m3_saturating is what MX and NVFP4 block quantization needs, since
// those scale amax into a range whose top lies past e4m3's maxpos of 448.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <string>
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// The OCP OFP8 E4M3 value of every one of the 256 encodings, decoded from
	// the specification rather than from microfloat:
	//     e == 0            (-1)^s * 2^-6 * (m/8)          subnormal
	//     e == 15 && m == 7 NaN                            the only non-finite
	//     otherwise         (-1)^s * 2^(e-7) * (1 + m/8)   normal
	// The two NaN slots hold 0.0f as a placeholder; is_nan_encoding() decides
	// which entries those are.
	constexpr float e4m3_reference[256] = {
		0.0f,                  0.001953125f,          0.00390625f,           0.005859375f,
		0.0078125f,            0.009765625f,          0.01171875f,           0.013671875f,
		0.015625f,             0.017578125f,          0.01953125f,           0.021484375f,
		0.0234375f,            0.025390625f,          0.02734375f,           0.029296875f,
		0.03125f,              0.03515625f,           0.0390625f,            0.04296875f,
		0.046875f,             0.05078125f,           0.0546875f,            0.05859375f,
		0.0625f,               0.0703125f,            0.078125f,             0.0859375f,
		0.09375f,              0.1015625f,            0.109375f,             0.1171875f,
		0.125f,                0.140625f,             0.15625f,              0.171875f,
		0.1875f,               0.203125f,             0.21875f,              0.234375f,
		0.25f,                 0.28125f,              0.3125f,               0.34375f,
		0.375f,                0.40625f,              0.4375f,               0.46875f,
		0.5f,                  0.5625f,               0.625f,                0.6875f,
		0.75f,                 0.8125f,               0.875f,                0.9375f,
		1.0f,                  1.125f,                1.25f,                 1.375f,
		1.5f,                  1.625f,                1.75f,                 1.875f,
		2.0f,                  2.25f,                 2.5f,                  2.75f,
		3.0f,                  3.25f,                 3.5f,                  3.75f,
		4.0f,                  4.5f,                  5.0f,                  5.5f,
		6.0f,                  6.5f,                  7.0f,                  7.5f,
		8.0f,                  9.0f,                  10.0f,                 11.0f,
		12.0f,                 13.0f,                 14.0f,                 15.0f,
		16.0f,                 18.0f,                 20.0f,                 22.0f,
		24.0f,                 26.0f,                 28.0f,                 30.0f,
		32.0f,                 36.0f,                 40.0f,                 44.0f,
		48.0f,                 52.0f,                 56.0f,                 60.0f,
		64.0f,                 72.0f,                 80.0f,                 88.0f,
		96.0f,                 104.0f,                112.0f,                120.0f,
		128.0f,                144.0f,                160.0f,                176.0f,
		192.0f,                208.0f,                224.0f,                240.0f,
		256.0f,                288.0f,                320.0f,                352.0f,
		384.0f,                416.0f,                448.0f,                0.0f,
		-0.0f,                 -0.001953125f,         -0.00390625f,          -0.005859375f,
		-0.0078125f,           -0.009765625f,         -0.01171875f,          -0.013671875f,
		-0.015625f,            -0.017578125f,         -0.01953125f,          -0.021484375f,
		-0.0234375f,           -0.025390625f,         -0.02734375f,          -0.029296875f,
		-0.03125f,             -0.03515625f,          -0.0390625f,           -0.04296875f,
		-0.046875f,            -0.05078125f,          -0.0546875f,           -0.05859375f,
		-0.0625f,              -0.0703125f,           -0.078125f,            -0.0859375f,
		-0.09375f,             -0.1015625f,           -0.109375f,            -0.1171875f,
		-0.125f,               -0.140625f,            -0.15625f,             -0.171875f,
		-0.1875f,              -0.203125f,            -0.21875f,             -0.234375f,
		-0.25f,                -0.28125f,             -0.3125f,              -0.34375f,
		-0.375f,               -0.40625f,             -0.4375f,              -0.46875f,
		-0.5f,                 -0.5625f,              -0.625f,               -0.6875f,
		-0.75f,                -0.8125f,              -0.875f,               -0.9375f,
		-1.0f,                 -1.125f,               -1.25f,                -1.375f,
		-1.5f,                 -1.625f,               -1.75f,                -1.875f,
		-2.0f,                 -2.25f,                -2.5f,                 -2.75f,
		-3.0f,                 -3.25f,                -3.5f,                 -3.75f,
		-4.0f,                 -4.5f,                 -5.0f,                 -5.5f,
		-6.0f,                 -6.5f,                 -7.0f,                 -7.5f,
		-8.0f,                 -9.0f,                 -10.0f,                -11.0f,
		-12.0f,                -13.0f,                -14.0f,                -15.0f,
		-16.0f,                -18.0f,                -20.0f,                -22.0f,
		-24.0f,                -26.0f,                -28.0f,                -30.0f,
		-32.0f,                -36.0f,                -40.0f,                -44.0f,
		-48.0f,                -52.0f,                -56.0f,                -60.0f,
		-64.0f,                -72.0f,                -80.0f,                -88.0f,
		-96.0f,                -104.0f,               -112.0f,               -120.0f,
		-128.0f,               -144.0f,               -160.0f,               -176.0f,
		-192.0f,               -208.0f,               -224.0f,               -240.0f,
		-256.0f,               -288.0f,               -320.0f,               -352.0f,
		-384.0f,               -416.0f,               -448.0f,               0.0f,
	};

	// NaN is S.1111.111: encodings 0x7F and 0xFF
	constexpr bool is_nan_encoding(unsigned code) noexcept { return (code & 0x7Fu) == 0x7Fu; }

	// the value one step above maxpos, had the top encoding not been NaN, and
	// the round-to-nearest boundary halfway to it.  464.0 itself is a tie
	// between 448 (mantissa 110, even) and 480 (mantissa 111, odd), so it
	// rounds down to 448; anything strictly above it overflows.
	constexpr float e4m3_step_above = 480.0f;          // 448 + one ulp of 32
	constexpr float e4m3_overflow_boundary = 464.0f;   // (448 + 480) / 2

	// Brute-force OCP conversion oracle: the nearest table entry to x, ties to
	// even, NaN once x is past the boundary.  O(256) per call and completely
	// independent of the implementation under test.
	[[maybe_unused]] unsigned ocp_encode(float x) {
		unsigned sign = std::signbit(x) ? 0x80u : 0x00u;
		if (x != x) return sign | 0x7Fu;                       // NaN keeps its sign
		if (std::isinf(x)) return sign | 0x7Fu;                // no infinity: NaN
		float a = std::fabs(x);
		if (a > e4m3_overflow_boundary) return sign | 0x7Fu;   // overflow: NaN
		unsigned best = 0u;
		float bestDistance = std::numeric_limits<float>::infinity();
		for (unsigned code = 0u; code < 128u; ++code) {        // the positive half
			if (is_nan_encoding(code)) continue;
			float candidate = e4m3_reference[code];
			float distance = std::fabs(candidate - a);
			if (distance < bestDistance) {
				bestDistance = distance;
				best = code;
			}
			else if (distance == bestDistance && (code & 1u) == 0u) {
				best = code;                                   // tie: take the even one
			}
		}
		return sign | best;
	}

	// every encoding decodes to the specification's value
	template<typename Microfloat>
	int VerifyDecodeTable(bool reportTestCases, const std::string& tag) {
		int nrOfFailedTests = 0;
		for (unsigned code = 0u; code < 256u; ++code) {
			Microfloat a{};
			a.setbits(code);
			if (is_nan_encoding(code)) {
				if (!a.isnan()) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "  FAIL " << tag << " 0x" << std::hex << code
					                               << std::dec << " should decode to NaN\n";
				}
				continue;
			}
			float decoded = float(a);
			if (decoded != e4m3_reference[code] || std::signbit(decoded) != std::signbit(e4m3_reference[code])) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " 0x" << std::hex << code << std::dec
				                               << " decoded " << decoded << " expected "
				                               << e4m3_reference[code] << '\n';
			}
		}
		return nrOfFailedTests;
	}

	[[maybe_unused]] int check(bool reportTestCases, const std::string& tag, float input, unsigned encoding, unsigned expected) {
		if (encoding == expected) return 0;
		if (reportTestCases) std::cerr << "  FAIL " << tag << " (" << input << ") -> 0x" << std::hex
		                               << encoding << " expected 0x" << expected << std::dec << '\n';
		return 1;
	}

	// the four divergences of universal#1302, stated as the issue states them
	[[maybe_unused]] int VerifyOcpDivergences(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		float inf = std::numeric_limits<float>::infinity();
		struct { float input; unsigned expected; } cases[] = {
			{ 448.0f,        0x7Eu },   // maxpos, unchanged
			{ 464.0f,        0x7Eu },   // the tie, rounds down to even
			{ 465.0f,        0x7Fu },   // first value past the boundary
			{ 480.0f,        0x7Fu },   // the encoding the NaN slot took
			{ 500.0f,        0x7Fu },
			{ 1e30f,         0x7Fu },
			{ -1e30f,        0xFFu },
			{ inf,           0x7Fu },
			{ -inf,          0xFFu },
			{ std::numeric_limits<float>::quiet_NaN(),  0x7Fu },
			{ -std::numeric_limits<float>::quiet_NaN(), 0xFFu },
		};
		for (auto& c : cases) {
			e4m3 a(c.input);
			nrOfFailedTests += check(reportTestCases, "e4m3", c.input, a.bits(), c.expected);
		}
		return nrOfFailedTests;
	}

	// conversion against the brute-force oracle, over the values that decide it:
	// every representable value, every midpoint between neighbours, and a step
	// either side of every midpoint
	[[maybe_unused]] int VerifyConversionAgainstOracle(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		auto probe = [&](float x) {
			e4m3 a(x);
			nrOfFailedTests += check(reportTestCases, "e4m3 oracle", x, a.bits(), ocp_encode(x));
		};
		for (unsigned code = 0u; code < 128u; ++code) {
			if (is_nan_encoding(code)) continue;
			float v = e4m3_reference[code];
			float next = (code == 0x7Eu) ? e4m3_step_above : e4m3_reference[code + 1u];
			if (is_nan_encoding(code + 1u)) next = e4m3_step_above;
			float mid = 0.5f * (v + next);
			for (float x : { v, mid, std::nextafter(mid, 0.0f), std::nextafter(mid, 1e30f) }) {
				probe(x);
				probe(-x);
			}
		}
		return nrOfFailedTests;
	}

	// the saturating policy stays reachable and keeps clamping: MX and NVFP4
	// quantization depend on it
	[[maybe_unused]] int VerifySaturatingPolicy(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		float inf = std::numeric_limits<float>::infinity();
		struct { float input; unsigned expected; } cases[] = {
			{ 464.0f, 0x7Eu }, { 500.0f, 0x7Eu }, { 1e30f, 0x7Eu },
			{ -1e30f, 0xFEu }, { inf, 0x7Eu }, { -inf, 0xFEu },
		};
		for (auto& c : cases) {
			e4m3_saturating a(c.input);
			nrOfFailedTests += check(reportTestCases, "e4m3_saturating", c.input, a.bits(), c.expected);
		}
		// the encoding is the same format: it must decode identically
		nrOfFailedTests += VerifyDecodeTable<e4m3_saturating>(reportTestCases, "e4m3_saturating");
		return nrOfFailedTests;
	}

	// e5m2 is IEEE-like, so the top of its range rounds to nearest against
	// infinity: 57344 is maxpos, 61440 is the tie and rounds up to inf because
	// the significand of 2^16 is the even one.  The pre-clamp used to send
	// every value above maxpos to infinity.
	[[maybe_unused]] int VerifyE5m2Boundary(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		float inf = std::numeric_limits<float>::infinity();
		struct { float input; unsigned expected; } cases[] = {
			{ 57344.0f, 0x7Bu },   // maxpos
			{ 58000.0f, 0x7Bu },   // rounds back down to maxpos
			{ 61439.0f, 0x7Bu },   // still below the tie
			{ 61440.0f, 0x7Cu },   // the tie, rounds up to infinity
			{ 61441.0f, 0x7Cu },
			{ 1e30f,    0x7Cu },
			{ -1e30f,   0xFCu },
			{ inf,      0x7Cu },
			{ -inf,     0xFCu },
		};
		for (auto& c : cases) {
			e5m2 a(c.input);
			nrOfFailedTests += check(reportTestCases, "e5m2", c.input, a.bits(), c.expected);
		}
		// a negative NaN stays a negative quiet NaN
		e5m2 n(-std::numeric_limits<float>::quiet_NaN());
		if (!n.isnan() || !n.isneg()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "  FAIL e5m2 (-NaN) lost its sign\n";
		}
		return nrOfFailedTests;
	}

	// an infinite source must never convert to zero.  It did for every
	// non-saturating configuration without infinity, which turned an overflow
	// into the most benign value in the format.
	template<typename Microfloat>
	int VerifyInfiniteSourceIsNeverZero(bool reportTestCases, const std::string& tag) {
		int nrOfFailedTests = 0;
		float inf = std::numeric_limits<float>::infinity();
		for (float x : { inf, -inf }) {
			Microfloat a(x);
			if (a.iszero()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " (" << x << ") converted to zero\n";
			}
			if (a.isneg() != std::signbit(x)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " (" << x << ") lost its sign\n";
			}
		}
		return nrOfFailedTests;
	}

}  // anonymous namespace

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

	std::string test_suite  = "OCP OFP8 E4M3 conformance";
	std::string test_tag    = "conformance";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyOcpDivergences(true), test_tag, "e4m3 divergences");
	nrOfFailedTestCases += ReportTestResult(VerifyConversionAgainstOracle(true), test_tag, "e4m3 vs oracle");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDecodeTable<e4m3>(reportTestCases, "e4m3"), test_tag, "e4m3 decode, all 256");
	nrOfFailedTestCases += ReportTestResult(VerifyOcpDivergences(reportTestCases), test_tag, "e4m3 conversion, OCP cases");
	nrOfFailedTestCases += ReportTestResult(VerifyConversionAgainstOracle(reportTestCases), test_tag, "e4m3 conversion, vs oracle");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingPolicy(reportTestCases), test_tag, "e4m3_saturating policy");
	nrOfFailedTestCases += ReportTestResult(VerifyE5m2Boundary(reportTestCases), test_tag, "e5m2 overflow boundary");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<e2m1>(reportTestCases, "e2m1"), test_tag, "e2m1 infinite source");
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<e2m3>(reportTestCases, "e2m3"), test_tag, "e2m3 infinite source");
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<e3m2>(reportTestCases, "e3m2"), test_tag, "e3m2 infinite source");
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<e4m3>(reportTestCases, "e4m3"), test_tag, "e4m3 infinite source");
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<e4m3_saturating>(reportTestCases, "e4m3_saturating"), test_tag, "e4m3_saturating infinite source");
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<e5m2>(reportTestCases, "e5m2"), test_tag, "e5m2 infinite source");
	// the configurations without an alias: no infinity and no NaN to fall back on
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<microfloat<8, 4, false, false, false>>(reportTestCases, "microfloat<8,4,-,-,->"), test_tag, "no inf, no nan, no saturation");
	nrOfFailedTestCases += ReportTestResult(VerifyInfiniteSourceIsNeverZero<microfloat<6, 3, false, true, false>>(reportTestCases, "microfloat<6,3,-,nan,->"), test_tag, "no inf, nan, no saturation");
#endif

#if REGRESSION_LEVEL_4
	// a dense sweep of the whole finite range against the oracle, at a step
	// well below the smallest subnormal
	{
		int nrOfFailedTests = 0;
		for (float x = 0.0f; x <= 520.0f; x += 0.000244140625f) {   // 2^-12
			e4m3 a(x), b(-x);
			nrOfFailedTests += check(reportTestCases, "e4m3 sweep", x, a.bits(), ocp_encode(x));
			nrOfFailedTests += check(reportTestCases, "e4m3 sweep", -x, b.bits(), ocp_encode(-x));
		}
		nrOfFailedTestCases += ReportTestResult(nrOfFailedTests, test_tag, "e4m3 dense sweep");
	}
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
