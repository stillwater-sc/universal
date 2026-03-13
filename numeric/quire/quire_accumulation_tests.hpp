#pragma once
// quire_accumulation_tests.hpp: shared test functions for quire accumulation
//
// These test functions are templated on <typename Scalar> and use quire_mul()
// generically. Each number type must provide:
//   - quire_mul(lhs, rhs)  → blocktriple product for quire accumulation
//   - quire_traits<Scalar>  → compile-time quire sizing
//   - SpecificValue::minpos, maxpos
//   - double conversion for collectPow2Scales enumeration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/quire/quire.hpp>
#include <iostream>
#include <vector>
#include <cmath>
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

/// Collect all representable powers of 2 for Scalar, sorted ascending.
///
/// Not all number systems can represent every integer power of 2:
///   - posit: tapered precision creates gaps near minpos/maxpos
///   - fixpnt: limited range means only a few powers fit
///   - lns: all powers within range are representable
///   - cfloat: all powers within normal+subnormal range
///
/// This helper uses std::ldexp + roundtrip check, which is safe for all types.
template<typename Scalar>
std::vector<Scalar> collectPow2Scales() {
	Scalar minpos(sw::universal::SpecificValue::minpos);
	Scalar maxpos(sw::universal::SpecificValue::maxpos);
	int min_exp = static_cast<int>(std::floor(std::log2(double(minpos))));
	int max_exp = static_cast<int>(std::floor(std::log2(double(maxpos))));
	std::vector<Scalar> scales;
	for (int e = min_exp; e <= max_exp; ++e) {
		double exact = std::ldexp(1.0, e);
		Scalar v;
		v = exact;
		if (double(v) == exact) {
			scales.push_back(v);
		}
	}
	return scales;
}

/// TestQuirePowerOfTwoSweep
///
/// Exercises every bit position in the quire by accumulating power-of-two
/// products spanning the full dynamic range, then subtracting them all back.
///
/// Uses collectPow2Scales() to enumerate only exactly-representable powers
/// of 2, which handles posit's tapered gaps and fixpnt's limited range.
///
template<typename Scalar>
int TestQuirePowerOfTwoSweep() {
	using namespace sw::universal;

	using QuireType = sw::universal::quire<Scalar, 8u>;
	std::cout << "Power-of-two sweep test: " << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	QuireType q;

	std::cout << quire_properties<Scalar>() << '\n';
	std::cout << "minpos : " << to_binary(minpos) << " : " << minpos << '\n';
	std::cout << "maxpos : " << to_binary(maxpos) << " : " << maxpos << '\n';

	auto scales = collectPow2Scales<Scalar>();
	int nScales = static_cast<int>(scales.size());
	std::cout << "Number of representable power-of-two scales: " << nScales << '\n';

	// Phase 1: accumulate v^2 at every power-of-two scale (bottom to top)
	std::cout << "\nPhase 1 — accumulate v^2 from minpos to maxpos:\n";
	{
		bool elided = false;
		for (int step = 0; step < nScales; ++step) {
			q += quire_mul(scales[step], scales[step]);
			printProgress(q, step, nScales, elided);
		}
	}
	std::cout << "After accumulation (" << nScales << " products):\n  " << to_binary(q) << '\n';

	// Phase 2: subtract v^2 at every power-of-two scale (top to bottom)
	std::cout << "\nPhase 2 — subtract v^2 from top down:\n";
	{
		bool elided = false;
		for (int step = 0; step < nScales; ++step) {
			int idx = nScales - 1 - step;
			q -= quire_mul(scales[idx], scales[idx]);
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
/// Products are constructed via std::ldexp to avoid saturation issues when
/// multiplying near maxpos (posit saturates, fixpnt wraps/saturates).
///
template<typename Scalar>
int TestQuireBitWalk() {
	using namespace sw::universal;

	constexpr unsigned cap = 8u;
	using QuireType = quire<Scalar, cap>;
	using Traits    = quire_traits<Scalar>;

	std::cout << "Bit walk test: " << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar minpos(SpecificValue::minpos);
	QuireType q;

	// Compute topPow2 and max_half from the representable power-of-two set
	auto scales = collectPow2Scales<Scalar>();
	Scalar topPow2 = scales.back();

	// max_half = exponent distance from minpos to topPow2
	int min_exp = static_cast<int>(std::round(std::log2(double(minpos))));
	int top_exp = static_cast<int>(std::round(std::log2(double(topPow2))));
	unsigned max_half = static_cast<unsigned>(top_exp - min_exp);

	// Maximum bit position reachable by a single quire_mul of two representable Scalars
	unsigned max_single = 2 * max_half;

	// Target bit: second bit in the capacity range = range + 1
	constexpr unsigned target = Traits::range + 1;
	unsigned totalSteps = target + 1;  // steps 0 through target

	// Helper: scale minpos up by 2^exp using exact ldexp (avoids rounding cascade)
	auto scaleUp = [&](unsigned exp) {
		double d = std::ldexp(double(minpos), static_cast<int>(exp));
		Scalar v;
		v = d;
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
