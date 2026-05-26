// scale_expansion_nonoverlap_bug.cpp: regression guard for the scale_expansion /
// expansion_product non-overlapping property.
//
// Historically scale_expansion returned the sorted partial products without a
// renormalization pass, so the result could violate the non-overlapping
// expansion invariant (and ereal multiplication inherited it -- issue #981,
// fixed in #982). This test scales and multiplies a variety of multi-component
// expansions by non-trivial scalars and asserts that every result is
// non-overlapping (adjacent components separated by at least 2^53) and that its
// value matches the exact reference, guarding against the bug's return.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;
	using namespace sw::universal::expansion_ops;

	double value(const std::vector<double>& e) { double s = 0.0; for (double v : e) s += v; return s; }

	// Non-overlapping: adjacent components separated by at least 2^53 in
	// magnitude (no shared significand bits). Returns the offending index or -1.
	int first_overlap(const std::vector<double>& e) {
		const double SEP = std::pow(2.0, 53);
		for (size_t i = 1; i < e.size(); ++i) {
			if (e[i] == 0.0) continue;
			if (std::abs(e[i - 1]) / std::abs(e[i]) < SEP) return static_cast<int>(i);
		}
		return -1;
	}

	bool close_rel(double x, double y, double relTol) {
		double scale = std::max({ std::abs(x), std::abs(y), 1.0 });
		return std::abs(x - y) <= relTol * scale;
	}

	// build a genuine multi-component expansion: leading + a few small tails
	std::vector<double> make_expansion(double lead, std::initializer_list<double> tails) {
		std::vector<double> e{ lead };
		for (double t : tails) e = renormalize_expansion(grow_expansion(e, t));
		return e;
	}

	// scale_expansion(e, b) must be non-overlapping and equal value(e)*b
	int VerifyScaleNonoverlapping(bool reportTestCases) {
		int fails = 0;
		struct Case { std::vector<double> e; double b; };
		std::vector<Case> cases = {
			{ make_expansion(1.0,  { 1.0e-16 }),            0.1 },
			{ make_expansion(2.0,  { 1.0e-16 }),            0.3 },
			{ make_expansion(7.0,  { 3.0e-16, 1.0e-31 }),   1.0 / 7.0 },
			{ make_expansion(1.0e6,{ 1.0e-10 }),            3.0 },
			{ make_expansion(123.0,{ 4.0e-15, 2.0e-30 }),   -2.5 },
		};
		for (const auto& c : cases) {
			std::vector<double> r = scale_expansion(c.e, c.b);
			int ov = first_overlap(r);
			if (ov >= 0) {
				if (reportTestCases) std::cout << "    FAIL scale_expansion(.., " << c.b
					<< ") overlaps at limb " << ov << "\n";
				++fails;
			}
			if (!close_rel(value(r), value(c.e) * c.b, 1.0e-13)) {
				if (reportTestCases) std::cout << "    FAIL scale_expansion value (b=" << c.b << ")\n";
				++fails;
			}
		}
		return fails;
	}

	// expansion_product(e, f) must likewise be non-overlapping (the path ereal
	// multiplication takes; the original #981 surface)
	int VerifyProductNonoverlapping(bool reportTestCases) {
		int fails = 0;
		std::vector<std::vector<double>> exps = {
			make_expansion(3.0,   { 1.0e-16 }),
			make_expansion(11.0,  { 5.0e-16, 2.0e-31 }),
			make_expansion(1.0e8, { 1.0e-9 }),
		};
		for (const auto& e : exps) {
			for (const auto& f : exps) {
				std::vector<double> p = expansion_product(e, f);
				int ov = first_overlap(p);
				if (ov >= 0) {
					if (reportTestCases) std::cout << "    FAIL expansion_product overlaps at limb " << ov << "\n";
					++fails;
				}
				if (!close_rel(value(p), value(e) * value(f), 1.0e-12)) {
					if (reportTestCases) std::cout << "    FAIL expansion_product value\n";
					++fails;
				}
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "expansion scale/product non-overlapping (regression #981)";
	std::string test_tag    = "non-overlapping";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(VerifyScaleNonoverlapping(reportTestCases),   "expansion", "scale_expansion non-overlapping");
	nrOfFailedTestCases += ReportTestResult(VerifyProductNonoverlapping(reportTestCases), "expansion", "expansion_product non-overlapping");

#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
