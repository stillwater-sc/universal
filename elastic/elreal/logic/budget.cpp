// budget.cpp: tests for the refinement budget contract (Phase D)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The lazy-real comparison protocol is *budgeted*: when the budget is
// exhausted with all-zero components, the value is treated as zero. This
// file exercises the contract -- a value that is mathematically non-zero
// but whose first non-zero component lies past the budget must compare
// equal to zero under that budget, and unequal once the budget is large
// enough to find the component.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase D budget";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- Default budget constant is documented and usable ---------------
	{
		// Semantic minimum: must be at least 2 so refinement past the
		// leading double is possible at the default. A future tuning pass
		// can lower or raise the default, but values < 2 break the
		// rational-vs-rounded distinctness test in compare.cpp.
		if (elreal_default_budget < 2) {
			std::cerr << "FAIL: elreal_default_budget should be >= 2 "
				<< "(too small to refine past the leading double)\n";
			++nrOfFailedTestCases;
		}
		// Documented contract: header docblock and PR description both
		// state the default is 8 (~424 bits cumulative). Lock the contract
		// here so a silent change is caught -- if the default is intentionally
		// re-tuned, this line gets updated in the same PR.
		if (elreal_default_budget != 8) {
			std::cerr << "FAIL: elreal_default_budget expected 8 (the documented "
				<< "contract; see elreal_impl.hpp Phase D docblock), got "
				<< elreal_default_budget << '\n';
			++nrOfFailedTestCases;
		}
		// budget == 0 means "do not materialise anything"; sign returns 0
		// regardless of value.
		elreal a(3.14);
		if (sign(a, 0) != 0) {
			std::cerr << "FAIL: sign(3.14, budget=0) != 0\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Budget controls whether refinement finds the first non-zero ---
	// (1/3 rational) - (1/3 double) has at(0) == 0 and at(1) != 0.
	// budget=1 only looks at component 0 (zero), so sign returns 0.
	// budget>=2 finds the non-zero correction at component 1.
	{
		elreal rat(1LL, 3LL);
		elreal rnd(1.0/3.0);
		elreal diff = rat - rnd;

		if (sign(diff, 1) != 0) {
			std::cerr << "FAIL: sign(diff, budget=1) != 0 "
				<< "(found a non-zero past component 0 at the budget cliff)\n";
			++nrOfFailedTestCases;
		}
		if (sign(diff, 2) != +1) {
			std::cerr << "FAIL: sign(diff, budget=2) != +1 "
				<< "(did not find the rational residual at component 1)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- compare() respects the budget for the same reason -------------
	{
		elreal rat(1LL, 3LL);
		elreal rnd(1.0/3.0);
		if (compare(rat, rnd, 1) != 0) {
			std::cerr << "FAIL: compare(rat, rnd, budget=1) != 0 "
				<< "(found non-zero past component 0 at the budget cliff)\n";
			++nrOfFailedTestCases;
		}
		if (compare(rat, rnd, 2) != +1) {
			std::cerr << "FAIL: compare(rat, rnd, budget=2) != +1 "
				<< "(did not distinguish rational from rounded at budget=2)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- a - a is all-zero at any depth -- budget cannot change result -
	{
		elreal a(22.0/7.0);
		for (std::size_t budget : {1U, 2U, 4U, 8U, 64U}) {
			if (sign(a - a, budget) != 0) {
				std::cerr << "FAIL: sign(a - a, budget=" << budget << ") != 0\n";
				++nrOfFailedTestCases;
			}
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
