// precision_lifting.cpp: self-referential precision-lifting oracle (#954, D2).
//
// First-principles verification with no external oracle: the widest
// configuration ereal<19> (1007 bits) is the value-canonical ground truth for
// any narrower one. For random narrow inputs lifted value-preservingly to
// ereal<19>:
//   - The ERROR-FREE operations (+, -, *) are maxlimbs-independent: the narrow
//     result (lifted) EQUALS the wide computation bit-for-bit, because nothing
//     is dropped (maxlimbs is the algorithmic ceiling on inputs, not a runtime
//     truncation).
//   - DIVISION is the exception: it is an iterative Newton-reciprocal whose
//     precision intentionally scales with maxlimbs (iterations ==
//     ceil(log2(maxlimbs))+1, see #1005/#1004), so the narrow and wide quotients
//     are NOT bit-identical. The narrow quotient is correct to ~53*NARROW bits
//     and the wide quotient to ~53*WIDE bits, so they must agree to the narrow
//     configuration's precision -- which is exactly what an adaptive type should
//     do (higher maxlimbs -> more correct digits). Verified with a relative
//     tolerance at NARROW precision rather than bit-equality.
//
// REGRESSION_LEVEL convention:
//   LEVEL 1 -- config pairs (2,19) (4,19) (8,19) (16,19), fuzz x100 each.
//   LEVEL 2 -- fuzz x1k.   LEVEL 3 -- x10k.   LEVEL 4 -- x100k.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <random>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/ereal_test_support.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	template <unsigned maxlimbs>
	ereal<maxlimbs> random_ereal(std::mt19937_64& rng, double leading_magnitude = 1e6) {
		std::uniform_int_distribution<unsigned> n_dist(1, maxlimbs);
		std::uniform_int_distribution<int> sign_dist(0, 1);
		std::uniform_real_distribution<double> unit(0.5, 2.0);
		ereal<maxlimbs> result(0.0);
		unsigned n_limbs = n_dist(rng);
		double mag = leading_magnitude;
		for (unsigned i = 0; i < n_limbs; ++i) {
			result += unit(rng) * mag * (sign_dist(rng) ? 1.0 : -1.0);
			mag = std::ldexp(mag, -50);
			if (mag < std::ldexp(1.0, -950)) break;
		}
		return result;
	}

	constexpr unsigned WIDE = 19;  // ground-truth configuration (1007 bits)

	// Precision-lifting fuzz for one narrow configuration NARROW against WIDE.
	template <unsigned NARROW>
	int VerifyPrecisionLifting(bool reportTestCases, unsigned nrIterations) {
		const uint64_t seed = 0xC0FFEE'A11CEULL + NARROW;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		// Error-free ops: narrow result lifted to WIDE must equal the wide
		// computation bit-for-bit (maxlimbs-independent).
		auto agree = [&](const char* op, const ereal<NARROW>& rn, const ereal<WIDE>& rw, unsigned i) {
			if (widen<WIDE>(rn) != rw) {
				if (reportTestCases) std::cout << "    FAIL " << op << " ereal<" << NARROW
					<< "> vs ereal<19> (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
		};

		// Division (iterative): the narrow quotient is correct only to its own
		// ~53*NARROW bits, so it must agree with the wide quotient to NARROW
		// precision, not bit-for-bit. Tolerance = one limb of slack below the
		// narrow precision (relative).
		const double div_rel_tol = std::ldexp(1.0, -53 * (static_cast<int>(NARROW) - 1));
		auto agree_div = [&](const ereal<NARROW>& rn, const ereal<WIDE>& rw, unsigned i) {
			ereal<WIDE> lifted = widen<WIDE>(rn);
			bool ok;
			if (rw.iszero()) {
				ok = lifted.iszero();
			} else {
				double rel = std::abs(double(abs(lifted - rw) / abs(rw)));
				ok = (rel <= div_rel_tol);
			}
			if (!ok) {
				if (reportTestCases) std::cout << "    FAIL a/b ereal<" << NARROW
					<< "> vs ereal<19> beyond narrow precision (seed=0x" << std::hex << seed
					<< " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
		};

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<NARROW> a = random_ereal<NARROW>(rng);
			ereal<NARROW> b = random_ereal<NARROW>(rng);
			ereal<WIDE> aw = widen<WIDE>(a);
			ereal<WIDE> bw = widen<WIDE>(b);

			agree("a+b", a + b, aw + bw, i);
			agree("a-b", a - b, aw - bw, i);
			agree("a*b", a * b, aw * bw, i);
			if (!b.iszero()) agree_div(a / b, aw / bw, i);
		}
		return nrOfFailedTestCases;
	}

	int RunAllConfigs(bool reportTestCases, unsigned nrIterations) {
		int fails = 0;
		fails += VerifyPrecisionLifting<2>(reportTestCases, nrIterations);
		fails += VerifyPrecisionLifting<4>(reportTestCases, nrIterations);
		fails += VerifyPrecisionLifting<8>(reportTestCases, nrIterations);
		fails += VerifyPrecisionLifting<16>(reportTestCases, nrIterations);
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
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "ereal precision-lifting oracle (ereal<19> ground truth)";
	std::string test_tag            = "precision lifting";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 100), "ereal", "precision lifting manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 100), "ereal", "precision lifting x100");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 1000), "ereal", "precision lifting x1k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 10000), "ereal", "precision lifting x10k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(RunAllConfigs(reportTestCases, 100000), "ereal", "precision lifting x100k");
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
