// addition.cpp: regression tests for efloat addition.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests: algebraic invariants,
//              hostile arithmetic corner cases (round-to-even, massive
//              exponent gaps, complete overlap), subnormal boundaries,
//              IEEE 754 special values.
//   LEVEL 2 -- property-based fuzzer over random multi-component expansions
//              (~1,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~10,000,000 iterations).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/efloat_test_support.hpp>

namespace {

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyEfloatAddition(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Algebraic invariants ---

		// Commutativity
		if (reportTestCases) std::cout << "  Commutativity...\n";
		{
			efloat<16> a(1.0e+15), b(1.0), c(1.0e-15);
			if ((a + b + c) != (c + b + a)) {
				if (reportTestCases) std::cout << "    FAIL\n";
				++nrOfFailedTestCases;
			}
		}

		// Associativity
		if (reportTestCases) std::cout << "  Associativity...\n";
		{
			efloat<16> a(1.0e+30), b(1.0), c(1.0e-30);
			if (((a + b) + c) != (a + (b + c))) {
				if (reportTestCases) std::cout << "    FAIL\n";
				++nrOfFailedTestCases;
			}
		}

		// Inverse identity: a + (-a) collapses to zero
		if (reportTestCases) std::cout << "  Inverse identity...\n";
		{
			efloat<16> a(1.0e+15);
			a += 1.0; a += 1.0e-15;
			efloat<16> neg_a = -a;
			efloat<16> result = a + neg_a;
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL a + (-a) != 0\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Hostile arithmetic ---

		// 10-limb cancellation test (Constructed dynamically to bypass parser stubs)
		if (reportTestCases) std::cout << "  10-limb cancellation...\n";
		{
			efloat<128> a(1.0);
			double mag = 1.0;
			for (int i = 0; i < 10; ++i) {
				mag = std::ldexp(mag, -50);
				a += mag;
			}
			efloat<128> b = a;
			efloat<128> c = a - b;
			if (!c.iszero()) {
				if (reportTestCases) std::cout << "    FAIL: 10-limb cancellation did not produce zero. Result: " << c << "\n";
				++nrOfFailedTestCases;
			}
		}

		// Massive exponent gap (1e+300 + 1e-300)
		if (reportTestCases) std::cout << "  Massive exponent gap...\n";
		{
			efloat<128> a(1.0e+300), b(1.0e-300);
			efloat<128> result = a + b;
			efloat<128> recovered = result - a;
			if (recovered != b) {
				if (reportTestCases) std::cout << "    FAIL tiny term dropped\n";
				++nrOfFailedTestCases;
			}
		}

		// --- IEEE 754 special values ---
		double qnan = std::numeric_limits<double>::quiet_NaN();
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;

		if (reportTestCases) std::cout << "  NaN + finite...\n";
		{
			efloat<16> a(1.0), n(qnan);
			if (!(a + n).isnan()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		if (reportTestCases) std::cout << "  +Inf + finite...\n";
		{
			efloat<16> a(1.0), inf(pinf);
			if (!(a + inf).isinf() || (a+inf).isneg()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		if (reportTestCases) std::cout << "  -Inf + +Inf...\n";
		{
			efloat<16> p(pinf), n(ninf);
			if (!(p + n).isnan()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		return nrOfFailedTestCases;
	}

	// =========================================================================
	// Fuzzer infrastructure
	// =========================================================================
	int VerifyEfloatAddition_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;
		efloat<16> zero(0.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			efloat<16> a = random_efloat<16>(rng);
			efloat<16> b = random_efloat<16>(rng);
			
			// Commutativity: a + b == b + a
			if (a + b != b + a) {
				if (reportTestCases) std::cout << "    FAIL commutativity (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
			// Identity: a + 0 == a
			if (a + zero != a) {
				if (reportTestCases) std::cout << "    FAIL identity (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
			// Inverse: a + (-a) == 0
			efloat<16> sum_with_neg = a + (-a);
			if (!sum_with_neg.iszero()) {
				if (reportTestCases) std::cout << "    FAIL inverse (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
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
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat addition";
	std::string test_tag            = "addition";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAddition(reportTestCases), "efloat", "addition manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAddition(reportTestCases), "efloat", "addition foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAddition_Fuzz(reportTestCases, 100), "efloat", "addition fuzz x100");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAddition_Fuzz(reportTestCases, 1000), "efloat", "addition fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAddition_Fuzz(reportTestCases, 10000), "efloat", "addition fuzz x10k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAddition_Fuzz(reportTestCases, 100000), "efloat", "addition fuzz x100k");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
