// addition.cpp: regression tests for expansion (multi-component) addition
//
// Verifies the algebraic invariants of expansion addition (identity,
// commutativity, associativity, zero, cancellation). Value comparisons use the
// double projection (sum of the non-overlapping limbs) within a relative
// tolerance, since the projection -- not the limb structure -- is the invariant
// under test here. (Bit-exact value correctness of ereal addition is proven
// separately in elastic/ereal/arithmetic/exact_value_oracle.cpp, #989.)
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

	double value(const std::vector<double>& e) {
		double s = 0.0;
		for (double v : e) s += v;
		return s;
	}
	bool close_rel(double x, double y, double relTol) {
		double scale = std::max({ std::abs(x), std::abs(y), 1.0 });
		return std::abs(x - y) <= relTol * scale;
	}

	// random multi-component expansion built by repeated grow (renormalized)
	std::vector<double> random_expansion(std::mt19937_64& rng, unsigned maxComponents = 4) {
		std::uniform_int_distribution<unsigned> n_dist(1, maxComponents);
		std::uniform_real_distribution<double> unit(-2.0, 2.0);
		std::vector<double> e{ 0.0 };
		double mag = std::ldexp(1.0, std::uniform_int_distribution<int>(-4, 20)(rng));
		unsigned n = n_dist(rng);
		for (unsigned i = 0; i < n; ++i) {
			e = renormalize_expansion(grow_expansion(e, unit(rng) * mag));
			mag = std::ldexp(mag, -55);
		}
		return e;
	}

	// (a + b) - a == b
	int VerifyIdentity(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 10.0, 1.0e-15 };
		std::vector<double> b = { 5.0, 5.0e-16 };
		std::vector<double> sum = linear_expansion_sum(a, b);
		std::vector<double> neg_a = a; for (auto& v : neg_a) v = -v;
		std::vector<double> recovered = compress_expansion(linear_expansion_sum(sum, neg_a), 0.0);
		if (!close_rel(value(recovered), value(b), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL identity (a+b)-a != b\n";
			++fails;
		}
		return fails;
	}

	// a + b == b + a
	int VerifyCommutative(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 7.0, 3.5e-16 };
		std::vector<double> b = { 3.0, 1.5e-16 };
		if (!close_rel(value(linear_expansion_sum(a, b)), value(linear_expansion_sum(b, a)), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL commutativity a+b != b+a\n";
			++fails;
		}
		return fails;
	}

	// (a + b) + c ~= a + (b + c)
	int VerifyAssociative(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 10.0, 1.0e-15 }, b = { 5.0, 5.0e-16 }, c = { 2.0, 2.0e-16 };
		double left  = value(linear_expansion_sum(linear_expansion_sum(a, b), c));
		double right = value(linear_expansion_sum(a, linear_expansion_sum(b, c)));
		if (!close_rel(left, right, 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL associativity (a+b)+c != a+(b+c)\n";
			++fails;
		}
		return fails;
	}

	// a + 0 == a  and  0 + b == b
	int VerifyZero(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 10.0, 1.0e-15 };
		std::vector<double> zero = { 0.0 };
		if (!close_rel(value(linear_expansion_sum(a, zero)), value(a), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL a+0 != a\n";
			++fails;
		}
		if (!close_rel(value(linear_expansion_sum(zero, a)), value(a), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL 0+a != a\n";
			++fails;
		}
		return fails;
	}

	// a + (-a) == 0; partial cancellation lands on the expected value
	int VerifyCancellation(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 10.0, 1.0e-15 };
		std::vector<double> neg_a = { -10.0, -1.0e-15 };
		if (std::abs(value(compress_expansion(linear_expansion_sum(a, neg_a), 0.0))) > 1.0e-14) {
			if (reportTestCases) std::cout << "    FAIL a+(-a) != 0\n";
			++fails;
		}
		std::vector<double> b = { -9.0, -0.9e-15 };
		double got = value(compress_expansion(linear_expansion_sum(a, b), 0.0));
		if (!close_rel(got, 1.0 + 0.1e-15, 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL partial cancellation\n";
			++fails;
		}
		return fails;
	}

	// fuzz: commutativity, identity, and zero over random expansions
	int VerifyAddition_Fuzz(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0xADD'5EEDULL & 0xFFFFFFFF);
		int fails = 0;
		std::vector<double> zero{ 0.0 };
		for (unsigned i = 0; i < nrIterations; ++i) {
			std::vector<double> a = random_expansion(rng), b = random_expansion(rng);
			if (!close_rel(value(linear_expansion_sum(a, b)), value(linear_expansion_sum(b, a)), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL fuzz commutativity (iter " << i << ")\n";
			++fails;
			}
			if (!close_rel(value(linear_expansion_sum(a, zero)), value(a), 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL fuzz a+0 (iter " << i << ")\n";
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

	std::string test_suite  = "expansion addition";
	std::string test_tag    = "addition";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyIdentity(reportTestCases),     "expansion", "identity (a+b)-a==b");
	nrOfFailedTestCases += ReportTestResult(VerifyCommutative(reportTestCases),  "expansion", "commutativity");
	nrOfFailedTestCases += ReportTestResult(VerifyAssociative(reportTestCases),  "expansion", "associativity");
	nrOfFailedTestCases += ReportTestResult(VerifyZero(reportTestCases),         "expansion", "additive identity 0");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellation(reportTestCases), "expansion", "cancellation");

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyAddition_Fuzz(reportTestCases, 1000), "expansion", "addition fuzz");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAddition_Fuzz(reportTestCases, 1000),   "expansion", "addition fuzz x1k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAddition_Fuzz(reportTestCases, 10000),  "expansion", "addition fuzz x10k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAddition_Fuzz(reportTestCases, 100000), "expansion", "addition fuzz x100k");
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
