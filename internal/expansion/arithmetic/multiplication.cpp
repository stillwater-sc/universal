// multiplication.cpp: regression tests for expansion (multi-component) multiplication
//
// Covers scale_expansion (expansion * scalar) and expansion_product (expansion *
// expansion): scalar correctness, multiplicative identity, zero, negation,
// distributivity, commutativity, associativity, and precision preservation.
// Value comparisons use the double projection within a relative tolerance;
// bit-exact value correctness of ereal * is proven in
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
	bool close_rel(double x, double y, double relTol) {
		double scale = std::max({ std::abs(x), std::abs(y), 1.0 });
		return std::abs(x - y) <= relTol * scale;
	}
	std::vector<double> random_expansion(std::mt19937_64& rng, unsigned maxComponents = 3) {
		std::uniform_int_distribution<unsigned> n_dist(1, maxComponents);
		std::uniform_real_distribution<double> unit(-2.0, 2.0);
		std::vector<double> e{ 0.0 };
		double mag = std::ldexp(1.0, std::uniform_int_distribution<int>(-2, 8)(rng));
		unsigned n = n_dist(rng);
		for (unsigned i = 0; i < n; ++i) { e = renormalize_expansion(grow_expansion(e, unit(rng) * mag)); mag = std::ldexp(mag, -55); }
		return e;
	}

	// scale_expansion(e, b) value == value(e) * b
	int VerifyScalarCorrectness(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 100.0, 1.0e-13 };
		for (double b : { 3.0, -2.5, 0.25, 1.0e6 }) {
			if (!close_rel(value(scale_expansion(e, b)), value(e) * b, 1.0e-14)) {
				if (reportTestCases) std::cout << "    FAIL scale by " << b << "\n";
				++fails;
			}
		}
		return fails;
	}

	// e * 1 == e, e x [1] == e
	int VerifyIdentity(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 10.0, 1.0e-15 };
		if (!close_rel(value(scale_expansion(e, 1.0)), value(e), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL e * 1.0 != e\n";
			++fails;
		}
		if (!close_rel(value(expansion_product(e, { 1.0 })), value(e), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL e x [1] != e\n";
			++fails;
		}
		return fails;
	}

	// e * 0 == 0, e x [0] == [0]
	int VerifyZero(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 10.0, 1.0e-15 };
		if (value(scale_expansion(e, 0.0)) != 0.0) {
			if (reportTestCases) std::cout << "    FAIL e * 0 != 0\n";
			++fails;
		}
		if (value(expansion_product(e, { 0.0 })) != 0.0) {
			if (reportTestCases) std::cout << "    FAIL e x [0] != [0]\n";
			++fails;
		}
		return fails;
	}

	// e * (-1) == -e
	int VerifyNegation(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 7.0, 3.0e-16 };
		if (!close_rel(value(scale_expansion(e, -1.0)), -value(e), 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL e * (-1) != -e\n";
			++fails;
		}
		return fails;
	}

	// (a + b) * c == a*c + b*c
	int VerifyDistributive(bool reportTestCases) {
		int fails = 0;
		std::vector<double> a = { 10.0, 1.0e-15 }, b = { 5.0, 5.0e-16 };
		double c = 3.0;
		double lhs = value(scale_expansion(renormalize_expansion(linear_expansion_sum(a, b)), c));
		double rhs = value(linear_expansion_sum(scale_expansion(a, c), scale_expansion(b, c)));
		if (!close_rel(lhs, rhs, 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL distributivity\n";
			++fails;
		}
		return fails;
	}

	// e x f == f x e
	int VerifyCommutative(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 6.0, 2.0e-16 }, f = { 7.0, 3.0e-16 };
		if (!close_rel(value(expansion_product(e, f)), value(expansion_product(f, e)), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL commutativity e x f != f x e\n";
			++fails;
		}
		return fails;
	}

	// (e x f) x g == e x (f x g)
	int VerifyAssociative(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 2.0 }, f = { 3.0 }, g = { 5.0 };
		double lhs = value(expansion_product(expansion_product(e, f), g));
		double rhs = value(expansion_product(e, expansion_product(f, g)));
		if (!close_rel(lhs, rhs, 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL associativity\n";
			++fails;
		}
		return fails;
	}

	// precision: (1 + eps) * (1 + eps) keeps the cross term a single double drops
	int VerifyPrecision(bool reportTestCases) {
		int fails = 0;
		double eps = std::ldexp(1.0, -40);
		std::vector<double> x = renormalize_expansion(grow_expansion({ 1.0 }, eps));  // 1 + 2^-40
		std::vector<double> p = expansion_product(x, x);  // 1 + 2*eps + eps^2
		double expected = 1.0 + 2.0 * eps + eps * eps;
		if (!close_rel(value(p), expected, 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL precision (1+eps)^2  got " << value(p) << "\n";
			++fails;
		}
		return fails;
	}

	// fuzz: commutativity and scalar correctness over random expansions
	int VerifyMultiplication_Fuzz(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0x3B7C5EEDULL);
		std::uniform_real_distribution<double> sd(-4.0, 4.0);
		int fails = 0;
		for (unsigned i = 0; i < nrIterations; ++i) {
			std::vector<double> e = random_expansion(rng), f = random_expansion(rng);
			if (!close_rel(value(expansion_product(e, f)), value(expansion_product(f, e)), 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL fuzz commutativity (iter " << i << ")\n";
				++fails;
			}
			double b = sd(rng);
			if (!close_rel(value(scale_expansion(e, b)), value(e) * b, 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL fuzz scale (iter " << i << ")\n";
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

	std::string test_suite  = "expansion multiplication";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyScalarCorrectness(reportTestCases), "expansion", "scalar correctness");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentity(reportTestCases),          "expansion", "multiplicative identity");
	nrOfFailedTestCases += ReportTestResult(VerifyZero(reportTestCases),              "expansion", "multiply by zero");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation(reportTestCases),          "expansion", "negation e*(-1)");
	nrOfFailedTestCases += ReportTestResult(VerifyDistributive(reportTestCases),      "expansion", "distributivity");
	nrOfFailedTestCases += ReportTestResult(VerifyCommutative(reportTestCases),       "expansion", "commutativity");
	nrOfFailedTestCases += ReportTestResult(VerifyAssociative(reportTestCases),       "expansion", "associativity");
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision(reportTestCases),         "expansion", "precision preservation");

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication_Fuzz(reportTestCases, 1000), "expansion", "multiplication fuzz");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication_Fuzz(reportTestCases, 1000),  "expansion", "multiplication fuzz x1k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication_Fuzz(reportTestCases, 10000), "expansion", "multiplication fuzz x10k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication_Fuzz(reportTestCases, 100000),"expansion", "multiplication fuzz x100k");
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
