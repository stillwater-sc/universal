// nan_conversion.cpp: verify that every IEEE-754 NaN encoding converts to a blocktriple NaN
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// blocktriple carried the same NaN classification defect cfloat did (issue
// #1303), and it is the most consequential of the four places that defect was
// copy-pasted to: blocktriple is what the arithmetic paths run on, so a NaN
// operand entering an add or a multiply was not merely mis-encoded, it stopped
// being a NaN at all.  Unmatched payloads fell through to the numeric path and
// came out as FINITE values -- not even the infinity the conversion path
// produced -- so a NaN silently stopped propagating.
//
// The classification compared the source fraction for equality against three
// specific payloads, so every other payload, including the canonical signalling
// 0x1, missed all three.  std::nan("") is one of the three, which is why nothing
// noticed.
//
// The sources here are built as BIT PATTERNS through sw::bit_cast, never with
// std::nan or a computed double: building a signaling NaN in a double and passing
// it reproduces on Linux and macOS but NOT on Windows, where the payload is
// quieted first and the obvious test passes while the bug remains.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <cstdint>
#include <universal/utility/bit_cast.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// binary16 pattern -> float pattern, by moving bits: the exponent stays all ones
// and the 10-bit fraction shifts up 13 places, so the quiet bit stays the quiet
// bit and a non-zero payload stays non-zero.
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

template<typename BlockTriple>
int VerifyDoubleNaNPayloads(bool reportTestCases) {
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
	for (uint64_t bits : payloads) {
		BlockTriple t;
		t = sw::bit_cast<double>(bits);
		if (!t.isnan() || t.isinf()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL double NaN 0x" << std::hex << bits << std::dec
				          << " -> isnan=" << t.isnan() << " isinf=" << t.isinf() << '\n';
			}
		}
	}
	// and the infinities must stay infinities, distinct from the NaN class
	for (uint64_t bits : { 0x7FF0000000000000ull, 0xFFF0000000000000ull }) {
		BlockTriple t;
		t = sw::bit_cast<double>(bits);
		if (t.isnan() || !t.isinf()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL double inf 0x" << std::hex << bits << std::dec
				          << " -> isnan=" << t.isnan() << " isinf=" << t.isinf() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

template<typename BlockTriple>
int VerifyBinary16NaNSpace(bool reportTestCases) {
	int nrOfFailedTests = 0;
	long nans = 0;
	for (uint32_t i = 0; i < 0x10000u; ++i) {
		const uint16_t h = static_cast<uint16_t>(i);
		if (((h & 0x7C00u) != 0x7C00u) || ((h & 0x03FFu) == 0u)) continue;   // NaNs only
		++nans;
		BlockTriple t;
		t = sw::bit_cast<float>(half_bits_to_float_bits(h));
		if (!t.isnan()) {
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

	std::string test_suite  = "blocktriple NaN conversion validation";
	std::string test_tag    = "nan conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using bt23 [[maybe_unused]] = blocktriple<23, BlockTripleOperator::REP, uint32_t>;
	using bt52 [[maybe_unused]] = blocktriple<52, BlockTripleOperator::REP, uint32_t>;
	using bt10 [[maybe_unused]] = blocktriple<10, BlockTripleOperator::REP, uint16_t>;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleNaNPayloads<bt23>(true), "blocktriple<23>", "double NaN payloads");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary16NaNSpace<bt23>(reportTestCases), "blocktriple<23>", "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleNaNPayloads<bt52>(reportTestCases), "blocktriple<52>", "double NaN payloads");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary16NaNSpace<bt52>(reportTestCases), "blocktriple<52>", "binary16 NaN space");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleNaNPayloads<bt10>(reportTestCases), "blocktriple<10>", "double NaN payloads");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary16NaNSpace<bt10>(reportTestCases), "blocktriple<10>", "binary16 NaN space");
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
