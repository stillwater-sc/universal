// nan_conversion.cpp: verify that every IEEE-754 NaN encoding converts to a cfloat NaN
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Converting a signaling NaN into a narrower cfloat produced INFINITY (issue
// #1303).  The classification compared the source fraction for equality against
// three specific payloads -- the platform's canonical quiet and signalling
// patterns and their union -- so every other payload, including the canonical
// 0x1, missed all three, fell through to the numeric path, and was projected to
// infinity by the out-of-range logic.  std::nan("") happens to be one of the
// three, which is why the existing coverage never saw it.
//
// A NaN silently becoming +-inf is worse than a failed conversion: it turns a
// value that would have propagated and been detectable into a plausible one that
// survives arithmetic and round-trips.  Downstream, 2040 of binary16's 2046 NaN
// encodings decoded to the wrong CLASS of value.
//
// HOW THIS SUITE IS BUILT, which matters more than usual here
//
// Do NOT reach for std::nan variants or a memcpy-constructed double.  Building a
// signaling NaN in a double and passing it in reproduces on Linux and macOS but
// NOT on Windows, where the sNaN is quieted before it reaches the conversion --
// the obvious test passes on MSVC while the bug remains.  Reported on the issue
// after it cost a red CI run.
//
// So the source values here are built as BIT PATTERNS and handed over through
// sw::bit_cast, with no arithmetic in between that could quiet a payload, and the
// classification itself is additionally exercised through integer arguments that
// never touch a floating-point register at all.  The two layers fail
// independently: the integer one pins the logic on every platform, the bit-cast
// one pins the whole conversion path on platforms that deliver the payload.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <cstdint>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// ---------------------------------------------------------------------------
// Layer 1: the classification, on integer fields only
//
// convert_ieee754_special() takes the decoded sign / exponent / fraction, so this
// walks the payload space without ever putting a NaN in a register.  Nothing a
// platform does to signaling NaNs can make this vacuous.
// ---------------------------------------------------------------------------
template<typename Cfloat, typename Real>
int VerifyClassification(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	constexpr uint64_t eallset  = ieee754_parameter<Real>::eallset;
	constexpr uint64_t fmask    = ieee754_parameter<Real>::fmask;
	constexpr uint64_t quietbit = fmask & ieee754_parameter<Real>::qnanmask;

	auto expect = [&](const char* what, bool sign, uint64_t e, uint64_t f,
	                  bool wantSpecial, bool wantNaN, bool wantInf) {
		Cfloat c{};
		const bool handled = c.template convert_ieee754_special<Real>(sign, e, f);
		if (handled != wantSpecial || (wantSpecial && (c.isnan() != wantNaN || c.isinf() != wantInf))) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << what << " e=0x" << std::hex << e << " f=0x" << f << std::dec
				          << ": handled=" << handled << " isnan=" << c.isnan() << " isinf=" << c.isinf()
				          << " (wanted handled=" << wantSpecial << " isnan=" << wantNaN
				          << " isinf=" << wantInf << ")\n";
			}
		}
	};

	// infinity: exponent all ones, fraction zero
	expect("+inf", false, eallset, 0ull, true, false, true);
	expect("-inf", true,  eallset, 0ull, true, false, true);

	// a finite exponent is not a special at all, whatever the fraction
	expect("finite", false, 1ull, 0ull, false, false, false);
	expect("finite", false, eallset - 1ull, quietbit, false, false, false);

	// EVERY non-zero fraction is a NaN.  The walk covers each single-bit payload,
	// which is where the old code failed: only three exact values were recognised.
	for (unsigned b = 0; b < 64u; ++b) {
		const uint64_t f = (1ull << b) & fmask;
		if (f == 0ull) continue;
		expect("single-bit payload", false, eallset, f, true, true, false);
		expect("single-bit payload", true,  eallset, f, true, true, false);
	}

	// and the quiet bit decides WHICH NaN: set is quiet, clear is signalling.
	// cfloat encodes a quiet NaN with sign 0 and a signalling NaN with sign 1.
	{
		Cfloat q{}, sn{};
		q.template convert_ieee754_special<Real>(false, eallset, quietbit);
		sn.template convert_ieee754_special<Real>(false, eallset, 1ull);
		if (!q.isnan(NAN_TYPE_QUIET)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL quiet bit set did not yield a quiet NaN\n";
		}
		if (!sn.isnan(NAN_TYPE_SIGNALLING)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL quiet bit clear did not yield a signalling NaN\n";
		}
	}
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// Layer 2: the whole conversion, driven from binary16 encodings
//
// This is the downstream repro in C++: walk all 65536 binary16 patterns, widen
// each to a float by MOVING BITS -- exponent all ones stays all ones and the
// 10-bit fraction shifts up 13 places, so the quiet bit stays the quiet bit and a
// non-zero payload stays non-zero -- and convert.  No arithmetic touches the
// value, so a signaling payload arrives intact.
// ---------------------------------------------------------------------------
constexpr uint32_t half_bits_to_float_bits(uint16_t h) noexcept {
	const uint32_t sign = static_cast<uint32_t>(h >> 15) << 31;
	const uint32_t exp  = static_cast<uint32_t>((h >> 10) & 0x1Fu);
	const uint32_t frac = static_cast<uint32_t>(h & 0x3FFu);
	if (exp == 0x1Fu) return sign | 0x7F800000u | (frac << 13);   // inf / NaN
	if (exp == 0u) {
		if (frac == 0u) return sign;                              // +-0
		// subnormal: normalize into float's wider exponent range
		uint32_t f = frac, e = 0;
		while ((f & 0x400u) == 0u) { f <<= 1; ++e; }
		f &= 0x3FFu;
		return sign | ((127u - 15u - e) << 23) | (f << 13);
	}
	return sign | ((exp + 127u - 15u) << 23) | (frac << 13);
}

template<typename Cfloat>
int VerifyBinary16NaNSpace(bool reportTestCases) {
	int nrOfFailedTests = 0;
	long nans = 0, infs = 0;

	for (uint32_t i = 0; i < 0x10000u; ++i) {
		const uint16_t h = static_cast<uint16_t>(i);
		const bool srcIsNaN = ((h & 0x7C00u) == 0x7C00u) && ((h & 0x03FFu) != 0u);
		const bool srcIsInf = ((h & 0x7C00u) == 0x7C00u) && ((h & 0x03FFu) == 0u);
		if (!srcIsNaN && !srcIsInf) continue;

		const float f = sw::bit_cast<float>(half_bits_to_float_bits(h));
		const Cfloat c(f);

		if (srcIsNaN) {
			++nans;
			if (!c.isnan()) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL binary16 0x" << std::hex << h << std::dec
					          << " is a NaN but converted to "
					          << (c.isinf() ? "infinity" : "a finite value") << '\n';
				}
			}
			else {
				// and it must be the RIGHT KIND of NaN.  Asserting only "is a NaN"
				// would pass a conversion that discarded the source's quiet bit, or
				// reversed it -- which is the very field this fix reads.  binary16's
				// quiet bit is bit 9 of the fraction.
				const bool srcQuiet = (h & 0x0200u) != 0u;
				const bool gotQuiet = c.isnan(sw::universal::NAN_TYPE_QUIET);
				if (gotQuiet != srcQuiet) {
					++nrOfFailedTests;
					if (reportTestCases) {
						std::cout << "FAIL binary16 0x" << std::hex << h << std::dec
						          << " is a " << (srcQuiet ? "quiet" : "signalling")
						          << " NaN but converted to a "
						          << (gotQuiet ? "quiet" : "signalling") << " one\n";
					}
				}
			}
		}
		else {
			++infs;
			if (!c.isinf()) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL binary16 0x" << std::hex << h << std::dec
					          << " is an infinity but did not convert to one\n";
				}
			}
		}
	}
	// binary16 has 2046 NaN encodings and 2 infinities; if the sweep saw a
	// different number it is not testing what it claims to.
	if (nans != 2046 || infs != 2) {
		++nrOfFailedTests;
		std::cout << "FAIL binary16 sweep covered " << nans << " NaNs and " << infs
		          << " infinities, expected 2046 and 2\n";
	}
	return nrOfFailedTests;
}

// The issue's own repro, kept as a named regression.  Only meaningful on
// platforms that deliver a signaling payload through a double; where the
// platform quiets it first this degrades to a second quiet-NaN check rather than
// a false pass, because the assertion is "is a NaN", not "is a signalling NaN".
int VerifyIssue1303Repro(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	const double snan = sw::bit_cast<double>(0x7FF0000000000001ull);
	const double qnan = sw::bit_cast<double>(0x7FF8000000000000ull);

	using F16 = cfloat<16, 5, uint16_t, true, false, false>;
	const F16 fs(snan), fq(qnan);
	if (!fs.isnan()) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL sNaN double converted to "
			          << (fs.isinf() ? "infinity\n" : "a finite value\n");
		}
	}
	if (!fq.isnan()) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL qNaN double converted to "
			          << (fq.isinf() ? "infinity\n" : "a finite value\n");
		}
	}
	return nrOfFailedTests;
}

} // anonymous namespace

#define MANUAL_TESTING 0
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

	std::string test_suite  = "cfloat NaN conversion validation";
	std::string test_tag    = "nan conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// The shapes named in issue #1303.  maybe_unused because which of them a build
	// reaches depends on the regression level.
	using f16_sub    [[maybe_unused]] = cfloat<16, 5, uint16_t, true,  false, false>;
	using f16_super  [[maybe_unused]] = cfloat<16, 5, uint16_t, true,  true,  false>;
	using f16_nosub  [[maybe_unused]] = cfloat<16, 5, uint16_t, false, false, false>;
	using f16_sat    [[maybe_unused]] = cfloat<16, 5, uint16_t, true,  false, true>;
	using f8_e5m2    [[maybe_unused]] = cfloat<8,  5, uint8_t,  true,  false, false>;
	using f32_single [[maybe_unused]] = cfloat<32, 8, uint32_t, true,  false, false>;
	using f64_double [[maybe_unused]] = cfloat<64, 11, uint64_t, true, false, false>;

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyIssue1303Repro(true), "cfloat<16,5>", "issue 1303 repro");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f16_sub, double>(true)), "cfloat<16,5>", "classification");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIssue1303Repro(true), "cfloat<16,5>", "issue 1303 repro");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f16_sub, double>(reportTestCases)), "cfloat<16,5> from double", "classification");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f16_sub, float>(reportTestCases)),  "cfloat<16,5> from float",  "classification");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f16_sub>(reportTestCases), "cfloat<16,5>", "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_2
	// the flag combinations from the issue's table: subnormals, supernormals and
	// saturation are all irrelevant to the defect, and the suite says so
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f16_super>(reportTestCases), "cfloat<16,5> supernormals", "binary16 NaN space");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f16_nosub>(reportTestCases), "cfloat<16,5> no subnormals", "binary16 NaN space");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f16_sat>(reportTestCases),   "cfloat<16,5> saturating",    "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_3
	// narrower and wider targets: e5m2 is narrower than the source fraction,
	// cfloat<32,8> and cfloat<64,11> take the same-width fast paths
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f8_e5m2>(reportTestCases),    "cfloat<8,5>",   "binary16 NaN space");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f32_single>(reportTestCases), "cfloat<32,8>",  "binary16 NaN space");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<f64_double>(reportTestCases), "cfloat<64,11>", "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f8_e5m2, float>(reportTestCases)),    "cfloat<8,5>",   "classification");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f32_single, double>(reportTestCases)), "cfloat<32,8>",  "classification");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f64_double, double>(reportTestCases)), "cfloat<64,11>", "classification");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyClassification<f16_super, double>(reportTestCases)),  "cfloat<16,5> supernormals", "classification");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
