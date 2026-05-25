// exact_value_oracle.cpp: independent exact-value oracle for ereal (#987 Layer 2)
//
// This is the keystone of the ereal/Priest conformance effort. The structural
// oracle (#954) proves results are in Priest normal form (shape); the
// precision-lifting oracle compares ereal<N> against ereal<19> (self-consistency).
// Neither catches a normalized expansion that encodes the WRONG value. This test
// does: it compares every ereal result bit-for-bit against an INDEPENDENT exact
// reference that shares no code with the expansion algorithms.
//
// The reference is exact dyadic-rational arithmetic (dyadic_exact.hpp, backed by
// einteger). The exact value of an expansion is the sum of its limbs (they are
// non-overlapping), each an exactly representable dyadic. ereal's +, -, * are
// error-free (Shewchuk/Priest), so they must reproduce the exact mathematical
// result -- verified as bit-exact dyadic equality:
//
//   exact_value(a + b) == exact_value(a) + exact_value(b)   (and -, *)
//
// Division is not exact (a quotient is generally not a finite dyadic, and ereal
// division rounds), so it is checked with a relative-error bound rather than
// exact equality.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <random>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// Exact value of an ereal expansion: the (exact) sum of its non-overlapping
	// limbs, as a dyadic rational. Shares no code with the expansion algorithms.
	template <unsigned maxlimbs>
	dyadic exact_value(const ereal<maxlimbs>& v) {
		dyadic acc;  // 0
		for (double limb : v.limbs()) acc = acc + dyadic::from_double(limb);
		return acc;
	}

	// Build a multi-component expansion of random length / signs / scales,
	// CENTERED around 2^0 so that the whole expansion lives in the exponent
	// band (-511, 511). Every limb then has |exponent| < 511, so every partial
	// product limb_i*limb_j has |exponent| < 1022 and stays a normal double --
	// which is the precondition for the product to be EXACTLY representable as a
	// double-limb expansion (two_prod's error term underflows otherwise). This
	// is a representability constraint, not a workaround: an expansion spanning
	// more than ~1022 binary exponents simply cannot have an exact double
	// product, regardless of the algorithm (the low bits fall below
	// DBL_TRUE_MIN). The exact-product identity is verified within this band;
	// behaviour outside it (graceful precision loss) is a separate concern.
	template <unsigned maxlimbs>
	ereal<maxlimbs> random_ereal(std::mt19937_64& rng) {
		std::uniform_int_distribution<unsigned> n_dist(1, maxlimbs);
		std::uniform_int_distribution<int> sign_dist(0, 1);
		std::uniform_real_distribution<double> unit(0.5, 2.0);
		ereal<maxlimbs> result(0.0);
		unsigned n_limbs = n_dist(rng);
		constexpr int drop = 55;  // > 53: keeps limbs non-overlapping (distinct)
		// center the full-length band on 2^0: top limb at 2^(+span/2)
		int exp = (drop * static_cast<int>(maxlimbs - 1)) / 2;
		for (unsigned i = 0; i < n_limbs; ++i) {
			result += unit(rng) * std::ldexp(1.0, exp) * (sign_dist(rng) ? 1.0 : -1.0);
			exp -= drop;
		}
		return result;
	}

	// Considers x and y close if  |x - y| <= absTol  OR  |x - y| <= relTol * max(|y|, absTol)
	// (an absolute floor OR a relative tolerance scaled by max(|y|, absTol)).
	// Used only for the (inexact) division check, since a quotient need not be dyadic.
	bool close_rel(double x, double y, double relTol, double absTol = 1e-300) {
		double diff = std::abs(x - y);
		double scale = std::max(std::abs(y), absTol);
		return diff <= std::max(absTol, relTol * scale);
	}

	template <unsigned maxlimbs>
	int VerifyExactValue(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0x0EAC70ACEULL + maxlimbs);
		int nrOfFailedTestCases = 0;

		auto exact_ok = [&](const char* op, const dyadic& got, const dyadic& want, unsigned i) {
			if (got != want) {
				if (reportTestCases) std::cout << "    FAIL exact " << op << " ereal<" << maxlimbs
					<< "> (iter " << i << ")\n";
				++nrOfFailedTestCases;
			}
		};

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<maxlimbs> a = random_ereal<maxlimbs>(rng);
			ereal<maxlimbs> b = random_ereal<maxlimbs>(rng);
			dyadic da = exact_value(a), db = exact_value(b);

			// the keystone: error-free operations reproduce the exact value
			exact_ok("a+b", exact_value(a + b), da + db, i);
			exact_ok("a-b", exact_value(a - b), da - db, i);
			exact_ok("a*b", exact_value(a * b), da * db, i);

			// division: not exact -> relative-error bound on the projected value
			if (!b.iszero()) {
				double dbl_b = double(b);
				if (dbl_b != 0.0) {
					double q  = double(a / b);
					double ref = double(a) / dbl_b;
					if (!close_rel(q, ref, 1.0e-12)) {
						if (reportTestCases) std::cout << "    FAIL div bound ereal<" << maxlimbs
							<< "> (iter " << i << ") q=" << q << " ref=" << ref << "\n";
						++nrOfFailedTestCases;
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	int RunAllConfigs(bool reportTestCases, unsigned nrIterations) {
		int fails = 0;
		fails += VerifyExactValue<2>(reportTestCases, nrIterations);
		fails += VerifyExactValue<4>(reportTestCases, nrIterations);
		fails += VerifyExactValue<8>(reportTestCases, nrIterations);
		fails += VerifyExactValue<16>(reportTestCases, nrIterations);
		return fails;
	}

}  // anonymous namespace

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "ereal exact-value oracle (independent dyadic reference)";
	std::string test_tag            = "exact value";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 1000), "ereal", "exact value manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 1000), "ereal", "exact value x1k");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 10000), "ereal", "exact value x10k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 100000), "ereal", "exact value x100k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 1000000), "ereal", "exact value x1M");
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
