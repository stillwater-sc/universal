// fma.cpp: functional tests for the fixed-size integer fused multiply-add fma(a,b,c)
//
// integer had no fma (#1192, sub-issue of the universal fma epic #1189). Integer
// arithmetic is exact, so fma(a,b,c) = a*b + c has no fused-vs-two-rounding
// distinction; the overload exists so generic ADL code compiles over integer.
// Fixed-size integer arithmetic is modular, so the result is (a*b + c) mod 2^nbits,
// consistent with integer's own operator* / operator+. This suite validates against
// an INDEPENDENT int64 oracle (exact a*b + c, then wrapped to nbits), checks that
// fma agrees with the operator form a*b + c, verifies identities, and confirms the
// modular overflow behavior on the maxpos*maxpos+maxpos corner.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/numeric_limits.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// exact a*b + c, reduced into signed nbits two's complement (integer is modular)
	template<unsigned nbits>
	int64_t fma_oracle(int64_t A, int64_t B, int64_t C) {
		static_assert(nbits <= 32, "int64 oracle: A*B must fit in int64 (nbits <= 32)");
		const int64_t full = A * B + C;                       // exact for nbits <= 32
		const uint64_t mask = (nbits >= 64) ? ~0ull : ((1ull << nbits) - 1);
		uint64_t u = static_cast<uint64_t>(full) & mask;
		if (nbits < 64 && (u & (1ull << (nbits - 1)))) return static_cast<int64_t>(u | ~mask);
		return static_cast<int64_t>(u);
	}

	// ---- 1. correctly computed vs the independent int64 modular oracle --------
	template<unsigned nbits, typename bt = uint32_t>
	int VerifyFmaVsOracle(bool reportTestCases, int nrTests, uint64_t seed) {
		using Integer = integer<nbits, bt, IntegerNumberType::IntegerNumber>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		const int64_t lo = -((int64_t)1 << (nbits - 1));
		const int64_t hi =  ((int64_t)1 << (nbits - 1)) - 1;
		std::uniform_int_distribution<int64_t> D(lo, hi);
		for (int t = 0; t < nrTests; ++t) {
			int64_t va = D(rng), vb = D(rng), vc = D(rng);
			Integer a(va), b(vb), c(vc);
			Integer r = fma(a, b, c);
			Integer expected(fma_oracle<nbits>(va, vb, vc));
			if (r != expected) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma(" << va << ", " << vb << ", " << vc << ") = " << r
					<< "  expected " << expected << '\n';
			}
			// fma must agree with the plain operator form a*b + c (same modular semantics)
			if (r != (a * b + c)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma != a*b+c for (" << va << ", " << vb << ", " << vc << ")\n";
			}
		}
		return fails;
	}

	// exhaustive over (a,b) for a small width, with a few fixed c, vs the oracle
	template<unsigned nbits, typename bt = uint32_t>
	int VerifyFmaExhaustiveAB(bool reportTestCases) {
		using Integer = integer<nbits, bt, IntegerNumberType::IntegerNumber>;
		int fails = 0;
		const int64_t lo = -((int64_t)1 << (nbits - 1));
		const int64_t hi =  ((int64_t)1 << (nbits - 1)) - 1;
		const int64_t cs[] = { 0, 1, -1, hi, lo, hi / 2 };
		for (int64_t vc : cs) {
			Integer c(vc);
			for (int64_t va = lo; va <= hi; ++va) {
				Integer a(va);
				for (int64_t vb = lo; vb <= hi; ++vb) {
					Integer b(vb);
					Integer r = fma(a, b, c);
					Integer expected(fma_oracle<nbits>(va, vb, vc));
					if (r != expected) {
						++fails;
						if (reportTestCases) std::cout << "    FAIL fma(" << va << ", " << vb << ", " << vc << ") = " << r
							<< "  expected " << expected << '\n';
					}
				}
			}
		}
		return fails;
	}

	// ---- 2. identities --------------------------------------------------------
	template<unsigned nbits, typename bt = uint32_t>
	int VerifyFmaIdentities(bool reportTestCases, int nrTests, uint64_t seed) {
		using Integer = integer<nbits, bt, IntegerNumberType::IntegerNumber>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		const int64_t lo = -((int64_t)1 << (nbits - 1));
		const int64_t hi =  ((int64_t)1 << (nbits - 1)) - 1;
		std::uniform_int_distribution<int64_t> D(lo, hi);
		Integer zero(0), one(1);
		for (int t = 0; t < nrTests; ++t) {
			Integer a(D(rng)), b(D(rng)), c(D(rng));
			if (fma(a, b, zero) != (a * b)) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(a,b,0) != a*b\n"; }
			if (fma(zero, b, c) != c)       { ++fails; if (reportTestCases) std::cout << "    FAIL fma(0,b,c) != c\n"; }
			if (fma(one, b, c)  != (b + c)) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,b,c) != b+c\n"; }
		}
		return fails;
	}

	// ---- 3. explicit modular-overflow corner ----------------------------------
	int VerifyOverflowCorner(bool reportTestCases) {
		int fails = 0;
		constexpr unsigned nbits = 16;
		using Integer = integer<nbits, uint16_t, IntegerNumberType::IntegerNumber>;
		const int64_t A = ((int64_t)1 << (nbits - 1)) - 1;   // maxpos = 32767
		Integer mx(A);
		Integer expected(fma_oracle<nbits>(A, A, A));
		if (fma(mx, mx, mx) != expected) {
			++fails; if (reportTestCases) std::cout << "    FAIL maxpos*maxpos+maxpos modular wrap mismatch\n";
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
	std::string test_suite = "fixed-size integer fused multiply-add fma(a,b,c) (#1192)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyFmaExhaustiveAB<8>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 5000;
#if REGRESSION_LEVEL_2
	base = 20000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustiveAB<8>(reportTestCases),          "fma exhaustive(a,b) integer<8>",   "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<16>(reportTestCases, base, 0xA1), "fma vs oracle integer<16>",        "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<24>(reportTestCases, base, 0xB2), "fma vs oracle integer<24>",        "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaVsOracle<32>(reportTestCases, base, 0xC3), "fma vs oracle integer<32>",        "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaIdentities<16>(reportTestCases, base / 4 + 1, 0xD4), "fma identities integer<16>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyOverflowCorner(reportTestCases),              "fma modular-overflow corner",      "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
