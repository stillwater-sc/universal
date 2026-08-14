// multiplication.cpp: multiplication and division verification for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// These two operators do not go through a double.  |x*y| = sqrt(e)^(lx + ly) and
// |x/y| = sqrt(e)^(lx - ly), and l is an integer characteristic plus a p-bit
// fraction, so combining them is exact integer arithmetic with a single rounding
// at the end (issue #1300).
//
// Addition and subtraction are NOT here.  They have no logarithmic shortcut, so
// they still evaluate in a double and are not correctly rounded at 64 bits; that
// is the remaining half of #1300 and this suite would have to carve an exception
// out for them.
//
// The checks are chosen to be independent of how the operators are built:
//
//   identities         a*1, a/1, a/a, a*0, and NaR propagation
//   commutativity      a*b == b*a, bit for bit
//   correctly rounded  the produced encoding is the nearest one to a reference
//                      recomputed in dd_cascade from the decoded operands.  This
//                      is the check with teeth, and the only one that reaches the
//                      division borrow: dropping the borrow when the divisor's
//                      fraction exceeds the dividend's shifts the result by a
//                      whole characteristic unit.  Confirmed by mutation --
//                      removing the borrow trips this at every width, while the
//                      identity, commutativity, api, mathlib and conversion
//                      suites ALL still pass.
//
// An earlier draft checked (a/b)*b against a instead, bounded in encoding steps.
// That premise is unsound: one ulp of l is not a fixed number of encoding steps,
// because p varies across DR boundaries, so the check failed on correct code.
// Comparing against the answer rather than against the input is what works.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Distance between two encodings, in encoding steps.  Both are two's-complement
// integers ordered by value (Prop. 4), so this is a meaningful ulp count.
template<typename TL>
int64_t step_distance(const TL& a, const TL& b) {
	const int64_t x = static_cast<int64_t>(a.raw_bits());
	const int64_t y = static_cast<int64_t>(b.raw_bits());
	return (x > y) ? (x - y) : (y - x);
}

template<unsigned nbits, unsigned rbits>
int VerifyIdentities(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);   // no 64-bit shift
	TL one(1.0), zero; zero.setzero();
	TL nar; nar.setnar();

	auto fail = [&](const char* what, uint64_t bits) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << " at bits=" << bits << '\n';
	};

	for (uint64_t i = 0; i < NR; ++i) {
		TL a; a.setbits(i);
		if (a.isnar()) continue;

		if ((a * one).raw_bits() != a.raw_bits())  fail("a * 1 != a", i);
		if (!(a * zero).iszero())                  fail("a * 0 != 0", i);
		if (!(a / nar).isnar())                    fail("a / NaR must be NaR", i);
		if (!(nar * a).isnar())                    fail("NaR * a must be NaR", i);
		if (!(a / zero).isnar())                   fail("a / 0 must be NaR", i);
		if (!a.iszero()) {
			if ((a / one).raw_bits() != a.raw_bits())  fail("a / 1 != a", i);
			if ((a / a).raw_bits() != one.raw_bits())  fail("a / a != 1", i);
			if (!(zero / a).iszero())                  fail("0 / a != 0", i);
		}
	}
	return nrOfFailedTests;
}

template<unsigned nbits, unsigned rbits>
int VerifyCommutative(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);   // no 64-bit shift

	for (uint64_t i = 0; i < NR; ++i) {
		TL a; a.setbits(i);
		if (a.isnar()) continue;
		for (uint64_t j = 0; j < NR; ++j) {
			TL b; b.setbits(j);
			if (b.isnar()) continue;
			if ((a * b).raw_bits() != (b * a).raw_bits()) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL multiplication is not commutative at " << i << ',' << j << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// The produced encoding must be the nearest to l = la + lb (or la - lb),
// recomputed here in dd_cascade from the decoded operands.  ~106 bits against the
// 67 that l needs, so the reference is exact for this purpose, and it is arrived
// at independently of the operator's integer path.
template<unsigned nbits, unsigned rbits>
int VerifyCorrectlyRounded(bool divide, bool reportTestCases, uint64_t stride = 1) {
	using sw::universal::dd_cascade;
	using TL    = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	using Codec = typename TL::Codec;
	int nrOfFailedTests = 0;
	// 1ull << nbits is undefined at nbits == 64, and 64 is exactly the width this
	// change exists for, so the bound is expressed without it.  stride > 1 samples
	// rather than enumerating, since the sweep is over PAIRS.
	const uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);
	const uint64_t span = (1ull << (nbits - 1)) - 1;
	long exercised = 0;

	// l = c + M/2^p, exactly.  M reaches 2^59, past a double, so it is split.
	auto logvalue = [&](uint64_t mag) -> dd_cascade {
		auto e = Codec::decode(mag);
		dd_cascade l(static_cast<double>(e.c));
		if (e.p > 0 && e.M_bits != 0) {
			const double hi = static_cast<double>(e.M_bits >> 32);
			const double lo = static_cast<double>(e.M_bits & 0xFFFFFFFFull);
			const double sc = std::ldexp(1.0, -static_cast<int>(e.p));
			l = l + dd_cascade(hi * std::ldexp(1.0, 32) * sc) + dd_cascade(lo * sc);
		}
		return l;
	};

	for (uint64_t i = 0; i < NR && i + stride > i; i += stride) {
		TL a; a.setbits(i);
		if (a.isnar() || a.iszero()) continue;
		for (uint64_t j = 0; j < NR && j + stride > j; j += stride) {
			TL b; b.setbits(j);
			if (b.isnar() || b.iszero()) continue;

			TL got = divide ? (a / b) : (a * b);
			if (got.isnar() || got.iszero()) continue;
			const uint64_t gm = got.magnitude_bits();
			// a saturated result is the range boundary, not a rounding decision
			if (gm == span || gm == 1ull) continue;

			dd_cascade la = logvalue(a.magnitude_bits());
			dd_cascade lb = logvalue(b.magnitude_bits());
			dd_cascade want = divide ? (la - lb) : (la + lb);

			auto dist = [&](uint64_t m) -> dd_cascade {
				dd_cascade d = logvalue(m) - want;
				return (d < dd_cascade(0.0)) ? (dd_cascade(0.0) - d) : d;
			};
			const dd_cascade mine = dist(gm);
			++exercised;
			for (int k = -1; k <= 1; k += 2) {
				if (k < 0 && gm < 1ull + 1ull) continue;
				const uint64_t m = (k < 0) ? (gm - 1ull) : (gm + 1ull);
				if (m < 1ull || m > span) continue;
				if (dist(m) < mine) {
					++nrOfFailedTests;
					if (reportTestCases) {
						std::cout << "FAIL " << (divide ? "division" : "multiplication")
						          << " not correctly rounded at " << i << ',' << j << '\n';
					}
					break;
				}
			}
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL correctly-rounded check compared nothing\n";
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

	std::string test_suite  = "takum_log multiplication and division verification";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<8, 3>(true, true), "takum_log<8,3>", "correctly rounded divide");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyIdentities<12, 3>(reportTestCases), "takum_log<12,3>", "identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIdentities<16, 3>(reportTestCases), "takum_log<16,3>", "identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<8, 3>(false, reportTestCases), "takum_log<8,3>", "correctly rounded multiply");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<8, 3>(true, reportTestCases), "takum_log<8,3>", "correctly rounded divide");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<8, 3>(reportTestCases), "takum_log<8,3>", "commutativity");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<10, 3>(false, reportTestCases), "takum_log<10,3>", "correctly rounded multiply");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<10, 3>(true, reportTestCases), "takum_log<10,3>", "correctly rounded divide");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<10, 3>(reportTestCases), "takum_log<10,3>", "commutativity");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIdentities<10, 2>(reportTestCases), "takum_log<10,2>", "identities");
#endif

#if REGRESSION_LEVEL_3
	// The dd_cascade reference check stays at 8 and 10 bits: it is exhaustive over
	// pairs, so 12 bits is 16.7M of them each doing several extended-precision
	// operations.  The cheap structural checks carry the wider configurations.
	// commutativity is exhaustive over pairs, so 12 bits is 16.7M of them; that is
	// the practical ceiling for a pairwise sweep in this suite
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<12, 3>(reportTestCases), "takum_log<12,3>", "commutativity");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIdentities<20, 3>(reportTestCases), "takum_log<20,3>", "identities");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<8, 1>(false, reportTestCases), "takum_log<8,1>", "correctly rounded multiply");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<8, 1>(true, reportTestCases), "takum_log<8,1>", "correctly rounded divide");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIdentities<14, 3>(reportTestCases), "takum_log<14,3>", "identities");
	// The width this change exists for.  takum_log<64,3> reaches p = 58 and 59,
	// past a double's 53, and the double-mediated operators it replaced were
	// incorrectly rounded for 99% of products here.  Sampled with a stride
	// coprime to the field structure so the sweep crosses every DR (about 225
	// samples per operand, so ~50k pairs); enumerating
	// pairs at 64 bits is obviously out of the question.
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(false, reportTestCases, 0x0123456789ABCDEFull),
		"takum_log<64,3>", "correctly rounded multiply");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(true, reportTestCases, 0x0123456789ABCDEFull),
		"takum_log<64,3>", "correctly rounded divide");
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
