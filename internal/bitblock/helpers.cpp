// helpers.cpp: bitblock helpers round<>, to_hex, copy_slice_into and the native fraction extractors
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2053) found that round<>() did not compile (bitblock has
// no unsigned long long constructor), that to_hex() emitted one hexit too many for a multiple of
// 4 bits and one too few otherwise, that copy_slice_into() tested its bounds inverted and was a
// no-op for every valid slice, and that extract_long_double_fraction() wrote out of bounds for
// 48 < nbits < 112. Every case below failed (or did not compile) before the fix.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <universal/internal/bitblock/bitblock.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using namespace sw::universal::internal;

template<unsigned N>
bitblock<N> bits_of(unsigned long long v) {
	bitblock<N> b;
	b = v;
	return b;
}

// round<tgt>(src, n): drop the n low bits, round to nearest, ties to even
int VerifyRound(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](unsigned long long src, unsigned n, unsigned long long expected) {
		const bitblock<8> r = round<8, 12>(bits_of<12>(src), n);
		if (r.to_ullong() != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: round<8,12>(0x" << std::hex << src << ", " << std::dec << n << ") = 0x" << std::hex << r.to_ullong() << " expected 0x" << expected << std::dec << '\n';
		}
	};
	check(0x5A7, 4, 0x5A);   // guard 0: down
	check(0x5A9, 4, 0x5B);   // guard 1, sticky: up
	check(0x5A8, 4, 0x5A);   // tie, even: down
	check(0x5B8, 4, 0x5C);   // tie, odd: up
	check(0x0FF, 0, 0xFF);   // nothing dropped
	// a source wider than 64 bits: to_ullong() threw for it
	bitblock<80> wide;
	wide.set(70);
	wide.set(3);
	const bitblock<8> w = round<8, 80>(wide, 66);
	if (w.to_ullong() != 0x10) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: round<8,80> of bit 70 by 66 = 0x" << std::hex << w.to_ullong() << std::dec << '\n';
	}
	return nrOfFailedTests;
}

int VerifyToHex(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](const std::string& got, const std::string& expected) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: to_hex = " << got << " expected " << expected << '\n';
		}
	};
	check(to_hex(bits_of<8>(0xAB), false), "0xab");
	check(to_hex(bits_of<12>(0x5A7), false), "0x5a7");
	check(to_hex(bits_of<16>(0xBEEF), false), "0xbeef");
	check(to_hex(bits_of<5>(0x1F), false), "0x1f");
	check(to_hex(bits_of<6>(0x2A), false), "0x2a");
	check(to_hex(bits_of<13>(0x1ABC), false), "0x1abc");
	return nrOfFailedTests;
}

int VerifyCopySlice(bool reportTestCases) {
	int nrOfFailedTests = 0;
	bitblock<8> s = bits_of<8>(0xF0);
	bitblock<16> t;
	copy_slice_into(s, t, 4, 8, 2);   // bits 4..7 of s to bits 6..9 of t
	if (t.to_ullong() != 0x3C0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: copy_slice_into(0xF0, [4,8), shift 2) = 0x" << std::hex << t.to_ullong() << std::dec << '\n';
	}
	bitblock<16> w;   // a shift that would wrap end + shift is rejected, not applied
	copy_slice_into(s, w, 4, 8, std::numeric_limits<unsigned>::max() - 3u);
	if (w.any()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: copy_slice_into with a wrapping shift wrote 0x" << std::hex << w.to_ullong() << std::dec << '\n';
	}
	bitblock<16> u = bits_of<16>(0x8001);   // the target is not reset
	copy_slice_into(s, u, 0, 8, 4);
	if (u.to_ullong() != 0x8F01) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: copy_slice_into keeps the target's other bits: 0x" << std::hex << u.to_ullong() << std::dec << '\n';
	}
	return nrOfFailedTests;
}

// the 112 fraction bits of a binary128 long double: 48 in the upper word, 64 in the lower
template<unsigned nbits>
int VerifyLongDoubleFraction(bool reportTestCases) {
	int nrOfFailedTests = 0;
	uint128 f;
	f.upper = 0x0000'8000'0000'0001ull;   // bit 47 (the leading fraction bit) and bit 0 of the upper 48
	f.lower = 0xC000'0000'0000'0001ull;   // the next two fraction bits, and the last one
	const bitblock<nbits> got = extract_long_double_fraction<nbits>(f);
	// fraction bit k (k = 0 the leading one) lands at nbits - 1 - k
	auto expect = [&](unsigned k, bool v) {
		if (k < nbits && got.test(nbits - 1 - k) != v) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: extract_long_double_fraction<" << nbits << "> fraction bit " << k << '\n';
		}
	};
	expect(0, true);
	expect(1, false);
	expect(47, true);
	expect(48, true);
	expect(49, true);
	expect(50, false);
	expect(111, true);
	return nrOfFailedTests;
}

} // namespace

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

	std::string test_suite  = "bitblock helpers";
	std::string test_tag    = "helpers";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyRound(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyRound(reportTestCases), "bitblock", "round<>");
	nrOfFailedTestCases += ReportTestResult(VerifyToHex(reportTestCases), "bitblock", "to_hex");
	nrOfFailedTestCases += ReportTestResult(VerifyCopySlice(reportTestCases), "bitblock", "copy_slice_into");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleFraction<40>(reportTestCases), "bitblock<40>", "extract_long_double_fraction");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleFraction<60>(reportTestCases), "bitblock<60>", "extract_long_double_fraction");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleFraction<112>(reportTestCases), "bitblock<112>", "extract_long_double_fraction");
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
