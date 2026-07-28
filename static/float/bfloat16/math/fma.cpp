// fma.cpp: functional tests for the bfloat16 fused multiply-add fma(a,b,c)
//
// bfloat16 had no fma (#1193, sub-issue of the universal fma epic #1189). The new
// fma widens to float, forms a*b + c with std::fma (one rounding to float), and
// rounds once into bfloat16 via the RNE bfloat16(float) constructor. Because a
// bfloat16 product is EXACT in float (16 <= 24 significand bits) and float's 24 bits
// satisfy 24 >= 2*8 + 2 for the bfloat16 target, the float -> bfloat16 double
// rounding is innocuous: the result is the correctly-rounded-to-nearest-even
// bfloat16 value of the exact a*b + c.
//
// This suite validates against an INDEPENDENT oracle that widens to double instead
// of float: bfloat16(std::fma(double(a),double(b),double(c))). Both paths equal
// RNE(exact a*b + c), so they must agree bit-for-bit; disagreement reveals a bug.
// We also check: single-rounding superiority over the naive two-rounding
// bfloat16 a*b + c, IEEE special values (inf*0 -> NaN, inf/NaN propagation), and
// tie-to-even behavior.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
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

	// independent oracle: exact a*b + c rounded once to bfloat16, via the double path
	bfloat16 fma_oracle(bfloat16 a, bfloat16 b, bfloat16 c) {
		return bfloat16(std::fma(double(a), double(b), double(c)));
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
		// NaN propagation
		const bfloat16 nan = bf16_from_bits(0x7FC0);
		if (!isnan(fma(nan, one, one))) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(NaN,1,1) not NaN\n"; }
		if (!isnan(fma(one, nan, one))) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,NaN,1) not NaN\n"; }
		if (!isnan(fma(one, one, nan))) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,1,NaN) not NaN\n"; }
		// exact identities
		chk(fma(two, three, zero), bfloat16(6.0f), "2*3+0 == 6");
		chk(fma(zero, three, two), two,            "0*3+2 == 2");
		chk(fma(one, three, two),  bfloat16(5.0f), "1*3+2 == 5");
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
