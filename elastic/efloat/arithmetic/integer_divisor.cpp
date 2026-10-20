// integer_divisor.cpp: efloat's in-place paths agree bit for bit with the general ones (#1761)
//
//   x / n    a divisor of one limb divides x in place, a limb at a time by a reciprocal; the
//            result must equal x / efloat(n), the general long division, in sign, scale, limbs
//            and precision, in every rounding mode.  Dividends of the form d Q 2^(32 m) + 1,
//            longer than the quotient window, leave only the remainder to say the quotient is
//            inexact, so the sticky bit is exercised.
//   x * x    squares with half the limb products; it must equal x * y for a copy y of x.
//   x += x, x -= x, x *= x   the operand is the destination.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 2,000 random dividends per precision
//   LEVEL 2 -- 20,000
//   LEVEL 3 -- 200,000
//   LEVEL 4 -- 2,000,000
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using Real = sw::universal::efloat<64>;

	bool identical(const Real& a, const Real& b) {
		return a.sign() == b.sign() && a.scale() == b.scale() && a.bits() == b.bits()
		    && a.get_precision() == b.get_precision() && a.carries_precision() == b.carries_precision();
	}

	// a random significand of nr_limbs limbs, which may exceed the precision of p limbs
	Real random_value(std::mt19937_64& rng, unsigned precision_limbs, unsigned nr_limbs) {
		std::vector<uint32_t> limbs(nr_limbs);
		for (auto& l : limbs) l = static_cast<uint32_t>(rng());
		if (rng() % 9 == 0) {   // long runs of ones and zeros
			for (size_t i = 1; i < limbs.size(); ++i) limbs[i] = (rng() & 1u) ? 0xFFFF'FFFFu : 0u;
		}
		limbs[0] |= 0x8000'0000u;
		const int64_t scale = static_cast<int64_t>(rng() % 201) - 100;
		Real v(static_cast<bool>(rng() & 1u), scale, limbs);
		v.set_precision(32 * precision_limbs);
		return v;
	}

	// d Q 2^(32 m) + 1 for a random Q, left-justified: x / d has zero limbs past Q and a nonzero
	// remainder
	Real sticky_only(std::mt19937_64& rng, unsigned precision_limbs, uint32_t d) {
		std::vector<uint32_t> q(1 + rng() % precision_limbs);
		for (auto& l : q) l = static_cast<uint32_t>(rng());
		q[0] |= 1u;
		std::vector<uint32_t> a(q.size() + 1, 0u);
		uint64_t carry = 0;
		for (size_t i = q.size(); i-- > 0;) {
			const uint64_t p = uint64_t(q[i]) * d + carry;
			a[i + 1] = static_cast<uint32_t>(p);
			carry = p >> 32;
		}
		a[0] = static_cast<uint32_t>(carry);
		if (a[0] == 0u) a.erase(a.begin());
		a.resize(a.size() + precision_limbs + 4 + rng() % 3, 0u);
		a.back() = 1u;
		unsigned shift = 0;
		while ((a[0] & (0x8000'0000u >> shift)) == 0u) ++shift;
		if (shift != 0u) {
			for (size_t i = 0; i + 1 < a.size(); ++i) a[i] = (a[i] << shift) | (a[i + 1] >> (32u - shift));
			a.back() <<= shift;
		}
		// set the precision first: the limbs run past it, and set_precision would round them away
		Real v(1);
		v.set_precision(32 * precision_limbs);
		for (size_t i = 0; i < a.size(); ++i) v.setlimb(static_cast<unsigned>(i), a[i]);
		v.setexponent(static_cast<int64_t>(rng() % 21) - 10);
		return v;
	}

	uint32_t random_divisor(std::mt19937_64& rng, unsigned i) {
		uint64_t d = 0;
		switch (i % 5) {
		case 0:  d = 3 + rng() % 61; break;                                          // small
		case 1:  d = 0xFFFF'FFFFull - rng() % 16; break;                              // largest
		case 2:  d = (uint64_t(1) << (1 + rng() % 31)) + 1; break;                    // just past a power of 2
		case 3:  d = (2 * (rng() % 300) + 1) * (2 * (rng() % 300) + 2); break;        // a series divisor
		default: d = 3 + (rng() & 0xFFFF'FFFFull); break;
		}
		if ((d & (d - 1)) == 0) ++d;   // powers of 2 take the general path
		return static_cast<uint32_t>(std::min<uint64_t>(d, 0xFFFF'FFFFull));
	}

	int VerifyIntegerDivisor(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1761);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const unsigned precisions[] = { 1, 2, 5, 16, 64 };   // in limbs
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned p : precisions) {
			int failures = 0;
			for (unsigned i = 0; i < nrSamples; ++i) {
				const uint32_t d = random_divisor(rng, i);
				Real x = (i % 4 == 3) ? sticky_only(rng, p, d) : random_value(rng, p, 1 + static_cast<unsigned>(rng() % (p + 3)));
				if (i % 13 == 0) x.clear_precision();
				efloat_rounding_mode = modes[i % 4];
				const int64_t sd = (i & 1u) ? -static_cast<int64_t>(d) : static_cast<int64_t>(d);
				const bool ok = identical(x / d, x / Real(d)) && identical(x / sd, x / Real(sd));
				efloat_rounding_mode = saved;
				if (!ok) {
					++failures;
					if (reportTestCases && failures <= 3) std::cout << "    FAIL: x / " << sd << " at " << 32 * p << " bits differs from the general division\n";
				}
			}
			nrOfFailedTestCases += failures;
		}
		return nrOfFailedTestCases;
	}

	int VerifySquareAndAliasing(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(17610);
		const unsigned precisions[] = { 1, 2, 5, 16, 64 };   // in limbs
		for (unsigned p : precisions) {
			for (unsigned i = 0; i < nrSamples; ++i) {
				const Real x = random_value(rng, p, p);
				const Real y = x;   // a copy: x * y takes the general multiplication
				Real s = x;  s *= s;
				Real t = x;  t += t;
				Real u = x;  u -= u;
				const bool ok = identical(x * x, x * y) && identical(s, x * y) && identical(t, x + y) && u.iszero();
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: x * x, x += x or x -= x at " << 32 * p << " bits differs from the general path\n";
				}
			}
		}
		return nrOfFailedTestCases;
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

	std::string test_suite          = "efloat in-place integer division and squaring";
	std::string test_tag            = "integer divisor";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisor(reportTestCases, 200), "efloat", "x / n");
	nrOfFailedTestCases += ReportTestResult(VerifySquareAndAliasing(reportTestCases, 100), "efloat", "x * x and aliasing");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisor(reportTestCases, 2000), "efloat", "x / n");
	nrOfFailedTestCases += ReportTestResult(VerifySquareAndAliasing(reportTestCases, 500), "efloat", "x * x and aliasing");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisor(reportTestCases, 20000), "efloat", "x / n");
	nrOfFailedTestCases += ReportTestResult(VerifySquareAndAliasing(reportTestCases, 5000), "efloat", "x * x and aliasing");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisor(reportTestCases, 200000), "efloat", "x / n");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisor(reportTestCases, 2000000), "efloat", "x / n");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
