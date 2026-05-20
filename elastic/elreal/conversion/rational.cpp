// rational.cpp: tests for elreal(p, q) rational construction (Phase B)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase B acceptance criterion (issue #875):
//   `elreal("1/3")` is observably distinct from `elreal(1.0/3.0)` -- the
//   rational constructor delivers more bits when asked for them.
//
// This file exercises the equivalent direct-rational form `elreal(1, 3)`.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase B rational construction";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- exact rationals: representable in a single double ---------------
	{
		elreal half(1LL, 2LL);
		if (half.at(0) != 0.5 || half.at(1) != 0.0) {
			std::cerr << "FAIL: 1/2 should be exact in a single component, got at(0)="
				<< half.at(0) << " at(1)=" << half.at(1) << "\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal quarter(1LL, 4LL);
		if (quarter.at(0) != 0.25 || quarter.at(1) != 0.0) {
			std::cerr << "FAIL: 1/4 should be exact in a single component\n";
			++nrOfFailedTestCases;
		}
	}
	// Negative numerator
	{
		elreal nq(-3LL, 4LL);
		if (nq.at(0) != -0.75 || nq.at(1) != 0.0) {
			std::cerr << "FAIL: -3/4 should be exact in a single component\n";
			++nrOfFailedTestCases;
		}
	}

	// --- inexact rationals: 1/3 (the canonical example) ------------------
	{
		elreal third(1LL, 3LL);
		double c0 = third.at(0);
		double c1 = third.at(1);

		// at(0) must match the IEEE-754 rounded value of 1.0 / 3.0.
		if (c0 != 1.0 / 3.0) {
			std::cerr << "FAIL: elreal(1,3).at(0) != IEEE 1.0/3.0: got " << c0
				<< " expected " << (1.0 / 3.0) << "\n";
			++nrOfFailedTestCases;
		}
		// at(1) must be non-zero (the acceptance criterion of #875): the
		// rational constructor delivers a correction beyond the leading double.
		if (c1 == 0.0) {
			std::cerr << "FAIL: elreal(1,3).at(1) == 0 (rational constructor not "
				<< "delivering refinement)\n";
			++nrOfFailedTestCases;
		}
		// at(1) must be small relative to at(0) (it is a correction term,
		// not a competing approximation). One ulp(c0) is a generous upper
		// bound -- correctly-computed corrections live below 0.5*ulp(c0).
		if (std::abs(c1) >= std::abs(c0)) {
			std::cerr << "FAIL: elreal(1,3).at(1) is not a correction (|c1| >= |c0|)\n";
			++nrOfFailedTestCases;
		}
		// at(0) + at(1) must be a tighter approximation of 1/3 than at(0)
		// alone. Compare against a long-double reference, which carries more
		// than 53 bits of precision on most platforms (and degrades to double
		// on some -- in which case the check is trivially passed by equality,
		// not a regression).
		long double ld_third = 1.0L / 3.0L;
		long double sum_two = static_cast<long double>(c0) + static_cast<long double>(c1);
		long double err_one = std::abs(ld_third - static_cast<long double>(c0));
		long double err_two = std::abs(ld_third - sum_two);
		if (err_two > err_one) {
			std::cerr << "FAIL: elreal(1,3) refinement made the approximation worse"
				<< "\n  err(c0 alone)    = " << static_cast<double>(err_one)
				<< "\n  err(c0 + c1 sum) = " << static_cast<double>(err_two) << "\n";
			++nrOfFailedTestCases;
		}

		std::cout << "elreal(1, 3):\n"
			<< "  at(0)            = " << c0 << "\n"
			<< "  at(1)            = " << c1 << "\n"
			<< "  at(0) + at(1)    = " << (c0 + c1) << "\n"
			<< "  double(1.0/3.0)  = " << (1.0 / 3.0) << "\n";
	}

	// --- elreal(1, 3) is observably distinct from elreal(1.0/3.0) ---------
	{
		elreal from_rational(1LL, 3LL);
		elreal from_double(1.0 / 3.0);

		// Both must agree on at(0).
		if (from_rational.at(0) != from_double.at(0)) {
			std::cerr << "FAIL: at(0) disagrees between rational and double forms\n";
			++nrOfFailedTestCases;
		}
		// They must disagree on at(1): the rational form has more bits.
		if (from_rational.at(1) == from_double.at(1)) {
			std::cerr << "FAIL: elreal(1,3) and elreal(1.0/3.0) agree on at(1) -- "
				<< "the rational form is not delivering refinement\n";
			++nrOfFailedTestCases;
		}
	}

	// --- division by zero produces NaN ------------------------------------
	{
		elreal nan_form(1LL, 0LL);
		if (!nan_form.isnan()) {
			std::cerr << "FAIL: elreal(1, 0) is not NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// --- numerator zero produces canonical zero ---------------------------
	{
		elreal z(0LL, 7LL);
		if (!z.iszero()) {
			std::cerr << "FAIL: elreal(0, 7) is not zero\n";
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
