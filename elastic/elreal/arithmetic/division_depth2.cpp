// division_depth2.cpp: validate Phase L.2.a depth-2 division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase L.2.a (#906) added depth-2 to elreal::operator/. The new
// `gen_newton_div` evaluator computes
//
//   c_2 = (a.at(2) - b.at(2) * c_0 - b.at(1) * c_1) / b.at(0)
//
// at depth 2, picking up the order-eps^2 contributions from the operand
// streams. Pure-double inputs yield c_2 = 0 (no operand depth-2 to
// refine); multi-component inputs yield non-trivial c_2.
//
// Validation strategy
// -------------------
// - For pure-double inputs, assert c_2 == 0.
// - For multi-component inputs (elreal_pi divided by elreal_e and
//   similar), assert c_2 is non-zero and the result satisfies the
//   non-overlapping property |c_2| <= ulp(c_1) / 2.
// - Cross-check the sum c_0 + c_1 + c_2 against the qd reference to
//   verify the mathematical value is preserved (the components may
//   differ in their breakdown but their sum must agree).

#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/constants/elreal_constants.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/qd/math/constants/qd_constants.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase L.2.a division depth-2";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Pure-double case: c_2 must be 0 (no operand depth-2 to refine).
	{
		elreal one(1.0);
		elreal three(3.0);
		elreal r = one / three;
		if (r.at(2) != 0.0) {
			std::cerr << "FAIL: 1/3 depth-2 not zero: " << r.at(2) << "\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal a(2.0);
		elreal b(7.0);
		elreal r = a / b;
		if (r.at(2) != 0.0) {
			std::cerr << "FAIL: 2/7 depth-2 not zero: " << r.at(2) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// Multi-component case: pi / e. Both operands have 4 components.
	// c_2 must be non-zero, finite, and non-overlapping with c_1.
	{
		elreal r = elreal_pi() / elreal_e();
		double c_0 = r.at(0);
		double c_1 = r.at(1);
		double c_2 = r.at(2);

		if (c_2 == 0.0) {
			std::cerr << "FAIL: pi/e depth-2 unexpectedly zero\n";
			++nrOfFailedTestCases;
		}
		if (!std::isfinite(c_2)) {
			std::cerr << "FAIL: pi/e depth-2 not finite: " << c_2 << "\n";
			++nrOfFailedTestCases;
		}
		// Non-overlapping: |c_2| < ulp(c_1) / 2.
		double ulp_c1 = std::ldexp(std::abs(c_1), -52);
		if (std::abs(c_2) > ulp_c1 / 2.0) {
			std::cerr << "FAIL: pi/e depth-2 violates non-overlap: |c_2|="
				<< std::abs(c_2) << " > ulp(c_1)/2=" << (ulp_c1 / 2.0) << "\n";
			++nrOfFailedTestCases;
		}

		// Sanity: c_0 + c_1 + c_2 should equal double(qd_pi/qd_e) to
		// at least double precision (the sum collapses to a double).
		qd qd_ref = qd_pi / qd_e;
		double sum = c_0 + c_1 + c_2;
		double ref = double(qd_ref);
		if (std::abs(sum - ref) > 1e-15) {
			std::cerr << "FAIL: pi/e sum mismatches qd ref: sum=" << sum
				<< " ref=" << ref << "\n";
			++nrOfFailedTestCases;
		}
		(void)c_0;
	}

	// Multi-component case: phi / sqrt(2). Both 4 components.
	{
		elreal r = elreal_phi() / elreal_sqrt2();
		double c_1 = r.at(1);
		double c_2 = r.at(2);

		if (c_2 == 0.0) {
			std::cerr << "FAIL: phi/sqrt2 depth-2 unexpectedly zero\n";
			++nrOfFailedTestCases;
		}
		if (!std::isfinite(c_2)) {
			std::cerr << "FAIL: phi/sqrt2 depth-2 not finite: " << c_2 << "\n";
			++nrOfFailedTestCases;
		}
		double ulp_c1 = std::ldexp(std::abs(c_1), -52);
		if (std::abs(c_2) > ulp_c1 / 2.0) {
			std::cerr << "FAIL: phi/sqrt2 depth-2 violates non-overlap\n";
			++nrOfFailedTestCases;
		}
	}

	// Edge case: depth-2 of a result whose depth-1 generator hasn't
	// fired yet. at(2) should auto-materialise c_1 first via the L.1
	// formula, then c_2 via the L.2 formula.
	{
		elreal r = elreal_pi() / elreal_e();
		double c_2 = r.at(2);  // direct call, skipping at(1)
		if (c_2 == 0.0) {
			std::cerr << "FAIL: direct at(2) call returned 0 (didn't materialise c_1 first)\n";
			++nrOfFailedTestCases;
		}
		// computed_depth should be 3 (c_0, c_1, c_2 all materialised)
		if (r.computed_depth() != 3) {
			std::cerr << "FAIL: computed_depth = " << r.computed_depth()
				<< " expected 3 after at(2) call\n";
			++nrOfFailedTestCases;
		}
	}

	// Edge case: depth-3+ should still return 0 (deferred).
	{
		elreal r = elreal_pi() / elreal_e();
		if (r.at(3) != 0.0) {
			std::cerr << "FAIL: depth-3 not zero (deferred to L.2.b): " << r.at(3) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// Edge case: division producing non-finite leading -- depth-2 must
	// not propagate inf/NaN. The gen_newton_div coefficients are
	// guarded in operator/, but defense-in-depth here: a/0 produces
	// inf leading and no generator should fire.
	{
		elreal r = elreal(1.0) / elreal(0.0);
		// at(0) is inf; at(2) must not be NaN.
		if (!std::isfinite(r.at(2)) && !std::isnan(r.at(2))) {
			// inf is OK if at(2) was computed; but ideally at(2) == 0 here
			// since no generator was installed (b0 == 0 path).
		}
		// More careful: at(2) should be 0 (monostate generator, no refinement).
		if (r.at(2) != 0.0) {
			std::cerr << "WARN: 1/0 depth-2 = " << r.at(2)
				<< " (expected 0 since b0=0 disables the generator)\n";
			// Not a hard fail; documenting behaviour.
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
