// comparison.cpp: blockdecimal ordering against native integer ordering
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1474. For BID, operator< compared the stored magnitudes with
// blockbinary's operator<, which treats the top storage bit as a sign bit even for an
// Unsigned blockbinary, so every magnitude >= 2^(nbits-1) ordered as if negative: about
// a quarter of all blockdecimal<3,BID> pairs compared wrong. Every encoding is checked
// here against the ordering of the integers they hold.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <universal/internal/blockdecimal/blockdecimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

template<typename B>
int CheckOrdering(const B& a, const B& b, bool lt, bool eq, const std::string& what, bool reportTestCases) {
	const bool gt = !lt && !eq;
	const bool ok = (a < b) == lt && (a <= b) == (lt || eq) && (a > b) == gt && (a >= b) == (gt || eq)
	             && (a == b) == eq && (a != b) == !eq;
	if (!ok && reportTestCases) std::cerr << "FAIL: " << what << '\n';
	return ok ? 0 : 1;
}

// every (x, y) with |x|, |y| < 10^N on a stride; stride 1 is exhaustive
template<unsigned N, DecimalEncoding E, typename BT>
int VerifyOrdering(long long stride, bool reportTestCases) {
	using B = blockdecimal<N, E, BT>;
	long long lim = 1;
	for (unsigned i = 0; i < N; ++i) lim *= 10;
	int nrOfFailedTests = 0;
	for (long long x = -(lim - 1); x < lim; x += stride) {
		for (long long y = -(lim - 1); y < lim; y += stride) {
			nrOfFailedTests += CheckOrdering(B(x), B(y), x < y, x == y, std::to_string(x) + " vs " + std::to_string(y), reportTestCases);
		}
	}
	return nrOfFailedTests;
}

// non-negative magnitudes on both sides of the top storage bit, up to the 19-digit
// BID limit where that bit is bit 63
template<unsigned N, DecimalEncoding E, typename BT>
int VerifyTopBitOrdering(bool reportTestCases) {
	using B = blockdecimal<N, E, BT>;
	constexpr unsigned nbits = B::nbits;
	std::uint64_t maxmag = 1;
	for (unsigned i = 0; i < N; ++i) maxmag *= 10;
	maxmag -= 1;
	const std::uint64_t top = std::uint64_t(1) << (nbits - 1);
	const std::uint64_t cases[] = { 0, 1, 100, top - 1, top, top + 1, maxmag - 1, maxmag };
	int nrOfFailedTests = 0;
	for (std::uint64_t x : cases) {
		for (std::uint64_t y : cases) {
			if (x > maxmag || y > maxmag) continue;
			B a(static_cast<unsigned long long>(x)), b(static_cast<unsigned long long>(y));
			nrOfFailedTests += CheckOrdering(a, b, x < y, x == y, std::to_string(x) + " vs " + std::to_string(y), reportTestCases);
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdecimal comparison (#1474)";
	std::string test_tag    = "comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockdecimal<3, DecimalEncoding::BID, std::uint8_t> a(600), b(100);
		std::cout << "600 < 100 : " << (a < b) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::BCD, std::uint8_t >(3, reportTestCases), "blockdecimal<3,BCD>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::DPD, std::uint8_t >(3, reportTestCases), "blockdecimal<3,DPD>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::BID, std::uint8_t >(3, reportTestCases), "blockdecimal<3,BID,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::BID, std::uint16_t>(3, reportTestCases), "blockdecimal<3,BID,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitOrdering<3,  DecimalEncoding::BID, std::uint8_t >(reportTestCases), "blockdecimal<3,BID> top bit", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitOrdering<6,  DecimalEncoding::BID, std::uint32_t>(reportTestCases), "blockdecimal<6,BID> top bit", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitOrdering<9,  DecimalEncoding::BID, std::uint32_t>(reportTestCases), "blockdecimal<9,BID> top bit", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitOrdering<19, DecimalEncoding::BID, std::uint64_t>(reportTestCases), "blockdecimal<19,BID> top bit", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<6, DecimalEncoding::BID, std::uint32_t>(4999, reportTestCases), "blockdecimal<6,BID>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// exhaustive over 3 digits: every ordered pair, both signs
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::BID, std::uint8_t>(1, reportTestCases), "blockdecimal<3,BID> exhaustive", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::BCD, std::uint8_t>(1, reportTestCases), "blockdecimal<3,BCD> exhaustive", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<3, DecimalEncoding::DPD, std::uint8_t>(1, reportTestCases), "blockdecimal<3,DPD> exhaustive", test_tag);
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
