// division.cpp: regression tests for expansion (multi-component) division
//
// Division is not error-free: expansion_reciprocal uses Newton iteration and
// expansion_quotient(e,f) = e * reciprocal(f), so results are approximate.
// These tests verify the algebraic invariants within a relative tolerance:
// reciprocal of one, multiplicative inverse, double reciprocal, quotient
// identity, self-division, the inverse property (e/f)*f ~= e, quotient ==
// e*reciprocal(f), and extreme scales.
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

	// reciprocal([1]) == [1]
	int VerifyReciprocalOfOne(bool reportTestCases) {
		int fails = 0;
		if (!close_rel(value(expansion_reciprocal({ 1.0 })), 1.0, 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL reciprocal(1) != 1\n";
			++fails;
		}
		return fails;
	}

	// reciprocal of simple values: 1/v
	int VerifyReciprocalSimple(bool reportTestCases) {
		int fails = 0;
		for (double v : { 2.0, 4.0, 8.0, 3.0, 10.0, 0.5 }) {
			std::vector<double> r = expansion_reciprocal({ v });
			if (!close_rel(value(r), 1.0 / v, 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL reciprocal(" << v << ") != " << (1.0 / v) << "\n";
				++fails;
			}
		}
		return fails;
	}

	// e * reciprocal(e) ~= 1
	int VerifyMultiplicativeInverse(bool reportTestCases) {
		int fails = 0;
		for (std::vector<double> e : { std::vector<double>{ 3.0 }, std::vector<double>{ 7.0, 1.0e-15 }, std::vector<double>{ 100.0, 2.0e-14 } }) {
			double got = value(expansion_product(e, expansion_reciprocal(e)));
			if (!close_rel(got, 1.0, 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL e * reciprocal(e) != 1  (got " << got << ")\n";
				++fails;
			}
		}
		return fails;
	}

	// reciprocal(reciprocal(e)) ~= e
	int VerifyDoubleReciprocal(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 3.0, 1.0e-16 };
		double got = value(expansion_reciprocal(expansion_reciprocal(e)));
		if (!close_rel(got, value(e), 1.0e-10)) {
			if (reportTestCases) std::cout << "    FAIL reciprocal(reciprocal(e)) != e  (got " << got << ")\n";
			++fails;
		}
		return fails;
	}

	// e / [1] == e
	int VerifyQuotientIdentity(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 12.0, 3.0e-15 };
		if (!close_rel(value(expansion_quotient(e, { 1.0 })), value(e), 1.0e-14)) {
			if (reportTestCases) std::cout << "    FAIL e / 1 != e\n";
			++fails;
		}
		return fails;
	}

	// e / e ~= 1
	int VerifySelfDivision(bool reportTestCases) {
		int fails = 0;
		for (std::vector<double> e : { std::vector<double>{ 5.0 }, std::vector<double>{ 13.0, 7.0e-16 } }) {
			double got = value(expansion_quotient(e, e));
			if (!close_rel(got, 1.0, 1.0e-12)) {
				if (reportTestCases) std::cout << "    FAIL e / e != 1  (got " << got << ")\n";
				++fails;
			}
		}
		return fails;
	}

	// (e / f) * f ~= e
	int VerifyInverseProperty(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 10.0, 1.0e-15 }, f = { 3.0, 2.0e-16 };
		double got = value(expansion_product(expansion_quotient(e, f), f));
		if (!close_rel(got, value(e), 1.0e-11)) {
			if (reportTestCases) std::cout << "    FAIL (e/f)*f != e  (got " << got << ")\n";
			++fails;
		}
		return fails;
	}

	// e / f == e * reciprocal(f)  (by construction; values must match)
	int VerifyQuotientVsReciprocal(bool reportTestCases) {
		int fails = 0;
		std::vector<double> e = { 7.0, 1.0e-16 }, f = { 11.0, 3.0e-16 };
		double q = value(expansion_quotient(e, f));
		double m = value(expansion_product(e, expansion_reciprocal(f)));
		if (!close_rel(q, m, 1.0e-15)) {
			if (reportTestCases) std::cout << "    FAIL e/f != e*reciprocal(f)\n";
			++fails;
		}
		return fails;
	}

	// extreme scale divisor and dividend
	int VerifyExtremeScales(bool reportTestCases) {
		int fails = 0;
		if (!close_rel(value(expansion_quotient({ 1.0e16 }, { 1.0e8 })), 1.0e8, 1.0e-10)) {
			if (reportTestCases) std::cout << "    FAIL 1e16 / 1e8\n";
			++fails;
		}
		if (!close_rel(value(expansion_quotient({ 1.0 }, { 1.0e12 })), 1.0e-12, 1.0e-10)) {
			if (reportTestCases) std::cout << "    FAIL 1 / 1e12\n";
			++fails;
		}
		return fails;
	}

	// fuzz: e * reciprocal(e) ~= 1 over random positive expansions
	[[maybe_unused]] 	int VerifyDivision_Fuzz(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0xD1F5EEDULL);
		std::uniform_real_distribution<double> mag(0.25, 16.0);
		int fails = 0;
		for (unsigned i = 0; i < nrIterations; ++i) {
			std::vector<double> e = { mag(rng), std::ldexp(mag(rng), -55) };
			double got = value(expansion_product(e, expansion_reciprocal(e)));
			if (!close_rel(got, 1.0, 1.0e-11)) {
				if (reportTestCases) std::cout << "    FAIL fuzz e*recip(e) != 1 (iter " << i << ", got " << got << ")\n";
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

	std::string test_suite  = "expansion division";
	std::string test_tag    = "division";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocalOfOne(reportTestCases),      "expansion", "reciprocal(1)=1");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocalSimple(reportTestCases),     "expansion", "reciprocal simple values");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplicativeInverse(reportTestCases),"expansion", "e*reciprocal(e)=1");
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleReciprocal(reportTestCases),     "expansion", "reciprocal(reciprocal(e))=e");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientIdentity(reportTestCases),     "expansion", "e/1=e");
	nrOfFailedTestCases += ReportTestResult(VerifySelfDivision(reportTestCases),         "expansion", "e/e=1");
	nrOfFailedTestCases += ReportTestResult(VerifyInverseProperty(reportTestCases),      "expansion", "(e/f)*f=e");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientVsReciprocal(reportTestCases), "expansion", "e/f = e*reciprocal(f)");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremeScales(reportTestCases),        "expansion", "extreme scales");

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyDivision_Fuzz(reportTestCases, 1000), "expansion", "division fuzz");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision_Fuzz(reportTestCases, 1000),  "expansion", "division fuzz x1k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDivision_Fuzz(reportTestCases, 10000), "expansion", "division fuzz x10k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDivision_Fuzz(reportTestCases, 100000),"expansion", "division fuzz x100k");
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
