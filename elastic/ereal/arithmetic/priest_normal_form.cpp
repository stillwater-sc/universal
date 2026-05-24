// priest_normal_form.cpp: structural-oracle tests for ereal (issue #954, D1).
//
// Verifies the assert_priest_normal helper itself on crafted limb vectors, and
// then asserts that every result of +, -, *, / over random multi-component
// expansions is in Priest normal form (decreasing magnitude, non-overlapping,
// no interior zeros). This is a first-principles check -- no external oracle.
//
// (Multiplication/division producing canonical, non-overlapping output depends
// on #981, which renormalizes expansion_product.)
//
// REGRESSION_LEVEL convention:
//   LEVEL 1 -- helper unit tests + structural fuzz x1k.
//   LEVEL 2 -- structural fuzz x10k.   LEVEL 3 -- x100k.   LEVEL 4 -- x1M.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <random>
#include <vector>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/ereal_test_support.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;
	using Violation = PriestNormalResult::Violation;

	// =========================================================================
	// LEVEL 1a: unit-test the structural oracle on crafted limb vectors
	// =========================================================================
	int VerifyPriestOracle(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		auto expect = [&](const char* name, const std::vector<double>& L, Violation want) {
			Violation got = check_priest_normal(L).violation;
			if (got != want) {
				if (reportTestCases) std::cout << "    FAIL " << name << ": got violation "
					<< static_cast<int>(got) << " want " << static_cast<int>(want) << '\n';
				++nrOfFailedTestCases;
			}
		};

		const double e60 = std::ldexp(1.0, -60);  // well below ulp(1.0)/2 = 2^-53
		// normal expansions
		expect("normal 2-limb",     {1.0, e60},          Violation::None);
		expect("normal single",     {3.14},              Violation::None);
		expect("canonical zero",    {0.0},               Violation::None);
		// non-decreasing magnitude
		expect("non-decreasing",    {1.0, 2.0},          Violation::NonDecreasing);
		// overlap: 0.5 > ulp(1.0)/2 (= 2^-53)
		expect("overlap",           {1.0, 0.5},          Violation::Overlap);
		// interior zero in a non-zero expansion
		expect("interior zero",     {1.0, 0.0},          Violation::InteriorZero);

		return nrOfFailedTestCases;
	}

	// Multi-component generator (same shape as the arithmetic fuzzers).
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

	// =========================================================================
	// LEVEL 1b+: every operator result must be Priest-normal
	// =========================================================================
	int VerifyResultsArePriestNormal(bool reportTestCases, unsigned nrIterations) {
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;

		auto must_be_normal = [&](const char* op, const ereal<16>& r, unsigned i) {
			PriestNormalResult pr = check_priest_normal(r);
			if (!pr.ok()) {
				if (reportTestCases) std::cout << "    FAIL " << op << " result not priest-normal ("
					<< pr.what() << " at limb " << pr.index << ", seed=0x" << std::hex << seed
					<< " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
		};

		for (unsigned i = 0; i < nrIterations; ++i) {
			ereal<16> a = random_ereal<16>(rng);
			ereal<16> b = random_ereal<16>(rng);
			must_be_normal("a+b", a + b, i);
			must_be_normal("a-b", a - b, i);
			must_be_normal("a*b", a * b, i);
			if (!b.iszero()) must_be_normal("a/b", a / b, i);
		}
		return nrOfFailedTestCases;
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

	std::string test_suite          = "ereal Priest normal form (structural oracle)";
	std::string test_tag            = "priest normal form";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyPriestOracle(reportTestCases), "ereal", "priest oracle manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPriestOracle(reportTestCases), "ereal", "priest oracle helper");
	nrOfFailedTestCases += ReportTestResult(VerifyResultsArePriestNormal(reportTestCases, 1000), "ereal", "results priest-normal x1k");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyResultsArePriestNormal(reportTestCases, 10000), "ereal", "results priest-normal x10k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyResultsArePriestNormal(reportTestCases, 100000), "ereal", "results priest-normal x100k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyResultsArePriestNormal(reportTestCases, 1000000), "ereal", "results priest-normal x1M");
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
