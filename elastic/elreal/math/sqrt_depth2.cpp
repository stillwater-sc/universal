// sqrt_depth2.cpp: validate Phase L.2.b depth-2 sqrt
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase L.2.b (#906) added depth-2 to elreal::sqrt. The new gen_newton_sqrt
// evaluator computes
//
//   c_2 = (a.at(2) - c_1 * c_1) / (2 * c_0)
//
// at depth 2, picking up the operand depth-2 contribution plus the
// self-interaction term c_1^2. Unlike gen_newton_div (which returns 0
// for pure-double inputs), gen_newton_sqrt produces non-zero c_2 even
// for pure-double inputs (sqrt(2), sqrt(3), etc.) because c_1^2 is
// always non-zero when c_1 is non-zero.
//
// Validation strategy
// -------------------
// - For pure-double inputs (sqrt(2), sqrt(3)), assert c_2 is non-zero,
//   finite, and respects the non-overlapping property.
// - For multi-component inputs (sqrt(pi), sqrt(elreal_e)), same plus
//   verifying c_2 picks up the operand depth-2 contribution.
// - Direct at(2) call must auto-materialise c_1 first.
// - Depth 3+ still returns 0 (deferred to a follow-up).
// - Edge cases: sqrt(-1) returns NaN at depth 0; depth-2 must not
//   propagate NaN beyond what depth 0 already produces.

#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <limits>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/constants/elreal_constants.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	// IEEE-754 ulp via std::nextafter -- correct for normals, subnormals,
	// and zero.
	double ulp_of(double x) {
		return std::abs(std::nextafter(x, std::numeric_limits<double>::infinity()) - x);
	}
}  // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase L.2.b sqrt depth-2";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Pure-double non-perfect-square inputs: c_2 must be non-zero
	// (self-interaction term c_1^2) and non-overlapping with c_1.
	for (double x : { 2.0, 3.0, 5.0, 1.5, 7.0 }) {
		elreal r = sqrt(elreal(x));
		double c_1 = r.at(1);
		double c_2 = r.at(2);

		if (c_2 == 0.0) {
			std::cerr << "FAIL: sqrt(" << x << ") depth-2 unexpectedly zero\n";
			++nrOfFailedTestCases;
		}
		if (!std::isfinite(c_2)) {
			std::cerr << "FAIL: sqrt(" << x << ") depth-2 not finite: " << c_2 << "\n";
			++nrOfFailedTestCases;
		}
		double ulp_c1 = ulp_of(c_1);
		if (std::abs(c_2) > ulp_c1 / 2.0) {
			std::cerr << "FAIL: sqrt(" << x << ") depth-2 violates non-overlap: |c_2|="
				<< std::abs(c_2) << " > ulp(c_1)/2=" << (ulp_c1 / 2.0) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// Perfect squares: c_1 should be 0 (no residual) and c_2 = 0 too.
	for (double x : { 1.0, 4.0, 9.0, 16.0, 25.0, 100.0, 10000.0 }) {
		elreal r = sqrt(elreal(x));
		double c_0 = r.at(0);
		double c_1 = r.at(1);
		double c_2 = r.at(2);
		double expected_c0 = std::sqrt(x);

		if (c_0 != expected_c0) {
			std::cerr << "FAIL: sqrt(" << x << ") c_0=" << c_0
				<< " expected " << expected_c0 << "\n";
			++nrOfFailedTestCases;
		}
		if (c_1 != 0.0) {
			std::cerr << "FAIL: sqrt(" << x << ") c_1 != 0 for perfect square: "
				<< c_1 << "\n";
			++nrOfFailedTestCases;
		}
		if (c_2 != 0.0) {
			std::cerr << "FAIL: sqrt(" << x << ") c_2 != 0 for perfect square: "
				<< c_2 << "\n";
			++nrOfFailedTestCases;
		}
	}

	// Multi-component input: sqrt(pi). c_2 must be non-zero, non-overlapping.
	{
		elreal r = sqrt(elreal_pi());
		double c_1 = r.at(1);
		double c_2 = r.at(2);

		if (c_2 == 0.0) {
			std::cerr << "FAIL: sqrt(pi) depth-2 unexpectedly zero\n";
			++nrOfFailedTestCases;
		}
		if (!std::isfinite(c_2)) {
			std::cerr << "FAIL: sqrt(pi) depth-2 not finite: " << c_2 << "\n";
			++nrOfFailedTestCases;
		}
		double ulp_c1 = ulp_of(c_1);
		if (std::abs(c_2) > ulp_c1 / 2.0) {
			std::cerr << "FAIL: sqrt(pi) depth-2 violates non-overlap\n";
			++nrOfFailedTestCases;
		}
	}

	// Direct at(2) call must auto-materialise c_1 first.
	{
		elreal r = sqrt(elreal(2.0));
		double c_2 = r.at(2);  // skip at(0), at(1)
		if (c_2 == 0.0) {
			std::cerr << "FAIL: direct at(2) on sqrt(2) returned 0\n";
			++nrOfFailedTestCases;
		}
		if (r.computed_depth() != 3) {
			std::cerr << "FAIL: computed_depth = " << r.computed_depth()
				<< " expected 3 after direct at(2)\n";
			++nrOfFailedTestCases;
		}
	}

	// Depth 3+ should still return 0 (deferred).
	{
		elreal r = sqrt(elreal(2.0));
		if (r.at(3) != 0.0) {
			std::cerr << "FAIL: sqrt depth-3 not zero (deferred): " << r.at(3) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// Edge case: sqrt(negative) -- depth-0 is NaN; depth-2 must not
	// propagate beyond what depth-0 already produces (no generator
	// is installed when c_0 is non-finite or zero per operator/'s guard).
	{
		elreal r = sqrt(elreal(-1.0));
		// at(0) is NaN. at(2) should be 0 (no generator).
		if (!std::isnan(r.at(0))) {
			std::cerr << "FAIL: sqrt(-1) leading is not NaN: " << r.at(0) << "\n";
			++nrOfFailedTestCases;
		}
		if (r.at(2) != 0.0) {
			std::cerr << "FAIL: sqrt(-1) depth-2 propagated something: " << r.at(2)
				<< " (expected 0 since c_0 is NaN, no generator)\n";
			++nrOfFailedTestCases;
		}
	}

	// Edge case: sqrt(0). Depth-0 is 0; the operator's guard disables
	// the generator (c_0 == 0.0), so depth-2 must be 0.
	{
		elreal r = sqrt(elreal(0.0));
		if (r.at(0) != 0.0) {
			std::cerr << "FAIL: sqrt(0) leading is not 0\n";
			++nrOfFailedTestCases;
		}
		if (r.at(2) != 0.0) {
			std::cerr << "FAIL: sqrt(0) depth-2 is not 0: " << r.at(2) << "\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
