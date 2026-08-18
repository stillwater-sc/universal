// nan_conversion.cpp: verify that every IEEE-754 NaN encoding converts to an dbns NaN
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dbns carried the same NaN classification defect cfloat did (issue #1303): the
// conversion compared the source fraction for equality against three specific
// payloads, so every other payload -- including the canonical signalling 0x1 --
// missed all three and fell through to the numeric path, where a NaN silently
// became a finite value or an infinity.
//
// The source values here are built as BIT PATTERNS and handed over through
// sw::bit_cast, never through std::nan or a computed double.  Building a
// signaling NaN in a double and passing it in reproduces on Linux and macOS but
// NOT on Windows, where the payload is quieted before it reaches the conversion
// and the obvious test passes while the bug remains.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <cstdint>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// binary16 pattern -> float pattern, by moving bits: exponent all ones stays all
// ones and the 10-bit fraction shifts up 13 places, so the quiet bit stays the
// quiet bit and a non-zero payload stays non-zero.
constexpr uint32_t half_bits_to_float_bits(uint16_t h) noexcept {
	const uint32_t sign = static_cast<uint32_t>(h >> 15) << 31;
	const uint32_t exp  = static_cast<uint32_t>((h >> 10) & 0x1Fu);
	const uint32_t frac = static_cast<uint32_t>(h & 0x3FFu);
	if (exp == 0x1Fu) return sign | 0x7F800000u | (frac << 13);
	if (exp == 0u) {
		if (frac == 0u) return sign;
		uint32_t f = frac, e = 0;
		while ((f & 0x400u) == 0u) { f <<= 1; ++e; }
		f &= 0x3FFu;
		return sign | ((127u - 15u - e) << 23) | (f << 13);
	}
	return sign | ((exp + 127u - 15u) << 23) | (frac << 13);
}

template<typename Number>
int VerifyBinary16NaNSpace(bool reportTestCases) {
	int nrOfFailedTests = 0;
	long nans = 0;

	for (uint32_t i = 0; i < 0x10000u; ++i) {
		const uint16_t h = static_cast<uint16_t>(i);
		if (((h & 0x7C00u) != 0x7C00u) || ((h & 0x03FFu) == 0u)) continue;   // NaNs only
		++nans;
		const Number v(sw::bit_cast<float>(half_bits_to_float_bits(h)));
		if (!v.isnan()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL binary16 NaN 0x" << std::hex << h << std::dec
				          << " did not convert to a NaN\n";
			}
		}
	}
	if (nans != 2046) {
		++nrOfFailedTests;
		std::cout << "FAIL binary16 sweep covered " << nans << " NaNs, expected 2046\n";
	}
	return nrOfFailedTests;
}

// A handful of double-sourced payloads, including the two the old code happened
// to recognise, so the contrast is on the record.
template<typename Number>
int VerifyDoublePayloads(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const uint64_t payloads[] = {
		0x7FF0000000000001ull,   // signalling, canonical payload 1
		0x7FF0004000000000ull,   // signalling, other payload
		0x7FF4000000000000ull,   // the platform's signalling pattern
		0x7FF8000000000000ull,   // the platform's quiet pattern
		0x7FFFFFFFFFFFFFFFull,   // all payload bits set
		0xFFF0000000000001ull,   // negative signalling
		0xFFF8000000000000ull,   // negative quiet
	};
	// No quiet / signalling assertion here, unlike cfloat's and blocktriple's
	// suites: dbns has a SINGLE NaN encoding and isnan() takes no kind, so the
	// source's quiet bit has nowhere to land and there is nothing to check.  Saying
	// so beats adding an assertion that could only ever be vacuous.
	for (uint64_t bits : payloads) {
		const Number v(sw::bit_cast<double>(bits));
		if (!v.isnan()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL double NaN 0x" << std::hex << bits << std::dec
				          << " did not convert to a NaN\n";
			}
		}
	}
	// The infinities must stay OUT of the NaN class.  This type has no infinity
	// encoding -- setinf() saturates to maxpos / maxneg by design, and isinf() is
	// therefore always false -- so the invariant worth asserting is that an
	// infinite input saturates rather than being swept into a NaN, which is what a
	// classification that mixed the two would do.
	{
		const Number pos(sw::bit_cast<double>(0x7FF0000000000000ull));
		const Number neg(sw::bit_cast<double>(0xFFF0000000000000ull));
		const Number maxpos(sw::universal::SpecificValue::maxpos);
		const Number maxneg(sw::universal::SpecificValue::maxneg);
		if (pos.isnan() || neg.isnan()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL an infinity converted into the NaN class\n";
		}
		if (pos != maxpos || neg != maxneg) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL an infinity did not saturate to maxpos / maxneg\n";
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

	std::string test_suite  = "dbns NaN conversion validation";
	std::string test_tag    = "nan conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDoublePayloads<dbns<16, 5>>(true), "dbns<16,5>", "double NaN payloads");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<dbns<16, 5>>(reportTestCases), "dbns<16,5>", "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<dbns<8, 3>>(reportTestCases), "dbns<8,3>", "binary16 NaN space");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<dbns<10, 4>>(reportTestCases), "dbns<10,4>", "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyDoublePayloads<dbns<8, 3>>(reportTestCases), "dbns<8,3>", "double NaN payloads");
	nrOfFailedTestCases += ReportTestResult(
		VerifyDoublePayloads<dbns<10, 4>>(reportTestCases), "dbns<10,4>", "double NaN payloads");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinary16NaNSpace<dbns<10, 3>>(reportTestCases), "dbns<10,3>", "binary16 NaN space");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
