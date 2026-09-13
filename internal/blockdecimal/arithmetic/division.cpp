// division.cpp: blockdecimal quotient and remainder against native integer division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1474. BID long division compared magnitudes with blockbinary's
// operator<, which reads the top storage bit as a sign bit even for an Unsigned
// blockbinary: every magnitude >= 2^(nbits-1) looked negative. blockdecimal<3,BID>
// gave 518/52 = 0, 600/700 = 116, and 1/512 never returned. BCD and DPD, which
// compare digit by digit, were exact. Every encoding is checked here against C++
// integer / and %, which truncate toward zero exactly as blockdecimal does.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <universal/internal/blockdecimal/blockdecimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// signed value of a blockdecimal as a magnitude and a sign, so 19-digit BID values
// above LLONG_MAX compare exactly (no __int128: MSVC has none)
template<unsigned N, DecimalEncoding E, typename BT>
bool matches(const blockdecimal<N, E, BT>& v, bool negative, std::uint64_t magnitude) {
	if (magnitude == 0) return v.iszero();
	return v.to_uint64() == magnitude && v.isneg() == negative;
}

// every (x, y) with |x|, |y| < 10^N on a stride; stride 1 is exhaustive
template<unsigned N, DecimalEncoding E, typename BT>
int VerifyDivisionAndRemainder(long long xstride, long long ystride, bool reportTestCases) {
	using B = blockdecimal<N, E, BT>;
	long long lim = 1;
	for (unsigned i = 0; i < N; ++i) lim *= 10;
	int nrOfFailedTests = 0;
	for (long long x = -(lim - 1); x < lim; x += xstride) {
		for (long long y = -(lim - 1); y < lim; y += ystride) {
			if (y == 0) continue;
			B a(x), b(y);
			B q(a); q /= b;
			B r(a); r %= b;
			const long long eq = x / y, er = x % y;
			const bool qok = matches(q, eq < 0, static_cast<std::uint64_t>(eq < 0 ? -eq : eq));
			const bool rok = matches(r, er < 0, static_cast<std::uint64_t>(er < 0 ? -er : er));
			if (!qok || !rok) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 10) {
					std::cerr << "FAIL: " << x << " / " << y << " -> q " << q.to_string() << " (expected " << eq
					          << "), r " << r.to_string() << " (expected " << er << ")\n";
				}
			}
		}
	}
	return nrOfFailedTests;
}

// magnitudes at and above the top storage bit, where the signed comparison broke:
// 2^(nbits-1) and its neighbours, as dividend and as divisor
template<unsigned N, DecimalEncoding E, typename BT>
int VerifyTopBitDivision(bool reportTestCases) {
	using B = blockdecimal<N, E, BT>;
	constexpr unsigned nbits = B::nbits;
	std::uint64_t maxmag = 1;
	for (unsigned i = 0; i < N; ++i) maxmag *= 10;
	maxmag -= 1;
	const std::uint64_t top = std::uint64_t(1) << (nbits - 1);
	const std::uint64_t cases[] = { 1, 2, 7, 52, 518, top - 1, top, top + 1, top + 12345, maxmag / 3, maxmag - 1, maxmag };
	int nrOfFailedTests = 0;
	for (std::uint64_t x : cases) {
		for (std::uint64_t y : cases) {
			if (x > maxmag || y > maxmag || y == 0) continue;
			B a(static_cast<unsigned long long>(x)), b(static_cast<unsigned long long>(y));
			B q(a); q /= b;
			B r(a); r %= b;
			if (!matches(q, false, x / y) || !matches(r, false, x % y)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: " << x << " / " << y << " -> q " << q.to_string() << " (expected " << x / y
					          << "), r " << r.to_string() << " (expected " << x % y << ")\n";
				}
			}
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

	std::string test_suite  = "blockdecimal division and remainder (#1474)";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockdecimal<3, DecimalEncoding::BID, std::uint8_t> a(518), b(52), q(a);
		q /= b;
		std::cout << "518 / 52 = " << q.to_string() << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// 3 digits on a stride: every encoding, both signs, magnitudes across the top storage bit
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::BCD, std::uint8_t >(7, 11, reportTestCases), "blockdecimal<3,BCD>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::DPD, std::uint8_t >(7, 11, reportTestCases), "blockdecimal<3,DPD>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::BID, std::uint8_t >(7, 11, reportTestCases), "blockdecimal<3,BID,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::BID, std::uint16_t>(7, 11, reportTestCases), "blockdecimal<3,BID,uint16>", test_tag);
	// the top storage bit at several widths, up to the 19-digit BID limit (bit 63)
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitDivision<3,  DecimalEncoding::BID, std::uint8_t >(reportTestCases), "blockdecimal<3,BID> top bit", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitDivision<6,  DecimalEncoding::BID, std::uint32_t>(reportTestCases), "blockdecimal<6,BID> top bit", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitDivision<9,  DecimalEncoding::BID, std::uint32_t>(reportTestCases), "blockdecimal<9,BID> top bit", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTopBitDivision<19, DecimalEncoding::BID, std::uint64_t>(reportTestCases), "blockdecimal<19,BID> top bit", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<6, DecimalEncoding::BID, std::uint32_t>(9973, 997, reportTestCases), "blockdecimal<6,BID>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<6, DecimalEncoding::DPD, std::uint16_t>(9973, 997, reportTestCases), "blockdecimal<6,DPD>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// exhaustive over 3 digits: every dividend and every nonzero divisor, both signs
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::BID, std::uint8_t>(1, 1, reportTestCases), "blockdecimal<3,BID> exhaustive", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::BCD, std::uint8_t>(1, 1, reportTestCases), "blockdecimal<3,BCD> exhaustive", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAndRemainder<3, DecimalEncoding::DPD, std::uint8_t>(1, 1, reportTestCases), "blockdecimal<3,DPD> exhaustive", test_tag);
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
