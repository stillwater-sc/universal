// dd_math_sweep.cpp: cross-validate dd math suite against elreal (Phase J, #904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Exercises every math function the dd type exposes through the
// elreal-as-oracle pipeline. Each call goes through:
//
//   target = math_fn(dd(x))
//   oracle = math_fn(elreal(x))
//   check_against_elreal_oracle(target, oracle)
//
// The two paths share no code (Bailey/Hida hand-crafted dd algorithms
// on one side, McCleeary lazy refinement on the other) so a clean
// sweep is genuine cross-implementation confirmation at double
// precision. See math_sweep_common.hpp for the precision-ceiling
// caveat.

#include <universal/utility/directives.hpp>

#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>
#include "math_sweep_common.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::elreal_oracle_sweep;

	std::string test_suite = "elreal Phase J: dd math oracle sweep";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	const auto& g = general_inputs();
	const auto& p = positive_inputs();
	const auto& b = bounded_inputs();

	// sqrt and hypot
	nrOfFailedTestCases += sweep_unary<dd>("dd::sqrt",  p,
		[](const dd& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	// exp family
	nrOfFailedTestCases += sweep_unary<dd>("dd::exp",   g,
		[](const dd& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::exp2",  g,
		[](const dd& a) { return exp2(a); },
		[](const elreal& a) { return exp2(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::expm1", g,
		[](const dd& a) { return expm1(a); },
		[](const elreal& a) { return expm1(a); });

	// log family
	nrOfFailedTestCases += sweep_unary<dd>("dd::log",   p,
		[](const dd& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::log2",  p,
		[](const dd& a) { return log2(a); },
		[](const elreal& a) { return log2(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::log10", p,
		[](const dd& a) { return log10(a); },
		[](const elreal& a) { return log10(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::log1p", log1p_inputs(),
		[](const dd& a) { return log1p(a); },
		[](const elreal& a) { return log1p(a); });

	// trig
	nrOfFailedTestCases += sweep_unary<dd>("dd::sin",   g,
		[](const dd& a) { return sin(a); },
		[](const elreal& a) { return sin(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::cos",   g,
		[](const dd& a) { return cos(a); },
		[](const elreal& a) { return cos(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::tan",   g,
		[](const dd& a) { return tan(a); },
		[](const elreal& a) { return tan(a); });

	// inverse trig
	nrOfFailedTestCases += sweep_unary<dd>("dd::asin",  b,
		[](const dd& a) { return asin(a); },
		[](const elreal& a) { return asin(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::acos",  b,
		[](const dd& a) { return acos(a); },
		[](const elreal& a) { return acos(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::atan",  g,
		[](const dd& a) { return atan(a); },
		[](const elreal& a) { return atan(a); });

	// hyperbolic
	nrOfFailedTestCases += sweep_unary<dd>("dd::sinh",  g,
		[](const dd& a) { return sinh(a); },
		[](const elreal& a) { return sinh(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::cosh",  g,
		[](const dd& a) { return cosh(a); },
		[](const elreal& a) { return cosh(a); });
	nrOfFailedTestCases += sweep_unary<dd>("dd::tanh",  g,
		[](const dd& a) { return tanh(a); },
		[](const elreal& a) { return tanh(a); });

	// pow with double exponent
	std::vector<std::pair<double, double>> pow_inputs = {
		{2.0, 3.0}, {2.0, 0.5}, {3.0, 2.5}, {0.5, 4.0}, {10.0, 0.25}
	};
	nrOfFailedTestCases += sweep_binary<dd>("dd::pow", pow_inputs,
		[](const dd& a, const dd& bb) { return pow(a, bb); },
		[](const elreal& a, const elreal& bb) { return pow(a, bb); });

	// Anchors: deterministic exact-result expectations
	nrOfFailedTestCases += anchor_check<dd>("exp(0)=1", 0.0, 1.0,
		[](const dd& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += anchor_check<dd>("log(1)=0", 1.0, 0.0,
		[](const dd& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += anchor_check<dd>("sqrt(1)=1", 1.0, 1.0,
		[](const dd& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	// Reject path: deliberately wrong oracle must fail the helper.
	{
		dd     target = exp(dd(1.0));
		elreal wrong  = elreal(2.0) * exp(elreal(1.0));
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: dd oracle helper accepted a 2x-off reference\n";
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
