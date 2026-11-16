// fma.cpp: functional tests for the bfloat16 fused multiply-add fma(a,b,c)
//
// bfloat16 had no fma (#1193, sub-issue of the universal fma epic #1189). fma forms
// the exact a*b + c as a TwoSum pair in double and rounds it once (#2106). Its first
// form rounded through float, and that double rounding is NOT innocuous for fma:
// rounding the exact result to float can land on a bfloat16 tie.
//
// This suite validates against an INDEPENDENT oracle: the exact sum s + e brackets
// between two adjacent bfloat16 values and is compared with their midpoint exactly,
// ties to even. The previous oracle, bfloat16(std::fma(double, double, double)),
// rounded to 53 bits first and shared the double-rounding defect: it agreed with
// the float path on exactly the cases both got wrong.
// We also check: single-rounding superiority over the naive two-rounding
// bfloat16 a*b + c, IEEE special values (inf*0 -> NaN, inf/NaN propagation), and
// tie-to-even behavior.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfenv>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	bfloat16 bf16_from_bits(uint16_t bits) {
		bfloat16 b;
		b.setbits(bits);
		return b;
	}

	// NaN-aware bit-faithful comparison: bfloat16 values are exact in float, so
	// float() equality is exact for finite/inf; NaN compares equal to NaN.
	bool match(bfloat16 r, bfloat16 e) {
		if (isnan(r) || isnan(e)) return isnan(r) && isnan(e);
		return float(r) == float(e);
	}

	// the sign of (s + e) - v, exact, for |e| <= ulp(s) / 2 and v a double
	int compare_pair(double s, double e, double v) {
		if (s != v) return (s < v) ? -1 : 1;
		return (e < 0.0) ? -1 : ((e > 0.0) ? 1 : 0);
	}

	// the next bfloat16 encoding up the value line (maxpos steps to +inf), by raw bits
	bfloat16 next_up(bfloat16 x) {
		const unsigned b = x.bits();
		bfloat16 r;
		if (b == 0x8000u || b == 0x0000u) r.setbits(0x0001u);
		else if (b & 0x8000u) r.setbits(b - 1u);
		else r.setbits(b + 1u);
		return r;
	}
	bfloat16 next_down(bfloat16 x) {
		const unsigned b = x.bits();
		bfloat16 r;
		if (b == 0x8000u || b == 0x0000u) r.setbits(0x8001u);
		else if (b & 0x8000u) r.setbits(b + 1u);
		else r.setbits(b - 1u);
		return r;
	}
	// a finite value, with +/-inf standing for the overflow threshold of the side it is on:
	// maxpos + half its ulp: 255 * 2^120 + 2^119 = 511 * 2^119
	double as_bound(bfloat16 x) {
		if (isinf(x)) return (x.bits() & 0x8000u) ? -std::ldexp(511.0, 119) : std::ldexp(511.0, 119);
		return double(x);
	}

	// independent oracle: the exact a*b + c rounded once to bfloat16 by a midpoint comparison
	bfloat16 fma_oracle(bfloat16 a, bfloat16 b, bfloat16 c) {
		const double p = double(a) * double(b);   // exact
		const double cd = double(c);
		const double s = p + cd;
		if (!std::isfinite(p) || !std::isfinite(cd) || !std::isfinite(s)) return bfloat16(std::fma(double(a), double(b), double(c)));
		const double bv = s - p;
		const double e = (p - (s - bv)) + (cd - bv);          // s + e == a*b + c
		if (s == 0.0) return bfloat16(s);
		// r is within half a bfloat16 ulp of s, and s + e within a double ulp of s: the exact
		// value lies between r and its neighbour on the side compare_pair points to
		const bfloat16 r(s);
		if (isinf(r)) {
			// past the overflow threshold, or just below it with s rounded up: decide on the threshold
			const double t = as_bound(r);
			// the threshold itself rounds to the infinity, on either side
			const int c_t = compare_pair(s, e, t);
			if (t > 0) return (c_t >= 0) ? r : bfloat16(SpecificValue::maxpos);
			return (c_t <= 0) ? r : bfloat16(SpecificValue::maxneg);
		}
		const int c_r = compare_pair(s, e, double(r));
		if (c_r == 0) return r;
		const bfloat16 x = (c_r > 0) ? r : next_down(r);
		const bfloat16 y = (c_r > 0) ? next_up(r) : r;
		if (isinf(y)) return (compare_pair(s, e, as_bound(y)) >= 0) ? y : x;
		if (isinf(x)) return (compare_pair(s, e, as_bound(x)) <= 0) ? x : y;
		const double mid = (double(x) + double(y)) / 2.0;      // 9 significant bits: exact
		const int c_mid = compare_pair(s, e, mid);
		if (c_mid < 0) return x;
		if (c_mid > 0) return y;
		return (x.bits() & 1u) ? y : x;                        // a tie: the even encoding
	}

	// ---- 1. correctly-rounded vs the independent double-path oracle -----------
	int VerifyFmaVsOracle(bool reportTestCases, int nrTests, uint64_t seed) {
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_int_distribution<uint32_t> B(0, 0xFFFF);
		for (int t = 0; t < nrTests; ++t) {
			bfloat16 a = bf16_from_bits(uint16_t(B(rng)));
			bfloat16 b = bf16_from_bits(uint16_t(B(rng)));
			bfloat16 c = bf16_from_bits(uint16_t(B(rng)));
			bfloat16 r = fma(a, b, c);
			bfloat16 e = fma_oracle(a, b, c);
			if (!match(r, e)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma(" << a << ", " << b << ", " << c << ") = " << r
					<< " (" << to_binary(r) << ")  expected " << e << " (" << to_binary(e) << ")\n";
			}
		}
		return fails;
	}

	// exhaustive over a with random (b,c): covers every bfloat16 first operand,
	// including subnormals / inf / NaN.
	int VerifyFmaExhaustiveA(bool reportTestCases, int samplesPerA, uint64_t seed) {
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_int_distribution<uint32_t> B(0, 0xFFFF);
		for (uint32_t abits = 0; abits <= 0xFFFF; ++abits) {
			bfloat16 a = bf16_from_bits(uint16_t(abits));
			for (int s = 0; s < samplesPerA; ++s) {
				bfloat16 b = bf16_from_bits(uint16_t(B(rng)));
				bfloat16 c = bf16_from_bits(uint16_t(B(rng)));
				if (!match(fma(a, b, c), fma_oracle(a, b, c))) {
					++fails;
					if (reportTestCases && fails < 20) std::cout << "    FAIL fma(" << a << ", " << b << ", " << c << ")\n";
				}
			}
		}
		return fails;
	}

	// ---- 2. fused single-rounding beats the naive two-rounding a*b + c ---------
	int VerifyFmaFusedBeatsNaive(bool reportTestCases, int nrTests, uint64_t seed, int& disagreements) {
		int fails = 0;
		disagreements = 0;
		std::mt19937_64 rng(seed);
		std::uniform_int_distribution<uint32_t> B(0, 0xFFFF);
		for (int t = 0; t < nrTests; ++t) {
			bfloat16 a = bf16_from_bits(uint16_t(B(rng)));
			bfloat16 b = bf16_from_bits(uint16_t(B(rng)));
			bfloat16 c = bf16_from_bits(uint16_t(B(rng)));
			bfloat16 e = fma_oracle(a, b, c);
			if (!match(fma(a, b, c), e)) {  // fma must ALWAYS be correctly rounded
				++fails;
				if (reportTestCases) std::cout << "    FAIL (fused != oracle) fma(" << a << ", " << b << ", " << c << ")\n";
			}
			bfloat16 naive = a * b; naive = naive + c;   // two bfloat16 roundings
			if (isnan(e) || isnan(naive)) continue;      // only count finite disagreements
			if (!match(naive, e)) ++disagreements;
		}
		return fails;
	}

	// ---- 3. IEEE special values -----------------------------------------------
	int VerifySpecialValues(bool reportTestCases) {
		int fails = 0;
		const bfloat16 inf  = bf16_from_bits(0x7F80);   // +inf
		const bfloat16 ninf = bf16_from_bits(0xFF80);   // -inf
		const bfloat16 zero(0.0f), one(1.0f), two(2.0f), three(3.0f);
		auto chk = [&](bfloat16 r, bfloat16 e, const char* tag) {
			if (!match(r, e)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL special: " << tag << " -> " << r << " expected " << e << '\n';
			}
		};
		chk(fma(inf, zero, one),  fma_oracle(inf, zero, one),  "inf*0+1 (NaN)");
		chk(fma(inf, two, three), fma_oracle(inf, two, three), "inf*2+3 (inf)");
		chk(fma(two, three, inf), fma_oracle(two, three, inf), "2*3+inf (inf)");
		chk(fma(two, three, ninf),fma_oracle(two, three, ninf),"2*3-inf (-inf)");
		chk(fma(ninf, two, one),  fma_oracle(ninf, two, one),  "-inf*2+1 (-inf)");
		// the exact negative overflow threshold, -511 * 2^119, is a tie that rounds to -inf (CodeRabbit, #2110)
		chk(fma(bf16_from_bits(0xDEE0), bf16_from_bits(0x6012), zero), ninf, "-511 * 2^119 (the overflow tie) -> -inf");
		chk(fma_oracle(bf16_from_bits(0xDEE0), bf16_from_bits(0x6012), zero), ninf, "oracle: the overflow tie -> -inf");
		chk(fma(bf16_from_bits(0x5EE0), bf16_from_bits(0x6012), zero), inf, "+511 * 2^119 (the overflow tie) -> +inf");
		// an infinite result from finite and infinite operands raises no invalid-operation flag
		std::feclearexcept(FE_ALL_EXCEPT);
		const bfloat16 rinf = fma(inf, two, three);
		if (std::fetestexcept(FE_INVALID) != 0 || !(rinf == inf)) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(inf,2,3) raised FE_INVALID\n"; }
		// NaN propagation
		const bfloat16 nan = bf16_from_bits(0x7FC0);
		if (!isnan(fma(nan, one, one))) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(NaN,1,1) not NaN\n"; }
		if (!isnan(fma(one, nan, one))) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,NaN,1) not NaN\n"; }
		if (!isnan(fma(one, one, nan))) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,1,NaN) not NaN\n"; }
		// exact identities
		chk(fma(two, three, zero), bfloat16(6.0f), "2*3+0 == 6");
		chk(fma(zero, three, two), two,            "0*3+2 == 2");
		chk(fma(one, three, two),  bfloat16(5.0f), "1*3+2 == 5");
		// deterministic round-to-nearest-even ties (ulp at 1.0 is 2^-7):
		//   1 + 2^-8   is the midpoint of 1.0 and 1+2^-7   -> even significand 1.0
		//   1 + 3*2^-8 is the midpoint of 1+2^-7 and 1+2^-6 -> even significand 1+2^-6
		const bfloat16 halfUlp(std::ldexp(1.0f, -8));         // 2^-8
		const bfloat16 threeHalfUlp(std::ldexp(3.0f, -8));    // 3*2^-8
		chk(fma(one, one, halfUlp),      one,                              "1*1+2^-8 tie -> 1.0 (even down)");
		chk(fma(one, one, threeHalfUlp), bfloat16(1.0f + std::ldexp(1.0f, -6)), "1*1+3*2^-8 tie -> 1+2^-6 (even up)");
		return fails;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite = "bfloat16 fused multiply-add fma(a,b,c) (#1193)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifySpecialValues(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 20000;
	int samplesPerA = 2;
#if REGRESSION_LEVEL_2
	base = 100000;
	samplesPerA = 8;
#endif

	nrOfFailedTestCases += ReportTestResult(
		VerifyFmaVsOracle(reportTestCases, base, 0xB16F), "fma vs double-path oracle", "fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFmaExhaustiveA(reportTestCases, samplesPerA, 0x0A16), "fma exhaustive first operand", "fma");
	{
		int disagreements = 0;
		nrOfFailedTestCases += ReportTestResult(
			VerifyFmaFusedBeatsNaive(reportTestCases, base, 0xF00D, disagreements), "fma fused == oracle", "fma");
		std::cout << "    (fused vs naive: the two-rounding bfloat16 a*b + c disagreed with the exact result in "
			<< disagreements << " of " << base << " finite cases -- fma is single-rounded)\n";
	}
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecialValues(reportTestCases), "fma IEEE special values", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
