// correct_rounding.cpp: efloat + - * / are correctly rounded at every precision (#1757)
//
// Each operation on random p-bit operands is checked against an exact oracle:
//   + - *  the exact result, formed at a precision wide enough to hold it, then rounded to p
//          by set_precision (a different code path: rounding of an exact value);
//   /      the residual a - q b, exact at a wide precision, which a correctly rounded
//          quotient keeps within half an ulp of q times |b|.
// Before #1757 a quotient whose dividend significand lay below the divisor's rounded one bit
// short, and a sum truncated the bits of the smaller addend that fell below the precision.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 200 random operand pairs per precision and operation
//   LEVEL 2 -- 2,000
//   LEVEL 3 -- 20,000
//   LEVEL 4 -- 200,000
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

	using Real = sw::universal::efloat<20>;
	constexpr unsigned wide_bits = 4096;   // holds any exact sum or product the tests form

	// a random value with a full p-bit significand and a scale in [-scale_range, scale_range]
	Real random_value(std::mt19937_64& rng, unsigned precision, int64_t scale_range) {
		const unsigned nr_limbs = (precision + 31) / 32;
		std::vector<uint32_t> limbs(nr_limbs);
		for (auto& l : limbs) l = static_cast<uint32_t>(rng());
		limbs[0] |= 0x80000000u;
		const int64_t scale = static_cast<int64_t>(rng() % static_cast<uint64_t>(2 * scale_range + 1)) - scale_range;
		Real v(static_cast<bool>(rng() & 1u), scale, limbs);
		v.set_precision(precision);
		return v;
	}

	Real widened(const Real& v) {
		Real w(v);
		w.set_precision(wide_bits);
		return w;
	}

	Real rounded(Real exact, unsigned precision) {
		exact.set_precision(precision);
		return exact;
	}

	// q is the correctly rounded a / b at q's precision: |a - q b| <= |b| ulp(q) / 2
	bool is_correctly_rounded_quotient(const Real& a, const Real& b, const Real& q) {
		Real residual = widened(a) - widened(q) * widened(b);
		residual.setsign(false);
		Real half_ulp_b = widened(b);
		half_ulp_b.setsign(false);
		// ulp(q) = 2^(scale(q) - (precision - 1)); |b| ulp(q) / 2 = |b| 2^(scale(q) - precision)
		half_ulp_b.setexponent(b.scale() + q.scale() - static_cast<int64_t>(q.get_precision()));
		return residual <= half_ulp_b;
	}

	int VerifyCorrectRounding(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1757);
		const unsigned precisions[] = { 32, 64, 96, 200, 320, 640 };
		for (unsigned p : precisions) {
			int failures[4] = { 0, 0, 0, 0 };
			for (unsigned i = 0; i < nrSamples; ++i) {
				// the second operand lies up to ~3p bits below or above the first, so the
				// alignment shifts of + and - fall inside, across and past the precision
				const Real a = random_value(rng, p, 8);
				Real b = random_value(rng, p, 8);
				b.setexponent(b.scale() + static_cast<int64_t>(rng() % (6 * p + 1)) - 3 * static_cast<int64_t>(p));

				if (a + b != rounded(widened(a) + widened(b), p)) ++failures[0];
				if (a - b != rounded(widened(a) - widened(b), p)) ++failures[1];
				if (a * b != rounded(widened(a) * widened(b), p)) ++failures[2];
				if (!is_correctly_rounded_quotient(a, b, a / b)) ++failures[3];
			}
			const char* ops[] = { "+", "-", "*", "/" };
			for (int op = 0; op < 4; ++op) {
				if (failures[op] > 0) {
					nrOfFailedTestCases += failures[op];
					if (reportTestCases) std::cout << "    FAIL: " << failures[op] << " of " << nrSamples << " results of " << ops[op] << " at " << p << " bits are not correctly rounded\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// a quotient whose dividend significand lies below the divisor's: 1/3, 2/3, 5/7
	int VerifyQuotientBelowDivisor(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const unsigned precisions[] = { 32, 64, 160, 640 };
		const int pairs[][2] = { { 1, 3 }, { 2, 3 }, { 5, 7 }, { 1, 7 }, { 4, 5 }, { 11, 13 } };
		for (unsigned p : precisions) {
			for (const auto& pr : pairs) {
				Real a(pr[0]);
				a.set_precision(p);
				const Real q = a / pr[1];
				if (q.get_precision() != p || !is_correctly_rounded_quotient(a, Real(pr[1]), q)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cout << "    FAIL: " << pr[0] << "/" << pr[1] << " at " << p << " bits is not correctly rounded\n";
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

	std::string test_suite          = "efloat correctly rounded arithmetic";
	std::string test_tag            = "correct rounding";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyQuotientBelowDivisor(reportTestCases), "efloat", "quotient below divisor");
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectRounding(reportTestCases, 100), "efloat", "+ - * / random");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientBelowDivisor(reportTestCases), "efloat", "quotient below divisor");
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectRounding(reportTestCases, 200), "efloat", "+ - * / random");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectRounding(reportTestCases, 2000), "efloat", "+ - * / random");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectRounding(reportTestCases, 20000), "efloat", "+ - * / random");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectRounding(reportTestCases, 200000), "efloat", "+ - * / random");
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
