// fma.cpp: functional tests for the fixed-point fused multiply-add fma(a,b,c)
//
// fixpnt had no fma (#1191, sub-issue of the universal fma epic #1189). The new
// free fma(a,b,c) forms a*b + c in a wide exact intermediate and rounds ONCE.
// This suite validates it against an INDEPENDENT integer oracle: a,b,c are exact
// multiples of 2^-rbits, so with A,B,C the signed integer bit-patterns the exact
// real value of a*b + c is (A*B + C*2^rbits) / 2^(2*rbits). The oracle rounds
// that exact rational round-to-nearest-even at rbits (matching blockbinary's
// roundingMode) and then applies the type's range policy (Modulo wrap / Saturate
// clamp) -- all in __int128, entirely independent of the implementation. We check:
//   - correctly-rounded: fma(a,b,c) equals the oracle for random operands, in
//     both Modulo and Saturate arithmetic;
//   - fused vs naive: fma matches the oracle in cases where the two-rounding
//     fixpnt(a*b) + c does not (single rounding is strictly more accurate);
//   - saturation / overflow: large a*b + c saturates (Saturate) or wraps (Modulo);
//   - identities: fma(a,b,0) == a*b, fma(0,b,c) == c, fma(1,b,c) == b + c (rounded).
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

#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// signed integer value of an nbits two's-complement bit pattern
	int64_t signextend(uint64_t bits, unsigned nbits) {
		uint64_t mask = (nbits >= 64) ? ~0ull : ((1ull << nbits) - 1);
		bits &= mask;
		if (nbits < 64 && (bits & (1ull << (nbits - 1)))) return static_cast<int64_t>(bits | ~mask);
		return static_cast<int64_t>(bits);
	}

	// floor division (C++ '/' truncates toward zero; we need floor for negative numerators)
	__int128 floordiv(__int128 num, __int128 den) {
		__int128 q = num / den, r = num - q * den;
		if (r != 0 && ((r < 0) != (den < 0))) --q;
		return q;
	}

	// exact a*b + c rounded round-to-nearest-even at rbits, then range-limited.
	// Returns the signed integer bit-pattern the fma result must carry.
	template<unsigned nbits, unsigned rbits, bool arithmetic>
	int64_t fma_oracle(int64_t A, int64_t B, int64_t C) {
		const __int128 den = (__int128)1 << rbits;                 // 2^rbits
		const __int128 num = (__int128)A * B + (__int128)C * den;  // (A*B + C*2^rbits), in units of 2^-2rbits scaled...
		// value = num / 2^(2rbits); its bit-pattern (value * 2^rbits) = num / 2^rbits, rounded.
		__int128 q = floordiv(num, den);
		__int128 r = num - q * den;                                // 0 <= r < den
		__int128 twice = r << 1;
		if (twice > den)      ++q;
		else if (twice == den && (q & 1)) ++q;                     // exact tie -> round to even
		// q is the ideal (unbounded) result bit-pattern
		const int64_t maxpos =  ((int64_t)1 << (nbits - 1)) - 1;
		const int64_t maxneg = -((int64_t)1 << (nbits - 1));
		if constexpr (arithmetic == Saturate) {
			if (q > maxpos) return maxpos;
			if (q < maxneg) return maxneg;
			return (int64_t)q;
		}
		else { // Modulo: reduce into signed nbits two's complement
			return signextend((uint64_t)(q & (((__int128)1 << nbits) - 1)), nbits);
		}
	}

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	fixpnt<nbits, rbits, arithmetic, bt> from_bits(uint64_t bits) {
		fixpnt<nbits, rbits, arithmetic, bt> f;
		f.setbits(bits);
		return f;
	}

	// value that the ideal result bit-pattern E represents, as a double (exact for the small nbits under test)
	double expected_value(int64_t E, unsigned rbits) { return std::ldexp(static_cast<double>(E), -static_cast<int>(rbits)); }

	// ---- 1. correctly-rounded vs the independent integer oracle ---------------
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt = uint32_t>
	int VerifyFmaVsOracle(bool reportTestCases, int nrTests, uint64_t seed) {
		using Fixpnt = fixpnt<nbits, rbits, arithmetic, bt>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		const uint64_t mask = (nbits >= 64) ? ~0ull : ((1ull << nbits) - 1);
		for (int t = 0; t < nrTests; ++t) {
			uint64_t ra = rng() & mask, rb = rng() & mask, rc = rng() & mask;
			Fixpnt a = from_bits<nbits, rbits, arithmetic, bt>(ra);
			Fixpnt b = from_bits<nbits, rbits, arithmetic, bt>(rb);
			Fixpnt c = from_bits<nbits, rbits, arithmetic, bt>(rc);
			Fixpnt r = fma(a, b, c);
			int64_t E = fma_oracle<nbits, rbits, arithmetic>(signextend(ra, nbits), signextend(rb, nbits), signextend(rc, nbits));
			if (double(r) != expected_value(E, rbits)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma(" << a << ", " << b << ", " << c << ") = " << r
					<< "  expected " << expected_value(E, rbits) << '\n';
			}
		}
		return fails;
	}

	// exhaustive over (a,b) for a small config, with a few fixed c, vs the oracle
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt = uint32_t>
	int VerifyFmaExhaustiveAB(bool reportTestCases) {
		using Fixpnt = fixpnt<nbits, rbits, arithmetic, bt>;
		int fails = 0;
		const uint64_t N = (1ull << nbits);
		const uint64_t cs[] = { 0ull, 1ull, N - 1, (N >> 1), (N >> 1) + 1, (N >> 2) };
		for (uint64_t rc : cs) {
			Fixpnt c = from_bits<nbits, rbits, arithmetic, bt>(rc);
			for (uint64_t ra = 0; ra < N; ++ra) {
				Fixpnt a = from_bits<nbits, rbits, arithmetic, bt>(ra);
				for (uint64_t rb = 0; rb < N; ++rb) {
					Fixpnt b = from_bits<nbits, rbits, arithmetic, bt>(rb);
					Fixpnt r = fma(a, b, c);
					int64_t E = fma_oracle<nbits, rbits, arithmetic>(signextend(ra, nbits), signextend(rb, nbits), signextend(rc, nbits));
					if (double(r) != expected_value(E, rbits)) {
						++fails;
						if (reportTestCases) std::cout << "    FAIL fma(" << a << ", " << b << ", " << c << ") = " << r
							<< "  expected " << expected_value(E, rbits) << '\n';
					}
				}
			}
		}
		return fails;
	}

	// ---- 2. fused single-rounding beats the two-rounding fixpnt(a*b) + c -------
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt = uint32_t>
	int VerifyFmaFusedBeatsNaive(bool reportTestCases, int nrTests, uint64_t seed, int& disagreements) {
		using Fixpnt = fixpnt<nbits, rbits, arithmetic, bt>;
		int fails = 0;
		disagreements = 0;
		std::mt19937_64 rng(seed);
		const uint64_t mask = (nbits >= 64) ? ~0ull : ((1ull << nbits) - 1);
		for (int t = 0; t < nrTests; ++t) {
			uint64_t ra = rng() & mask, rb = rng() & mask, rc = rng() & mask;
			Fixpnt a = from_bits<nbits, rbits, arithmetic, bt>(ra);
			Fixpnt b = from_bits<nbits, rbits, arithmetic, bt>(rb);
			Fixpnt c = from_bits<nbits, rbits, arithmetic, bt>(rc);
			Fixpnt fused = fma(a, b, c);
			Fixpnt naive = a * b; naive += c;                 // two roundings
			int64_t E = fma_oracle<nbits, rbits, arithmetic>(signextend(ra, nbits), signextend(rb, nbits), signextend(rc, nbits));
			double exact_rounded = expected_value(E, rbits);
			// fma must ALWAYS be the correctly-rounded (oracle) value
			if (double(fused) != exact_rounded) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL (fused != oracle) fma(" << a << ", " << b << ", " << c << ") = " << fused
					<< " expected " << exact_rounded << '\n';
			}
			// count the cases where the naive two-rounding form is wrong (fma's advantage)
			if (double(naive) != exact_rounded) ++disagreements;
		}
		return fails;
	}

	// ---- 3. identities --------------------------------------------------------
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt = uint32_t>
	int VerifyFmaIdentities(bool reportTestCases, int nrTests, uint64_t seed) {
		using Fixpnt = fixpnt<nbits, rbits, arithmetic, bt>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		const uint64_t mask = (nbits >= 64) ? ~0ull : ((1ull << nbits) - 1);
		Fixpnt zero(0), one(1);
		for (int t = 0; t < nrTests; ++t) {
			Fixpnt a = from_bits<nbits, rbits, arithmetic, bt>(rng() & mask);
			Fixpnt b = from_bits<nbits, rbits, arithmetic, bt>(rng() & mask);
			Fixpnt c = from_bits<nbits, rbits, arithmetic, bt>(rng() & mask);
			// fma(a,b,0) == a*b
			if (fma(a, b, zero) != (a * b)) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(a,b,0) != a*b\n"; }
			// fma(0,b,c) == c
			if (fma(zero, b, c) != c) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(0,b,c) != c\n"; }
			// fma(1,b,c) == b + c
			if (fma(one, b, c) != (b + c)) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,b,c) != b+c\n"; }
		}
		return fails;
	}

	// ---- 4. explicit saturation / overflow corners ----------------------------
	int VerifySaturationCorners(bool reportTestCases) {
		int fails = 0;
		// Saturate: maxpos*maxpos + maxpos overflows -> clamp to maxpos
		{
			using F = fixpnt<8, 4, Saturate, uint8_t>;
			F mx(SpecificValue::maxpos), mn(SpecificValue::maxneg);
			if (fma(mx, mx, mx) != mx) { ++fails; if (reportTestCases) std::cout << "    FAIL Saturate maxpos*maxpos+maxpos != maxpos\n"; }
			// maxneg*maxpos + maxneg  underflows -> clamp to maxneg
			if (fma(mn, mx, mn) != mn) { ++fails; if (reportTestCases) std::cout << "    FAIL Saturate maxneg*maxpos+maxneg != maxneg\n"; }
		}
		// Modulo: result wraps; compare against the integer oracle
		{
			constexpr unsigned nbits = 8, rbits = 4;
			using F = fixpnt<nbits, rbits, Modulo, uint8_t>;
			F mx(SpecificValue::maxpos);
			int64_t A = signextend(static_cast<uint64_t>(mx.bits()), nbits);
			int64_t E = fma_oracle<nbits, rbits, Modulo>(A, A, A);
			if (double(fma(mx, mx, mx)) != std::ldexp(double(E), -int(rbits))) {
				++fails; if (reportTestCases) std::cout << "    FAIL Modulo maxpos*maxpos+maxpos wrap mismatch\n";
			}
		}
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
	std::string test_suite = "fixed-point (fixpnt) fused multiply-add fma(a,b,c) (#1191)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// hand-run diagnostics (this branch ignores failures)
	nrOfFailedTestCases += VerifyFmaExhaustiveAB<8, 4, Modulo>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 5000;
#if REGRESSION_LEVEL_2
	base = 20000;
#endif

	// exhaustive (a,b) x fixed-c, small configs, both arithmetic policies
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustiveAB<8, 4, Modulo>(reportTestCases),   "fma exhaustive(a,b) fixpnt<8,4,Modulo>",   "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustiveAB<8, 4, Saturate>(reportTestCases), "fma exhaustive(a,b) fixpnt<8,4,Saturate>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustiveAB<8, 0, Modulo>(reportTestCases),   "fma exhaustive(a,b) fixpnt<8,0,Modulo>",   "fma");

	// randomized correctly-rounded vs oracle, wider configs, both policies
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<16, 8, Modulo>(reportTestCases, base, 0xF1A),   "fma vs oracle fixpnt<16,8,Modulo>",   "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<16, 8, Saturate>(reportTestCases, base, 0xF2B), "fma vs oracle fixpnt<16,8,Saturate>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<24, 12, Modulo>(reportTestCases, base, 0xF3C),  "fma vs oracle fixpnt<24,12,Modulo>",  "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<32, 16, Saturate>(reportTestCases, base, 0xF4D),"fma vs oracle fixpnt<32,16,Saturate>","fma");

	// fused single-rounding is always correctly rounded (and beats naive on some inputs)
	{
		int disagreements = 0;
		nrOfFailedTestCases += ReportTestResult(VerifyFmaFusedBeatsNaive<16, 8, Modulo>(reportTestCases, base, 0xBEEF, disagreements), "fma fused == oracle fixpnt<16,8,Modulo>", "fma");
		std::cout << "    (fused vs naive: naive two-rounding disagreed with the exact result in "
			<< disagreements << " of " << base << " cases -- fma is single-rounded)\n";
	}

	// identities and saturation corners
	nrOfFailedTestCases += ReportTestResult(VerifyFmaIdentities<16, 8, Modulo>(reportTestCases, base / 4 + 1, 0x1D),   "fma identities fixpnt<16,8,Modulo>",   "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaIdentities<16, 8, Saturate>(reportTestCases, base / 4 + 1, 0x2E), "fma identities fixpnt<16,8,Saturate>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifySaturationCorners(reportTestCases), "fma saturation/overflow corners", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
