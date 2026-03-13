//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <string>

/// helper: print quire state with optional elision for long sequences
template<typename QuireType>
void printProgress(const QuireType& q, int i, int total, bool& elided) {
	using namespace sw::universal;
	if (i < 3 || i >= total - 3) {
		std::cout << "  [" << i << "] " << to_binary(q) << '\n';
	} else if (!elided) {
		std::cout << "  ...\n";
		elided = true;
	}
}

/// TestQuirePowerOfTwoSweep
///
/// Exercises every bit position in the quire by accumulating power-of-two
/// products spanning the full dynamic range, then subtracting them all back.
///
/// Algorithm:
///   1. For every power-of-two v representable in Scalar from minpos to maxpos:
///        q += quire_mul(v, v)          — sets bit at quire position 2*scale(v)
///   2. Subtract the same products in reverse order (top to bottom):
///        q -= quire_mul(v, v)
///   3. Verify q == 0.
///
/// This works because:
///   - Every power-of-two is exactly representable (in both normal and subnormal ranges)
///   - Doubling a power of 2 is exact (just shifts the exponent, no significand rounding)
///   - quire_mul and quire +=/-= are exact (no rounding)
///   - Each v^2 = 2^(2k) lands on a distinct quire bit (no carry interaction)
///
template<typename Scalar>
int TestQuirePowerOfTwoSweep() {
	using namespace sw::universal;

	using QuireType = sw::universal::quire<Scalar, 8u>;
	std::cout << "Power-of-two sweep test: " << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	Scalar two(2.0f);
	QuireType q;

	std::cout << quire_properties<Scalar>() << '\n';
	std::cout << "minpos : " << to_binary(minpos) << " : " << minpos << '\n';
	std::cout << "maxpos : " << to_binary(maxpos) << " : " << maxpos << '\n';

	// count how many power-of-two scales exist
	int nScales = 0;
	for (Scalar v = minpos; v <= maxpos; v *= two) ++nScales;
	std::cout << "Number of power-of-two scales: " << nScales << '\n';

	// Phase 1: accumulate v^2 at every power-of-two scale (bottom to top)
	std::cout << "\nPhase 1 — accumulate v^2 from minpos to largest 2^k <= maxpos:\n";
	{
		bool elided = false;
		int step = 0;
		for (Scalar v = minpos; v <= maxpos; v *= two, ++step) {
			q += quire_mul(v, v);
			printProgress(q, step, nScales, elided);
		}
	}
	std::cout << "After accumulation (" << nScales << " products):\n  " << to_binary(q) << '\n';

	// Phase 2: subtract v^2 at every power-of-two scale (top to bottom)
	// Walk top-down by first finding the largest power of 2 <= maxpos
	Scalar topPow2 = minpos;
	for (Scalar v = minpos; v <= maxpos; v *= two) topPow2 = v;

	std::cout << "\nPhase 2 — subtract v^2 from top down:\n";
	{
		Scalar half(0.5f);
		bool elided = false;
		int step = 0;
		for (Scalar v = topPow2; v >= minpos; v *= half, ++step) {
			q -= quire_mul(v, v);
			printProgress(q, step, nScales, elided);
		}
	}

	std::cout << "\nFinal state:\n  " << to_binary(q) << '\n';
	if (q.iszero()) {
		std::cout << "PASS: quire returned to zero\n\n";
	} else {
		std::cout << "FAIL: quire is not zero\n\n";
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

/// TestQuireMaxposCancellation
///
/// Add maxpos^2 and minpos^2 to the quire (bits at both extremes), then
/// subtract both back.  Verifies that the largest and smallest products
/// accumulate and cancel exactly.
///
template<typename Scalar>
int TestQuireMaxposCancellation() {
	using namespace sw::universal;

	using QuireType = sw::universal::quire<Scalar, 8u>;
	std::cout << "Maxpos/minpos cancellation test: " << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	QuireType q;

	// load both extremes
	q += quire_mul(maxpos, maxpos);
	std::cout << "  +maxpos^2 : " << to_binary(q) << '\n';
	q += quire_mul(minpos, minpos);
	std::cout << "  +minpos^2 : " << to_binary(q) << '\n';

	// subtract them back
	q -= quire_mul(maxpos, maxpos);
	std::cout << "  -maxpos^2 : " << to_binary(q) << '\n';
	q -= quire_mul(minpos, minpos);
	std::cout << "  -minpos^2 : " << to_binary(q) << '\n';

	if (q.iszero()) {
		std::cout << "PASS: quire returned to zero\n\n";
	} else {
		std::cout << "FAIL: quire is not zero\n\n";
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

/// TestQuireAccumulationRepeated
///
/// Accumulate minpos^2 N times, then subtract it N times.
/// Verifies exact accumulation of many identical small products.
///
template<typename Scalar>
int TestQuireAccumulationRepeated(unsigned N = 1024) {
	using namespace sw::universal;

	using QuireType = sw::universal::quire<Scalar, 8u>;
	std::cout << "Repeated accumulation test (N=" << N << "): "
	          << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar minpos(SpecificValue::minpos);
	QuireType q;
	auto mp2 = quire_mul(minpos, minpos);

	// accumulate N copies of minpos^2
	for (unsigned i = 0; i < N; ++i) q += mp2;
	std::cout << "  After +" << N << " * minpos^2:\n    " << to_binary(q) << '\n';

	// subtract them back
	for (unsigned i = 0; i < N; ++i) q -= mp2;

	if (q.iszero()) {
		std::cout << "  PASS: quire returned to zero\n\n";
	} else {
		std::cout << "  FAIL: quire is not zero\n\n";
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

/// TestQuireBitWalk
///
/// Walks a single bit from minpos^2 (bit 0) up to the second bit in the
/// capacity range by doubling the quire value at each step.  Then replays
/// the same product sequence with negated sign to drain back to zero.
///
/// Algorithm:
///   Phase 1 (walk UP):
///     Step 0: q += minpos^2                    → q = 1·minpos^2  (bit 0)
///     Step 1: q += minpos^2                    → q = 2·minpos^2  (bit 1)
///     Step k (k≥2): q += 2^(k-1)·minpos^2     → q = 2^k·minpos^2  (bit k)
///     ...until bit reaches position range+1 (second capacity bit).
///
///   Phase 2 (walk DOWN):
///     Subtract the same products in the same order.
///     Step 0 triggers a borrow cascade that fills all bits below the top.
///     Each subsequent step clears the lowest set bit, walking the zeros
///     upward until the quire is empty.
///
/// Each product 2^k·minpos^2 is expressed as quire_mul(a, b) where a and b
/// are power-of-two multiples of minpos.  When k exceeds what a single
/// product can represent, multiple copies of the maximum product are used.
///
template<typename Scalar>
int TestQuireBitWalk() {
	using namespace sw::universal;

	constexpr unsigned cap = 8u;
	using QuireType = quire<Scalar, cap>;
	using Traits    = quire_traits<Scalar>;

	std::cout << "Bit walk test: " << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	Scalar two(2.0f);
	QuireType q;

	// Largest representable power of 2
	Scalar topPow2 = minpos;
	while (topPow2 * two <= maxpos) topPow2 *= two;

	// max_half = log2(topPow2 / minpos)
	unsigned max_half = 0;
	for (Scalar v = minpos; v < topPow2; v *= two) ++max_half;

	// Maximum bit position reachable by a single quire_mul of two representable Scalars
	unsigned max_single = 2 * max_half;

	// Target bit: second bit in the capacity range = range + 1
	constexpr unsigned target = Traits::range + 1;
	unsigned totalSteps = target + 1;  // steps 0 through target

	// Helper: scale minpos up by 2^exp (exact for power-of-two factors)
	auto scaleUp = [&](unsigned exp) {
		Scalar v = minpos;
		for (unsigned i = 0; i < exp; ++i) v *= two;
		return v;
	};

	// Maximum product: quire_mul(topPow2, topPow2) = 2^(2·max_half) · minpos^2
	auto maxProduct = quire_mul(topPow2, topPow2);

	std::cout << "  range=" << Traits::range << "  capacity=" << cap
	          << "  target bit=" << target
	          << "  max single product bit=" << max_single << '\n';

	// ---- Phase 1: walk bit UP ----
	std::cout << "\n  Phase 1 — walk bit UP (add):\n";
	{
		bool elided = false;
		for (unsigned k = 0; k <= target; ++k) {
			// At entry: q = 2^k · minpos^2 (except k=0 where q=0)
			// Add 2^prodBit · minpos^2 to double the quire value
			unsigned prodBit = (k == 0) ? 0 : k - 1;
			if (prodBit <= max_single) {
				unsigned a_exp = (prodBit + 1) / 2;  // ceil(prodBit / 2)
				unsigned b_exp = prodBit / 2;         // floor(prodBit / 2)
				q += quire_mul(scaleUp(a_exp), scaleUp(b_exp));
			} else {
				// Need 2^(prodBit - max_single) copies of the max product
				unsigned copies = 1u << (prodBit - max_single);
				for (unsigned c = 0; c < copies; ++c) q += maxProduct;
			}
			// After: q = 2^(k+1) · minpos^2 ... except after final step where
			// k = target, so q = 2^target · minpos^2 but we've added one extra
			// Actually: after step k, q = (1 + 1 + 2 + 4 + ... + 2^(k-1)) · minpos^2
			//         = 2^k · minpos^2  (geometric sum)
			// But step 0 and 1 both add minpos^2, so:
			//   step 0: q = minpos^2
			//   step 1: q = 2·minpos^2
			//   step k (k≥1): q = 2^k · minpos^2
			printProgress(q, static_cast<int>(k), static_cast<int>(totalSteps), elided);
		}
	}
	std::cout << "  After phase 1 (single bit at position " << target << "):\n    "
	          << to_binary(q) << '\n';

	// ---- Phase 2: walk bit DOWN (subtract same sequence) ----
	std::cout << "\n  Phase 2 — subtract same sequence (walk DOWN):\n";
	{
		bool elided = false;
		for (unsigned k = 0; k <= target; ++k) {
			unsigned prodBit = (k == 0) ? 0 : k - 1;
			if (prodBit <= max_single) {
				unsigned a_exp = (prodBit + 1) / 2;
				unsigned b_exp = prodBit / 2;
				q -= quire_mul(scaleUp(a_exp), scaleUp(b_exp));
			} else {
				unsigned copies = 1u << (prodBit - max_single);
				for (unsigned c = 0; c < copies; ++c) q -= maxProduct;
			}
			printProgress(q, static_cast<int>(k), static_cast<int>(totalSteps), elided);
		}
	}

	std::cout << "\n  Final state:\n    " << to_binary(q) << '\n';
	if (q.iszero()) {
		std::cout << "  PASS: quire returned to zero\n\n";
	} else {
		std::cout << "  FAIL: quire is not zero\n\n";
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "cfloat<> quire accumulation";
	std::string test_tag            = "cfloat<> quire";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#	if MANUAL_TESTING

	// Power-of-two sweep: exercises every quire bit position across full dynamic range
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<cfloat<8, 3, uint8_t, true, false, false>>();
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<cfloat<8, 3, uint8_t, true, true, false>>();
	nrOfFailedTestCases += TestQuirePowerOfTwoSweep<cfloat<8, 2, uint8_t, true, false, false>>();

	// Maxpos/minpos direct cancellation: extreme products at both ends of the quire
	nrOfFailedTestCases += TestQuireMaxposCancellation<cfloat<8, 3, uint8_t, true, false, false>>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<cfloat<8, 3, uint8_t, true, true, false>>();
	nrOfFailedTestCases += TestQuireMaxposCancellation<cfloat<8, 2, uint8_t, true, false, false>>();

	// Repeated minpos^2 round-trip: exact summation of many identical small products
	nrOfFailedTestCases += TestQuireAccumulationRepeated<cfloat<8, 3, uint8_t, true, false, false>>();
	nrOfFailedTestCases += TestQuireAccumulationRepeated<cfloat<8, 2, uint8_t, true, false, false>>();

	// Bit walk: single bit from minpos^2 to capacity, then negate back to zero
	nrOfFailedTestCases += TestQuireBitWalk<cfloat<8, 3, uint8_t, true, false, false>>();
	nrOfFailedTestCases += TestQuireBitWalk<cfloat<8, 3, uint8_t, true, true, false>>();
	nrOfFailedTestCases += TestQuireBitWalk<cfloat<8, 2, uint8_t, true, false, false>>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1

#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
