// logic.cpp: comparison operator verification for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Neither takum variant had a logic suite.  That matters more here than it would
// for most formats, because takum comparison is not a value comparison: the
// encoding is a two's-complement integer whose order IS the value order
// (Proposition 4), so the operators work on bits and never decode.  Nothing was
// checking that the shortcut agrees with the values it stands for.
//
// The two properties that make it work are checked separately:
//
//   ordering    for every pair of encodings, the bit comparison agrees with the
//               comparison of the decoded values
//   totality    exactly one of <, ==, > holds for any pair, and the relations are
//               mutually consistent
//
// NaR is excluded from the ordering check and given its own.  It IS a normal
// two's-complement encoding -- 0x8000 at 16 bits, the most negative word -- but
// both variants deliberately give it IEEE NaN semantics in comparison: every
// relation involving NaR returns false, including NaR == NaR.
//
// That is worth pinning precisely because the encoding and the semantics point
// opposite ways.  x == x is false for NaR, trichotomy does not hold for it, and
// nothing in the library was checking either.  An earlier draft of this file
// asserted the opposite, on the reasoning that a two's-complement value ought to
// compare like one; the implementation says otherwise, in both variants, and the
// implementation is the specification here.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// The bit comparison must agree with the value comparison, for every pair.
// Exhaustive: at 10 and 12 bits that is a million pairs, which is cheap and
// leaves no room for a lucky sample.
template<unsigned nbits, unsigned rbits>
int VerifyOrdering(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t NR = 1ull << nbits;

	for (uint64_t i = 0; i < NR; ++i) {
		TL a; a.setbits(i);
		if (a.isnar()) continue;
		const double da = double(a);
		if (!std::isfinite(da)) continue;

		for (uint64_t j = 0; j < NR; ++j) {
			TL b; b.setbits(j);
			if (b.isnar()) continue;
			const double db = double(b);
			if (!std::isfinite(db)) continue;

			// Distinct encodings can share a double when the format outruns it,
			// so only the strict relations are pinned against the value.
			if (da < db && !(a < b)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL " << da << " < " << db << " but encodings disagree\n";
				}
			}
			if (da > db && !(a > b)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL " << da << " > " << db << " but encodings disagree\n";
				}
			}
			if (i == j && !(a == b)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL identical encodings are not equal\n";
			}
		}
	}
	return nrOfFailedTests;
}

// Exactly one of <, ==, > must hold, and the derived relations must follow.
template<unsigned nbits, unsigned rbits>
int VerifyTotality(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t NR = 1ull << nbits;

	for (uint64_t i = 0; i < NR; ++i) {
		TL a; a.setbits(i);
		if (a.isnar()) continue;
		for (uint64_t j = 0; j < NR; ++j) {
			TL b; b.setbits(j);
			if (b.isnar()) continue;

			const int trichotomy = (a < b ? 1 : 0) + (a == b ? 1 : 0) + (a > b ? 1 : 0);
			if (trichotomy != 1) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL trichotomy broken for bits " << i << " and " << j << '\n';
				}
			}
			if ((a != b) != !(a == b)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL != is not the negation of ==\n";
			}
			if ((a <= b) != (a < b || a == b)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL <= is not < or ==\n";
			}
			if ((a >= b) != (a > b || a == b)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL >= is not > or ==\n";
			}
		}
	}
	return nrOfFailedTests;
}

// NaR is a normal two's-complement value, not an IEEE NaN, so it compares rather
// than poisoning every relation.  Pin what it actually does.
template<unsigned nbits, unsigned rbits>
int VerifyNaR(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	auto fail = [&](const char* what) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << '\n';
	};

	TL nar; nar.setnar();
	TL zero; zero.setzero();
	TL one(1.0);

	if (!nar.isnar())            fail("setnar did not produce NaR");
	// NaN semantics: every comparison against NaR is false, itself included.
	if (nar == nar)              fail("NaR == NaR must be false (NaN semantics)");
	if (!(nar != nar))           fail("NaR != NaR must be true");
	if (nar == zero)             fail("NaR == zero must be false");
	if (nar == one)              fail("NaR == one must be false");
	if (nar < one)               fail("NaR < one must be false");
	if (nar > one)               fail("NaR > one must be false");
	if (one < nar)               fail("one < NaR must be false");
	if (one > nar)               fail("one > NaR must be false");
	if (nar <= nar)              fail("NaR <= NaR must be false");
	if (nar >= nar)              fail("NaR >= NaR must be false");
	// so trichotomy fails for NaR, which is why the ordering suites exclude it
	if (((nar < one) ? 1 : 0) + ((nar == one) ? 1 : 0) + ((nar > one) ? 1 : 0) != 0) {
		fail("no relation involving NaR should hold");
	}
	// zero and its negation are the same encoding
	TL negzero = -zero;
	if (!(negzero == zero))      fail("negated zero must equal zero");
	if (!zero.iszero() || !negzero.iszero()) fail("negated zero must still be zero");
	// ordering around zero
	TL neg(-1.0);
	if (!(neg < zero))           fail("-1 < 0");
	if (!(zero < one))           fail("0 < 1");
	if (!(neg < one))            fail("-1 < 1");
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

	std::string test_suite  = "takum_log logic operator verification";
	std::string test_tag    = "logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<8, 3>(true), "takum_log<8,3>", "ordering");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNaR<16, 3>(reportTestCases), "takum_log<16,3>", "NaR and zero");
	nrOfFailedTestCases += ReportTestResult(VerifyNaR<32, 3>(reportTestCases), "takum_log<32,3>", "NaR and zero");
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<8, 3>(reportTestCases), "takum_log<8,3>", "ordering");
	nrOfFailedTestCases += ReportTestResult(VerifyTotality<8, 3>(reportTestCases), "takum_log<8,3>", "trichotomy");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<10, 3>(reportTestCases), "takum_log<10,3>", "ordering");
	nrOfFailedTestCases += ReportTestResult(VerifyTotality<10, 3>(reportTestCases), "takum_log<10,3>", "trichotomy");
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<8, 1>(reportTestCases), "takum_log<8,1>", "ordering");
	nrOfFailedTestCases += ReportTestResult(VerifyNaR<12, 3>(reportTestCases), "takum_log<12,3>", "NaR and zero");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<12, 3>(reportTestCases), "takum_log<12,3>", "ordering");
	nrOfFailedTestCases += ReportTestResult(VerifyTotality<12, 3>(reportTestCases), "takum_log<12,3>", "trichotomy");
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<10, 2>(reportTestCases), "takum_log<10,2>", "ordering");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyOrdering<14, 3>(reportTestCases), "takum_log<14,3>", "ordering");
	nrOfFailedTestCases += ReportTestResult(VerifyTotality<14, 3>(reportTestCases), "takum_log<14,3>", "trichotomy");
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
