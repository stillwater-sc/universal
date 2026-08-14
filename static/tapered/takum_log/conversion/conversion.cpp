// conversion.cpp: value conversion verification for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum_log converts by way of its logarithmic value: encoding a real x means
// finding the representable l nearest 2 ln|x|, and decoding means evaluating
// sqrt(e)^l.  The properties worth pinning follow from that, and two of them are
// specific enough to this format to be easy to get wrong:
//
//   decode-encode identity   every encoding must survive a round trip out to a
//                            double and back, wherever a double can hold it
//   monotonicity             encoding is order preserving, so a larger real must
//                            never produce a smaller encoding
//   saturation direction     a value past maxpos saturates UP and one below
//                            minpos goes to zero -- and those two must not swap,
//                            which is precisely the failure that made exp()
//                            return zero for exp(maxpos) in #1305
//   integers are not exact   an integer k is sqrt(e)^l only for irrational l, so
//                            takum_log<32,3>(3) holds 2.99999999 and NO
//                            configuration represents 3 exactly.  Tests that
//                            assume otherwise fail for the wrong reason; this one
//                            asserts the property instead.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Every encoding must survive encoding -> double -> encoding.
template<unsigned nbits, unsigned rbits>
int VerifyRoundTrip(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t NR = 1ull << nbits;
	long exercised = 0;

	for (uint64_t i = 0; i < NR; ++i) {
		TL x; x.setbits(i);
		if (x.isnar()) continue;
		const double d = double(x);
		// wide regime fields put minpos and maxpos outside a double's range; those
		// encodings are valid, they just cannot be compared through one
		if (!std::isfinite(d)) continue;
		if (d == 0.0 && !x.iszero()) continue;

		TL back(d);
		++exercised;
		if (back.raw_bits() != x.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL round trip bits=" << i << " value=" << d
				          << " came back as " << back.raw_bits() << '\n';
			}
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL round trip exercised nothing\n";
	}
	return nrOfFailedTests;
}

// Encoding preserves order: a larger real never yields a smaller encoding.
template<unsigned nbits, unsigned rbits>
int VerifyMonotonic(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;

	// sweep the representable span in the logarithmic domain, where the format is
	// uniform, so the samples are spread evenly across the encodings
	TL lo(sw::universal::SpecificValue::minpos);
	TL hi(sw::universal::SpecificValue::maxpos);
	const double dlo = double(lo), dhi = double(hi);
	if (!std::isfinite(dlo) || !std::isfinite(dhi) || dlo <= 0.0) return 0;   // beyond double

	const int N = 4000;
	const double llo = std::log(dlo), lhi = std::log(dhi);
	TL prev; bool have_prev = false;
	for (int i = 0; i <= N; ++i) {
		const double v = std::exp(llo + (lhi - llo) * (double(i) / double(N)));
		if (!std::isfinite(v) || v == 0.0) continue;
		TL x(v);
		if (x.isnar()) continue;
		if (have_prev && x < prev) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL monotonicity broken at " << v << '\n';
			}
		}
		prev = x; have_prev = true;
	}
	// and the same on the negative side, where the ordering reverses in value
	// but must still be consistent
	TL nprev; bool have_nprev = false;
	for (int i = N; i >= 0; --i) {
		const double v = -std::exp(llo + (lhi - llo) * (double(i) / double(N)));
		if (!std::isfinite(v)) continue;
		TL x(v);
		if (x.isnar()) continue;
		if (have_nprev && x < nprev) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL negative monotonicity broken at " << v << '\n';
		}
		nprev = x; have_nprev = true;
	}
	return nrOfFailedTests;
}

// Saturation must go the right way at both ends, and the two must not swap.
template<unsigned nbits, unsigned rbits>
int VerifySaturation(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	auto fail = [&](const char* what) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << '\n';
	};

	TL maxpos(sw::universal::SpecificValue::maxpos);
	TL minpos(sw::universal::SpecificValue::minpos);
	const double dmax = double(maxpos), dmin = double(minpos);
	if (!std::isfinite(dmax) || dmin <= 0.0) return 0;   // beyond double, cannot drive from one

	// past the top saturates UP, not to zero
	TL over(dmax * 16.0);
	if (over.iszero())  fail("a value above maxpos must not become zero");
	if (over.sign())    fail("a value above maxpos must stay positive");
	TL under_neg(-dmax * 16.0);
	if (under_neg.iszero()) fail("a value below maxneg must not become zero");
	if (!under_neg.sign())  fail("a value below maxneg must stay negative");

	// below the bottom goes to zero, not to maxpos
	TL tiny(dmin / 16.0);
	if (!tiny.iszero())     fail("a value below minpos must underflow to zero");
	TL tiny_neg(-dmin / 16.0);
	if (!tiny_neg.iszero()) fail("a negative value below minpos must underflow to zero");

	// specials
	TL zero(0.0);
	if (!zero.iszero())                                    fail("0.0 must encode as zero");
	TL nan(std::numeric_limits<double>::quiet_NaN());
	if (!nan.isnar())                                      fail("NaN must encode as NaR");
	TL inf(std::numeric_limits<double>::infinity());
	if (!inf.isnar())                                      fail("infinity must encode as NaR");
	TL ninf(-std::numeric_limits<double>::infinity());
	if (!ninf.isnar())                                     fail("-infinity must encode as NaR");
	return nrOfFailedTests;
}

// No integer is exactly representable, because k = sqrt(e)^l needs an irrational
// l for every integer k except none at all.  This is the property that makes
// naive expectations like "floor(TL(3)) == 3" fail, so assert it rather than
// letting a future test trip over it.
template<unsigned nbits, unsigned rbits>
int VerifyIntegersInexact(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;

	// 1 is the exception: l == 0, exactly representable.
	TL one(1.0);
	if (double(one) != 1.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL 1.0 must be exact (l == 0)\n";
	}
	// The inexactness itself can only be OBSERVED through a double while the format
	// is coarser than one.  At nbits = 64 the trailing field reaches 58 bits, finer
	// than a double's 53, so double(TL(2)) rounds to exactly 2.0 even though the
	// stored M is 111341769010871232 rather than 0.  Checking it there would assert
	// a property of the yardstick, not of the format.
	constexpr bool double_can_resolve = (nbits <= 32);
	for (double k : { 2.0, 3.0, 5.0, 7.0, 10.0, 100.0 }) {
		TL x(k);
		if (x.iszero() || x.isnar()) continue;
		if (double_can_resolve && double(x) == k) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << k << " came out exact; no integer but 1 should be\n";
			}
		}
		// What IS checkable at every width: the encoding has a non-zero fraction.
		// k = sqrt(e)^l needs l = 2 ln k, which is transcendental for every integer
		// k except 1, so the trailing field can never be empty.
		{
			auto dk = TL::Codec::decode(x.magnitude_bits());
			if (dk.p > 0 && dk.M_bits == 0) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL " << k << " encoded with an empty fraction,"
					          << " implying an exact power of sqrt(e)\n";
				}
			}
		}
		// but it must still be close
		const double rel = std::fabs((double(x) - k) / k);
		if (!(rel < 1.0e-2)) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << k << " encoded too far off: relerr " << rel << '\n';
			}
		}
	}
	// and the powers of the value base ARE exact: e, 1/e, sqrt(e)
	TL e_val(2.718281828459045235360287471352662497757);
	auto d = TL::Codec::decode(e_val.magnitude_bits());
	if (d.c != 2 || d.M_bits != 0) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL e should encode exactly as l == 2, got c=" << d.c
			          << " M=" << d.M_bits << '\n';
		}
	}
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "takum_log conversion verification";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<12, 3>(true), "takum_log<12,3>", "round trip");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifySaturation<16, 3>(reportTestCases), "takum_log<16,3>", "saturation direction");
	nrOfFailedTestCases += ReportTestResult(
		VerifySaturation<32, 3>(reportTestCases), "takum_log<32,3>", "saturation direction");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegersInexact<32, 3>(reportTestCases), "takum_log<32,3>", "integers are inexact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<12, 3>(reportTestCases), "takum_log<12,3>", "round trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<16, 3>(reportTestCases), "takum_log<16,3>", "round trip");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonic<16, 3>(reportTestCases), "takum_log<16,3>", "encoding is monotonic");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonic<32, 3>(reportTestCases), "takum_log<32,3>", "encoding is monotonic");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegersInexact<16, 3>(reportTestCases), "takum_log<16,3>", "integers are inexact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<16, 1>(reportTestCases), "takum_log<16,1>", "round trip");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<18, 3>(reportTestCases), "takum_log<18,3>", "round trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifySaturation<12, 3>(reportTestCases), "takum_log<12,3>", "saturation direction");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonic<12, 3>(reportTestCases), "takum_log<12,3>", "encoding is monotonic");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<20, 3>(reportTestCases), "takum_log<20,3>", "round trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegersInexact<64, 3>(reportTestCases), "takum_log<64,3>", "integers are inexact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonic<64, 3>(reportTestCases), "takum_log<64,3>", "encoding is monotonic");
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
