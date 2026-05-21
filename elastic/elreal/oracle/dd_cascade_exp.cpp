// dd_cascade_exp.cpp: cross-validate dd_cascade::exp against elreal::exp (Phase G, #880)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Demonstrates the elreal-as-oracle validation pipeline. The target type is
// `dd_cascade` (Bailey/Hida double-double via floatcascade<2>); the oracle
// is `elreal` (McCleeary lazy refinement). The two paths share zero code,
// so an agreement at the helper's tolerance is genuine cross-implementation
// confirmation.
//
// Current ceiling: elreal's depth-1 cap holds the actual validation at
// double precision today. The pipeline -- and this test -- transparently
// pick up the precision improvement when elreal gains depth-2+ refinement.

#include <universal/utility/directives.hpp>

#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_elreal_oracle.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase G dd_cascade::exp oracle";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Representative inputs span small / modest / slightly larger arguments,
	// all within the safe operating range of both dd_cascade::exp and
	// elreal::exp.
	for (double x : { -2.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 5.0 }) {
		dd_cascade target = exp(dd_cascade(x));
		elreal    oracle = exp(elreal(x));

		std::string label = std::string("dd_cascade::exp(") + std::to_string(x)
			+ ") vs elreal::exp";
		nrOfFailedTestCases += report_against_elreal_oracle(label.c_str(),
		                                                    target, oracle);
	}

	// Anchor: exp(0) is exactly 1.0 on both sides.
	{
		dd_cascade target = exp(dd_cascade(0.0));
		elreal     oracle = exp(elreal(0.0));
		if (double(target) != 1.0 || double(oracle) != 1.0) {
			std::cerr << "FAIL: exp(0) anchor: target=" << double(target)
				<< " oracle=" << double(oracle) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// Sanity check the helper's reject path: a deliberately wrong "oracle"
	// (factor 2 off) must be rejected. Guards against the helper
	// accidentally passing on any input due to loose tolerance.
	{
		dd_cascade target = exp(dd_cascade(1.0));               // ~= e
		elreal     wrong  = elreal(2.0) * exp(elreal(1.0));     // 2*e
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: oracle helper accepted a 2x-off reference -- "
				<< "tolerance is too loose to catch even gross errors\n";
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
