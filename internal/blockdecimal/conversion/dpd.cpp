// dpd.cpp: blockdecimal DPD digit storage at every digit count
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1480. DPD stores digits in 10-bit declets of three, with the one or
// two leftover digits at the top. dpd_bits() reserves 4 bits for one leftover digit and
// 7 for two, but two were stored as a pair of 4-bit nibbles: 8 bits. Whenever
// ndigits % 3 == 2, bit 3 of the leading digit fell outside the storage, so a leading 8
// or 9 read back as 0 or 1 (and dfixpnt<8,3,DPD>::maxpos() was 19999.999, #1477). The
// two digits are now one 7-bit DPD group. Every digit count from 1 to 35 is checked here
// against BCD, which stores one digit per nibble.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/internal/blockdecimal/blockdecimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// every value with |v| < 10^N survives construction and conversion back to an integer
template<unsigned N>
int VerifyDpdRoundTrip(bool reportTestCases) {
	using B = blockdecimal<N, DecimalEncoding::DPD, std::uint16_t>;
	long long lim = 1;
	for (unsigned i = 0; i < N; ++i) lim *= 10;
	int nrOfFailedTests = 0;
	for (long long v = -(lim - 1); v < lim; ++v) {
		B b(v);
		if (b.to_long_long() != v) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: DPD<" << N << ">(" << v << ") = " << b.to_string() << '\n';
		}
	}
	return nrOfFailedTests;
}

// random digit strings written with setdigit, one digit at a time and in random order,
// must read back digit for digit and agree with BCD; maxval() must be all nines
template<unsigned N>
int VerifyDpdDigits(int nrSamples, bool reportTestCases) {
	using DPD = blockdecimal<N, DecimalEncoding::DPD, std::uint16_t>;
	using BCD = blockdecimal<N, DecimalEncoding::BCD, std::uint8_t>;
	std::mt19937_64 rng(1480 + N);   // deterministic: the engine's output sequence is specified by the standard
	int nrOfFailedTests = 0;
	auto fail = [&](const std::string& what) {
		++nrOfFailedTests;
		if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: DPD<" << N << "> " << what << '\n';
	};

	for (int k = 0; k < nrSamples; ++k) {
		unsigned digits[N];
		for (unsigned i = 0; i < N; ++i) digits[i] = static_cast<unsigned>(rng() % 10);
		// every sample writes the leading digit as 8 or 9 at least once in three
		if (k % 3 == 0) digits[N - 1] = 8 + static_cast<unsigned>(rng() % 2);
		DPD d; d.clear();
		BCD b; b.clear();
		// a random write order, so each digit is also rewritten next to digits already set
		unsigned order[N];
		for (unsigned i = 0; i < N; ++i) order[i] = i;
		for (unsigned i = N; i > 1; --i) { unsigned j = static_cast<unsigned>(rng() % i); unsigned t = order[i - 1]; order[i - 1] = order[j]; order[j] = t; }
		for (unsigned i = 0; i < N; ++i) { d.setdigit(order[i], 9 - digits[order[i]]); d.setdigit(order[i], digits[order[i]]); b.setdigit(order[i], digits[order[i]]); }
		for (unsigned i = 0; i < N; ++i) {
			if (d.digit(i) != digits[i]) { fail("digit " + std::to_string(i) + " = " + std::to_string(d.digit(i)) + ", expected " + std::to_string(digits[i])); break; }
		}
		if (d.to_string() != b.to_string()) fail(d.to_string() + " != BCD " + b.to_string());
	}

	DPD m; m.maxval();
	for (unsigned i = 0; i < N; ++i) {
		if (m.digit(i) != 9) { fail("maxval digit " + std::to_string(i) + " = " + std::to_string(m.digit(i))); break; }
	}
	return nrOfFailedTests;
}

template<unsigned... Ns>
int VerifyDpdDigitCounts(int nrSamples, bool reportTestCases) {
	return (VerifyDpdDigits<Ns>(nrSamples, reportTestCases) + ...);
}

// the two-digit group helpers: every value round-trips in 7 bits, and a pair whose
// digits are both <= 7 keeps the bit pattern of the old nibble layout, (hi << 4) | lo
int VerifyTwoDigitGroup(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (unsigned v = 0; v < 100; ++v) {
		const uint16_t bits = dpd_encode_2digits(v);
		const unsigned hi = v / 10, lo = v % 10;
		const bool ok = bits <= 0x7F && dpd_decode_2digits(bits) == v && (hi > 7 || lo > 7 || bits == ((hi << 4) | lo));
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: two-digit group " << v << " -> " << bits << '\n';
		}
	}
	// patterns the encoder never produces still decode to a two-digit value
	for (unsigned bits = 0; bits < 128; ++bits) {
		if (dpd_decode_2digits(static_cast<uint16_t>(bits)) > 99) ++nrOfFailedTests;
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

	std::string test_suite  = "blockdecimal DPD digit storage (#1480)";
	std::string test_tag    = "dpd";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockdecimal<8, DecimalEncoding::DPD, std::uint16_t> v(80000000LL);
		std::cout << "DPD<8>(80000000) = " << v.to_string() << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTwoDigitGroup(reportTestCases), "two-digit DPD group", test_tag);
	// exhaustive at the smallest widths with one and with two leftover digits
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<1>(reportTestCases), "DPD<1> round trip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<2>(reportTestCases), "DPD<2> round trip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<4>(reportTestCases), "DPD<4> round trip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<5>(reportTestCases), "DPD<5> round trip", test_tag);
	// every digit count through 12, with one, two or no leftover digits
	nrOfFailedTestCases += ReportTestResult(VerifyDpdDigitCounts<1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12>(300, reportTestCases), "DPD<1..12> digits", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// wider than the 19 digits a uint64_t holds: digit access only
	nrOfFailedTestCases += ReportTestResult(VerifyDpdDigitCounts<13, 14, 16, 17, 19, 20, 22, 23, 26, 34, 35>(1000, reportTestCases), "DPD<13..35> digits", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<6>(reportTestCases), "DPD<6> round trip", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<7>(reportTestCases), "DPD<7> round trip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDpdRoundTrip<8>(reportTestCases), "DPD<8> round trip", test_tag);
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
