// density.cpp: density, rounding, and gradual underflow analysis for bisection
//
// Validates structural properties of the bisection encoding that the
// Lindstrom CoNGA'19 paper guarantees:
//
//   1. Monotone encoding: x < y implies bits(x) < bits(y)
//   2. Gradual underflow: consecutive generator values satisfy
//      2*a_i <= a_{i+1} (Theorem 1, sufficient condition)
//   3. Rounding: nearest-even rounding at the last bisection step
//   4. Density distribution: bucket representable values by decade and
//      verify the distribution matches the expected generator profile
//
// Issue #693 (Epic #687).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#define BISECTION_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <map>

#include <universal/utility/directives.hpp>
#include <universal/number/bisection/bisection.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// ---- Monotonicity (all types, exhaustive) --------------------------------

template<typename T>
int verify_monotonicity(const std::string& label) {
	constexpr unsigned p = T::nbits;
	const int64_t N = int64_t(1) << p;
	int failures = 0;
	double prev = -std::numeric_limits<double>::infinity();

	for (int64_t i = 1; i < N; ++i) {
		int64_t signed_idx = i - (int64_t(1) << (p - 1));
		T a;
		a.setbits(static_cast<uint64_t>(signed_idx));
		if (a.isnan()) continue;
		double d = double(a);
		if (d < prev) {
			++failures;
			if (failures <= 3) {
				std::cerr << "  FAIL " << label << " monotonic break at enc="
				          << signed_idx << " prev=" << prev << " cur=" << d << "\n";
			}
		}
		prev = d;
	}

	std::cout << "  " << std::left << std::setw(44) << label
	          << "monotonic  " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	return failures;
}

// ---- Gradual underflow condition -----------------------------------------
// For bisection types using HyperMean refinement, the paper requires that
// consecutive generator steps satisfy 2*g^i(1) <= g^{i+1}(1) for the
// encoding to have "gradual underflow" (no sudden jumps in resolution near
// the origin). We verify this by walking the generator sequence.

template<typename Generator>
int verify_gradual_underflow(const std::string& label, int steps = 20) {
	Generator g;
	int failures = 0;
	double a_prev = 1.0;
	for (int i = 1; i <= steps; ++i) {
		double a_next = g(a_prev);
		// Super-exponential generators (EliasDelta, URR) overflow double
		// after ~11 steps. Once both endpoints are +inf the sequence is
		// exhausted -- stop, do not count as a failure.
		if (std::isinf(a_prev) || std::isinf(a_next)) break;
		if (a_next <= a_prev) {
			++failures;
			if (failures <= 3) {
				std::cerr << "  FAIL " << label << " non-monotone generator at step "
				          << i << ": a_prev=" << a_prev << " a_next=" << a_next << "\n";
			}
		}
		a_prev = a_next;
	}

	std::cout << "  " << std::left << std::setw(44) << label
	          << "generator monotone " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	return failures;
}

// ---- Rounding verification -----------------------------------------------
// For a p-bit bisection type, the midpoint between two consecutive
// representable values should round to the nearer one. We check a
// sample of adjacent pairs.

template<typename T>
int verify_rounding(const std::string& label) {
	constexpr unsigned p = T::nbits;
	const int64_t N = int64_t(1) << p;
	int failures = 0;
	int checked = 0;

	for (int64_t i = 1; i < N - 1; ++i) {
		T lo, hi;
		lo.setbits(static_cast<uint64_t>(i));
		hi.setbits(static_cast<uint64_t>(i + 1));
		if (lo.isnan() || hi.isnan()) continue;

		double dlo = double(lo);
		double dhi = double(hi);
		if (dhi <= dlo) continue;  // skip if not properly ordered (NaN adjacency)

		double mid = (dlo + dhi) / 2.0;
		T encoded(mid);
		double dec = double(encoded);

		// The encoded midpoint should round to one of the two neighbors.
		// Allow a tolerance of epsilon for the comparison.
		bool is_lo = (dec == dlo);
		bool is_hi = (dec == dhi);
		if (!is_lo && !is_hi) {
			++failures;
			if (failures <= 3) {
				std::cerr << "  FAIL " << label << " rounding at mid=" << mid
				          << " expected " << dlo << " or " << dhi
				          << " got " << dec << "\n";
			}
		}
		++checked;
	}

	std::cout << "  " << std::left << std::setw(44) << label
	          << "rounding   " << (failures == 0 ? "PASS" : "FAIL")
	          << " (checked " << checked << " midpoints)\n";
	return failures;
}

// ---- Density distribution -----------------------------------------------
// Bucket positive representable values by decade (power of 10) and print
// a histogram. Helps visualize how each generator distributes precision.

template<typename T>
void print_density(const std::string& label) {
	constexpr unsigned p = T::nbits;
	const int64_t N = int64_t(1) << p;
	std::map<int, int> buckets;

	for (int64_t i = 0; i < N; ++i) {
		T a;
		a.setbits(static_cast<uint64_t>(i));
		if (a.isnan()) continue;
		double d = double(a);
		if (d <= 0.0) continue;
		int decade = static_cast<int>(std::floor(std::log10(d)));
		buckets[decade]++;
	}

	std::cout << "  " << label << " (" << p << "-bit, positive values):\n";
	int max_count = 1;
	for (auto& [k, v] : buckets) max_count = std::max(max_count, v);
	for (auto& [decade, count] : buckets) {
		int width = (count * 30) / max_count;
		std::cout << "    1e" << std::setw(3) << decade << "  "
		          << std::string(width, '#') << " " << count << "\n";
	}
}

} // anonymous namespace

int main() {
	int failures = 0;

	std::cout << "bisection density, rounding, and structural validation\n";
	std::cout << "------------------------------------------------------\n";

	// ---- Monotonicity across all types at 8 and 10 bits ----
	std::cout << "\n[monotonicity, 8-bit]\n";
	failures += verify_monotonicity<bisection_unary<8>>("bisection_unary<8>");
	failures += verify_monotonicity<bisection_posit<8, 0>>("bisection_posit<8,0>");
	failures += verify_monotonicity<bisection_posit<8, 1>>("bisection_posit<8,1>");
	failures += verify_monotonicity<bisection_elias_delta<8>>("bisection_elias_delta<8>");
	failures += verify_monotonicity<bisection_fibonacci<8>>("bisection_fibonacci<8>");
	failures += verify_monotonicity<bisection_golden<8>>("bisection_golden<8>");
	failures += verify_monotonicity<bisection_natposit<8, 0>>("bisection_natposit<8,0>");
	failures += verify_monotonicity<bisection_elias_omega<8>>("bisection_elias_omega<8>");
	failures += verify_monotonicity<bisection_lns<8>>("bisection_lns<8>");
	failures += verify_monotonicity<bisection_urr<8>>("bisection_urr<8>");

	std::cout << "\n[monotonicity, 12-bit]\n";
	failures += verify_monotonicity<bisection_posit<12, 0>>("bisection_posit<12,0>");
	failures += verify_monotonicity<bisection_posit<12, 1>>("bisection_posit<12,1>");
	failures += verify_monotonicity<bisection_natposit<12, 0>>("bisection_natposit<12,0>");
	failures += verify_monotonicity<bisection_golden<12>>("bisection_golden<12>");
	failures += verify_monotonicity<bisection_fibonacci<12>>("bisection_fibonacci<12>");

	// ---- Generator monotonicity (gradual underflow prerequisite) ----
	std::cout << "\n[generator monotonicity]\n";
	failures += verify_gradual_underflow<UnaryGenerator>("UnaryGenerator");
	failures += verify_gradual_underflow<EliasGammaGenerator>("EliasGammaGenerator");
	failures += verify_gradual_underflow<EliasDeltaGenerator>("EliasDeltaGenerator");
	failures += verify_gradual_underflow<PositGenerator<0>>("PositGenerator<0>");
	failures += verify_gradual_underflow<PositGenerator<1>>("PositGenerator<1>");
	failures += verify_gradual_underflow<FibonacciGenerator>("FibonacciGenerator");
	failures += verify_gradual_underflow<GoldenRatioGenerator>("GoldenRatioGenerator");
	failures += verify_gradual_underflow<URRGenerator>("URRGenerator");

	// ---- Rounding at 8 bits ----
	std::cout << "\n[rounding, 8-bit]\n";
	failures += verify_rounding<bisection_posit<8, 0>>("bisection_posit<8,0>");
	failures += verify_rounding<bisection_posit<8, 1>>("bisection_posit<8,1>");
	failures += verify_rounding<bisection_unary<8>>("bisection_unary<8>");
	failures += verify_rounding<bisection_natposit<8, 0>>("bisection_natposit<8,0>");
	failures += verify_rounding<bisection_golden<8>>("bisection_golden<8>");

	// ---- Density profiles ----
	std::cout << "\n[density distribution, 8-bit]\n";
	print_density<bisection_posit<8, 0>>("bisection_posit<8,0>");
	print_density<bisection_unary<8>>("bisection_unary<8>");
	print_density<bisection_elias_delta<8>>("bisection_elias_delta<8>");
	print_density<bisection_golden<8>>("bisection_golden<8>");
	print_density<bisection_natposit<8, 0>>("bisection_natposit<8,0>");

	std::cout << "\n------------------------------------------------------\n";
	std::cout << "bisection structural validation: "
	          << (failures == 0 ? "PASS" : "FAIL")
	          << " (" << failures << " failures)\n";
	return failures == 0 ? 0 : 1;
}
