// subtraction.cpp: regression tests for expansion (multi-component) subtraction
//
// Subtraction is a - b == a + (-b). Verifies the algebraic invariants (exact
// cancellation, zero identity, negation, inverse of addition, sign change,
// extreme scales) and the precision-preservation property that motivates
// expansions: (a + eps) - a recovers eps exactly even when a single double
// would round eps away. Value comparisons use the double projection within a
// relative tolerance; bit-exact value correctness of ereal -/+ is proven in
// elastic/ereal/arithmetic/exact_value_oracle.cpp (#989).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <random>
#include <vector>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;
	using namespace sw::universal::expansion_ops;

	double value(const std::vector<double>& e) { double s = 0.0; for (double v : e) s += v; return s; }
	std::vector<double> negate(std::vector<double> e) { for (auto& v : e) v = -v; return e; }
	std::vector<double> sub(const std::vector<double>& a, const std::vector<double>& b) {
		return renormalize_expansion(linear_expansion_sum(a, negate(b)));
	}
	bool close_rel(double x, double y, double relTol) {
		double scale = std::max({ std::abs(x), std::abs(y), 1.0 });
		return std::abs(x - y) <= relTol * scale;
	}
	std::vector<double> random_expansion(std::mt19937_64& rng, unsigned maxComponents = 4) {
		std::uniform_int_distribution<unsigned> n_dist(1, maxComponents);
		std::uniform_real_distribution<double> unit(-2.0, 2.0);
		std::vector<double> e{ 0.0 };
		double mag = std::ldexp(1.0, std::uniform_int_distribution<int>(-4, 20)(rng));
		unsigned n = n_dist(rng);
		for (unsigned i = 0; i < n; ++i) { e = renormalize_expansion(grow_expansion(e, unit(rng) * mag)); mag = std::ldexp(mag, -55); }
		return e;
	}

	// a - a == 0 (exact)
	int VerifyExactCancellation(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 10.0, 1.0e-15 };
		if (std::abs(value(sub(a, a))) != 0.0) {
			if (reportTestCases) std::cout << "    FAIL a - a != 0\n";
			++fails;
		}
		return fails;
	}

	// a - 0 == a, 0 - a == -a
	int VerifyZeroAndNegation(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 7.0, 3.5e-16 };
		std::vector<double> zero = { 0.0 };
		if (!close_rel(value(sub(a, zero)), value(a), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL a - 0 != a\n";
			++fails;
		}
		if (!close_rel(value(sub(zero, a)), -value(a), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL 0 - a != -a\n";
			++fails;
		}
		return fails;
	}

	// (a + b) - b == a
	int VerifyInverseOfAddition(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 100.0, 1.0e-13 };
		std::vector<double> b = { 3.0, 2.0e-16 };
		std::vector<double> back = sub(renormalize_expansion(linear_expansion_sum(a, b)), b);
		if (!close_rel(value(back), value(a), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL (a+b) - b != a\n";
			++fails;
		}
		return fails;
	}

	// precision preservation: (a + eps) - a == eps, where eps is below ulp(a)
	// so a single double would lose it entirely.
	int VerifyNearCancellation(bool reportTestCases) {
		int fails = 0;
		double a0 = 1.0;
		double eps = std::ldexp(1.0, -60);          // far below ulp(1.0)=2^-52
		std::vector<double> a = { a0 };
		std::vector<double> a_plus = renormalize_expansion(grow_expansion(a, eps));  // exact a + eps
		std::vector<double> recovered = sub(a_plus, a);
		if (!close_rel(value(recovered), eps, 1.0e-12)) {
			if (reportTestCases) std::cout << "    FAIL (a+eps) - a != eps  (got " << value(recovered) << ")\n";
			++fails;
		}
		return fails;
	}

	// a - b with b > a yields a negative result of the right magnitude
	int VerifySignChange(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 3.0, 1.0e-16 };
		std::vector<double> b = { 10.0, 5.0e-16 };
		double got = value(sub(a, b));
		if (!close_rel(got, value(a) - value(b), 1.0e-14) || got >= 0.0) {
			if (reportTestCases) std::cout << "    FAIL sign change a - b (b>a)\n";
			++fails;
		}
		return fails;
	}

	// extreme scale difference: large - tiny keeps the tiny component
	int VerifyExtremeScales(bool reportTestCases) {
		int fails = 0;
		std::vector<double> big = { 1.0e16 };
		std::vector<double> tiny = { 1.0 };
		std::vector<double> d = sub(big, tiny);     // 1e16 - 1, exactly representable in 2 limbs
		if (!close_rel(value(d), 1.0e16 - 1.0, 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL extreme-scale subtraction\n";
			++fails;
		}
		return fails;
	}

	// fuzz: (a + b) - b == a and a - a == 0 over random expansions
	int VerifySubtraction_Fuzz(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0x5B7B5EEDULL);
		int fails = 0;
		for (unsigned i = 0; i < nrIterations; ++i) {
			std::vector<double> a = random_expansion(rng), b = random_expansion(rng);
			std::vector<double> back = sub(renormalize_expansion(linear_expansion_sum(a, b)), b);
			if (!close_rel(value(back), value(a), 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL fuzz (a+b)-b != a (iter " << i << ")\n";
				++fails;
			}
			if (std::abs(value(sub(a, a))) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL fuzz a-a != 0 (iter " << i << ")\n";
				++fails;
			}
		}
		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "expansion subtraction";
	std::string test_tag    = "subtraction";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyExactCancellation(reportTestCases), "expansion", "exact cancellation a-a=0");
	nrOfFailedTestCases += ReportTestResult(VerifyZeroAndNegation(reportTestCases),   "expansion", "zero identity / negation");
	nrOfFailedTestCases += ReportTestResult(VerifyInverseOfAddition(reportTestCases), "expansion", "(a+b)-b=a");
	nrOfFailedTestCases += ReportTestResult(VerifyNearCancellation(reportTestCases),  "expansion", "near cancellation (a+eps)-a=eps");
	nrOfFailedTestCases += ReportTestResult(VerifySignChange(reportTestCases),        "expansion", "sign change");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremeScales(reportTestCases),     "expansion", "extreme scales");

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction_Fuzz(reportTestCases, 1000), "expansion", "subtraction fuzz");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction_Fuzz(reportTestCases, 1000),   "expansion", "subtraction fuzz x1k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction_Fuzz(reportTestCases, 10000),  "expansion", "subtraction fuzz x10k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction_Fuzz(reportTestCases, 100000), "expansion", "subtraction fuzz x100k");
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
