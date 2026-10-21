// subnormal_rounding.cpp: regression tests for efloat -> float/double rounding at the bottom of the IEEE range
//
// The conversion mishandled three cases below 2^E_min:
//   - a value in [2^(E_min-K), 2^(E_min-K+1)), whose leading bit is the guard bit of the
//     smallest subnormal, was treated as below half of it, so round-to-nearest returned 0;
//   - rounding up from below the smallest subnormal shifted a 64-bit word by 64 (undefined
//     behaviour, 0 on x86) instead of returning the smallest subnormal;
//   - a carry out of a subnormal's significand jumped to the smallest normal 2^E_min
//     instead of the next binade up.
// The reference below rounds the exact value to an integer multiple of the target's ulp
// one bit at a time, and scales that integer, which ldexp does exactly.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

constexpr unsigned nrLimbs = 4;  // 128-bit operands: two limbs past what float or double keeps

// the value 0.L[0]L[1]L[2]L[3] * 2^(e+1), MSB of L[0] set, correctly rounded to Real
template<typename Real>
Real reference(bool negative, const uint32_t (&L)[nrLimbs], int e, sw::universal::RoundingMode mode) {
	using namespace sw::universal;
	constexpr int K     = std::numeric_limits<Real>::digits;
	constexpr int E_min = std::numeric_limits<Real>::min_exponent - 1;
	const int     q     = (e > E_min ? e : E_min) - (K - 1);  // exponent of the result's ulp
	uint64_t      n     = 0;
	bool          guard = false, sticky = false;
	for (int i = 0; i < int(32 * nrLimbs); ++i) {
		bool bit = ((L[i / 32] >> (31 - i % 32)) & 1u) != 0;  // weight 2^(e - i)
		if (e - i >= q)          n = (n << 1) | (bit ? 1u : 0u);
		else if (e - i == q - 1) guard = bit;
		else                     sticky = sticky || bit;
	}
	bool up = false;
	switch (mode) {
	case RoundingMode::RoundToNearest:      up = guard && (sticky || (n & 1u)); break;
	case RoundingMode::RoundToZero:         up = false; break;
	case RoundingMode::RoundTowardPositive: up = !negative && (guard || sticky); break;
	case RoundingMode::RoundTowardNegative: up = negative && (guard || sticky); break;
	}
	if (up) ++n;
	Real r = std::ldexp(static_cast<Real>(n), q);  // n <= 2^K, so exact
	return negative ? -r : r;
}

template<typename Real>
int VerifySubnormalRounding(bool reportTestCases, int e_lo, int e_hi) {
	using namespace sw::universal;
	using E = efloat<nrLimbs>;
	const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
	const RoundingMode saved   = efloat_rounding_mode;
	std::mt19937_64    rng(1777);
	int                failures = 0;
	for (int e = e_lo; e <= e_hi; ++e) {
		for (int k = 0; k < 64; ++k) {
			uint32_t L[nrLimbs];
			for (unsigned j = 0; j < nrLimbs; ++j) L[j] = static_cast<uint32_t>(rng());
			switch (k % 4) {
			case 0: break;                                                    // random, inexact
			case 1: L[2] = L[3] = 0; break;                                   // 64 significant bits
			case 2: L[0] = 0; L[1] = L[2] = L[3] = 0; break;                  // a power of 2: ties
			case 3: L[0] = 0; L[1] = L[2] = 0; L[3] = 1; break;               // a power of 2 plus sticky
			}
			L[0] |= 0x80000000u;
			bool negative = (k & 8) != 0;
			E    a(1);
			a.set_precision(32 * nrLimbs);
			for (unsigned j = 0; j < nrLimbs; ++j) a.setlimb(j, L[j]);
			a.setexponent(e);
			if (negative) a = -a;
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = mode;
				Real got = static_cast<Real>(a);
				Real ref = reference<Real>(negative, L, e, mode);
				if (std::memcmp(&got, &ref, sizeof(Real)) != 0) {
					if (reportTestCases && failures < 16)
						std::cout << "    FAIL: scale " << e << " case " << k << " mode " << int(mode) << ": " << std::hexfloat
						          << got << " != " << ref << std::defaultfloat << '\n';
					++failures;
				}
			}
		}
	}
	efloat_rounding_mode = saved;
	return failures;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat conversion to float/double at the bottom of the range";
	std::string test_tag            = "subnormal rounding";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySubnormalRounding<double>(true, -1076, -1072), "efloat->double", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	// scales from below half the smallest subnormal, through the subnormals, into the normals
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalRounding<float>(reportTestCases, -160, -120), "efloat->float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalRounding<double>(reportTestCases, -1085, -1015), "efloat->double", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalRounding<float>(reportTestCases, -300, 0), "efloat->float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalRounding<double>(reportTestCases, -1300, 0), "efloat->double", test_tag);
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
