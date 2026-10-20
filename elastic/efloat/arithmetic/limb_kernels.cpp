// limb_kernels.cpp: efloat's multiplication and addition kernels against exact references (#1762)
//
//   x * y    at a precision wide enough to hold the whole product, so the result is exact: its
//            limbs must be the schoolbook product's, normalized, and no Inexact flag is raised.
//            Operands of 1 to 300 limbs reach the schoolbook kernel, Karatsuba (from 24 64-bit
//            limbs, 48 of efloat's), its odd splits, and the block loop for unbalanced operands;
//            odd limb counts exercise the zero limb the packing pads them with.
//   x * x    and x *= x take the squaring kernel.
//   x + y    and x - y align the operand with the smaller exponent to the other, keep two limbs
//            past the precision and fold whatever falls below them into a sticky bit.  The
//            result must be the exact sum, formed at a precision that holds all of it, rounded
//            once to the target, in every rounding mode: exponent gaps of 0 to 40 limbs, either
//            operand the longer, and near-cancellations, including one by an operand that carries
//            no precision and is longer than the target across a one-bit gap (#1767: it lost its
//            last bits to the sticky bit, and 2 - (2 - 2^-127) came out 2^-126 - 2^-158).
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 300 random products and the threshold lengths, 2,000 sums per precision
//   LEVEL 2 -- 3,000 and 20,000
//   LEVEL 3 -- 30,000 and 200,000
//   LEVEL 4 -- 300,000 and 2,000,000
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using Real = sw::universal::efloat<64>;

	// a normalized significand of n limbs, most significant first, with runs of all-ones and
	// all-zero limbs, which carry and borrow the furthest
	std::vector<uint32_t> random_limbs(std::mt19937_64& rng, size_t n) {
		std::vector<uint32_t> l(n);
		const unsigned style = static_cast<unsigned>(rng() % 4);
		for (auto& x : l) {
			const uint64_t r = rng();
			x = (style == 1) ? 0xFFFF'FFFFu : (style == 2 && (r & 3u) != 0u) ? ((r & 4u) ? 0xFFFF'FFFFu : 0u) : static_cast<uint32_t>(r >> 8);
		}
		l[0] |= 0x8000'0000u;
		if (rng() % 2) l.back() |= 1u;   // the last limb counts
		return l;
	}

	Real make(const std::vector<uint32_t>& limbs, int64_t exponent, bool negative, unsigned precision_bits) {
		Real v(1);
		v.set_precision(precision_bits);   // before the limbs: set_precision rounds to it
		for (size_t i = 0; i < limbs.size(); ++i) v.setlimb(static_cast<unsigned>(i), limbs[i]);
		v.setexponent(exponent);
		v.setsign(negative);
		return v;
	}

	// the exact product of a and b, most significant limb first, as efloat holds it: shifted to
	// its leading bit, trailing zero limbs dropped; returns the exponent of the leading bit
	int64_t reference_product(const std::vector<uint32_t>& a, int64_t ea, const std::vector<uint32_t>& b, int64_t eb, std::vector<uint32_t>& p) {
		const size_t m = a.size(), n = b.size();
		std::vector<uint32_t> r(m + n, 0u);   // least significant first
		for (size_t i = 0; i < m; ++i) {
			uint64_t carry = 0;
			for (size_t j = 0; j < n; ++j) {
				const uint64_t t = uint64_t(a[m - 1 - i]) * b[n - 1 - j] + r[i + j] + carry;
				r[i + j] = static_cast<uint32_t>(t);
				carry    = t >> 32;
			}
			r[i + n] = static_cast<uint32_t>(carry);
		}
		p.assign(r.rbegin(), r.rend());
		int64_t exponent = ea + eb + 1;
		if ((p[0] & 0x8000'0000u) == 0u) {   // a product of normalized significands has 0 or 1 leading zero bits
			for (size_t i = 0; i + 1 < p.size(); ++i) p[i] = (p[i] << 1) | (p[i + 1] >> 31);
			p.back() <<= 1;
			--exponent;
		}
		while (p.size() > 1 && p.back() == 0u) p.pop_back();
		return exponent;
	}

	bool matches(const Real& v, const std::vector<uint32_t>& p, int64_t exponent, bool negative) {
		return !v.iszero() && !v.isnan() && !v.isinf() && v.bits() == p && v.scale() == exponent && (v.sign() == -1) == negative;
	}

	int VerifyProduct(bool reportTestCases, std::mt19937_64& rng, size_t m, size_t n) {
		using namespace sw::universal;
		const auto     a  = random_limbs(rng, m);
		const auto     b  = random_limbs(rng, n);
		const int64_t  ea = static_cast<int64_t>(rng() % 201) - 100, eb = static_cast<int64_t>(rng() % 201) - 100;
		const bool     sa = (rng() & 1u) != 0u, sb = (rng() & 1u) != 0u;
		const unsigned bits = static_cast<unsigned>(64 * std::max(m, n));   // room for x * y and x * x
		const Real x = make(a, ea, sa, bits), y = make(b, eb, sb, bits);
		int failures = 0;

		std::vector<uint32_t> p;
		int64_t e = reference_product(a, ea, b, eb, p);
		clear_efloat_exceptions();
		const Real xy = x * y;
		const bool exact = !has_efloat_exception(ExceptionFlag::Inexact);
		if (!matches(xy, p, e, sa != sb) || !exact) {
			++failures;
			if (reportTestCases) std::cout << "    FAIL: " << m << " x " << n << " limb product differs from the schoolbook reference\n";
		}

		e = reference_product(a, ea, a, ea, p);
		Real s = x;
		s *= s;
		if (!matches(x * x, p, e, false) || !matches(s, p, e, false)) {
			++failures;
			if (reportTestCases) std::cout << "    FAIL: " << m << " limb square differs from the schoolbook reference\n";
		}
		return failures;
	}

	int VerifyExactProducts(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1762);
		// the Karatsuba thresholds, 24 and 32 64-bit limbs, and twice them, with odd neighbours
		const size_t lengths[] = { 1, 2, 3, 4, 7, 8, 9, 46, 47, 48, 49, 50, 63, 64, 65, 66, 95, 96, 97, 98, 128, 129, 255, 256 };
		for (size_t m : lengths) {
			nrOfFailedTestCases += VerifyProduct(reportTestCases, rng, m, m);
			nrOfFailedTestCases += VerifyProduct(reportTestCases, rng, m, 1 + rng() % m);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const size_t m = 1 + rng() % 300;
			const size_t n = (i % 3 == 0) ? m : 1 + rng() % 300;
			nrOfFailedTestCases += VerifyProduct(reportTestCases && nrOfFailedTestCases < 4, rng, m, n);
		}
		return nrOfFailedTestCases;
	}

	bool identical(const Real& a, const Real& b) {
		return a.sign() == b.sign() && a.scale() == b.scale() && a.bits() == b.bits()
		    && a.get_precision() == b.get_precision() && a.carries_precision() == b.carries_precision();
	}

	int VerifyAlignedSums(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(17621);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const unsigned precisions[] = { 1, 2, 5, 16 };   // in limbs
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned p : precisions) {
			for (unsigned i = 0; i < nrSamples; ++i) {
				const unsigned target = 32 * p;
				const size_t   nx     = 1 + rng() % p;
				size_t         ny     = 1 + rng() % p;
				const int64_t  ex     = static_cast<int64_t>(rng() % 41) - 20;
				int64_t        gap    = (i % 4 == 0) ? static_cast<int64_t>(rng() % 3) : static_cast<int64_t>(rng() % (32 * 40));
				const bool     sx     = (rng() & 1u) != 0u;
				const bool     sy     = (i % 3 == 0) ? !sx : ((rng() & 1u) != 0u);   // a third subtract
				Real x = make(random_limbs(rng, nx), ex, sx, target);
				Real y = make(random_limbs(rng, ny), ex - gap, sy, target);
				if (i % 5 == 0) {
					// a Y that carries no precision and is longer than the target, a gap of 0 to 2
					// bits, and x = 2^ex against y = 2^ex - 2^-k: the difference cancels nearly all
					// of y's bits
					const size_t     wide_ny = p + 1 + rng() % 4;
					std::vector<uint32_t> ones(wide_ny, 0xFFFF'FFFFu);
					if (rng() & 1u) ones.back() = static_cast<uint32_t>(rng()) | 1u;
					gap = static_cast<int64_t>(rng() % 3);
					x = make(std::vector<uint32_t>{ 0x8000'0000u }, ex, sx, target);
					y = make((rng() & 1u) ? ones : random_limbs(rng, wide_ny), ex - gap, !sx, 32 * static_cast<unsigned>(wide_ny));
					y.clear_precision();
					ny = wide_ny;
				}
				// the exact sum: at this precision neither operand loses a bit to the alignment
				const unsigned wide = static_cast<unsigned>(32 * (nx + ny + gap / 32 + 4));
				Real X = x, Y = y;
				X.set_precision(wide);
				Y.set_precision(wide);
				efloat_rounding_mode = modes[i % 4];
				Real sum = X + Y, difference = X - Y;
				sum.set_precision(target);          // rounds once, in the current mode
				difference.set_precision(target);
				const bool ok = identical(x + y, sum) && identical(y + x, sum) && identical(x - y, difference);
				efloat_rounding_mode = saved;
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: x + y with an exponent gap of " << gap << " bits at " << target << " bits is not the rounded exact sum\n";
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

	std::string test_suite          = "efloat multiplication and addition kernels";
	std::string test_tag            = "limb kernels";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 100), "efloat", "rounded sums");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000), "efloat", "rounded sums");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 3000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 20000), "efloat", "rounded sums");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 200000), "efloat", "rounded sums");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000000), "efloat", "rounded sums");
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
