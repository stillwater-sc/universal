// dd_cascade_math_sweep.cpp: cross-validate dd_cascade math suite against elreal (Phase J, #904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Companion of dd_math_sweep.cpp for the dd_cascade type (the
// floatcascade<2>-based double-double).

#include <universal/utility/directives.hpp>

#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include "math_sweep_common.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::elreal_oracle_sweep;

	std::string test_suite = "elreal Phase J: dd_cascade math oracle sweep";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	const auto& g = general_inputs();
	const auto& p = positive_inputs();
	const auto& b = bounded_inputs();

	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::sqrt", p,
		[](const dd_cascade& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::exp",   g,
		[](const dd_cascade& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::exp2",  g,
		[](const dd_cascade& a) { return exp2(a); },
		[](const elreal& a) { return exp2(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::expm1", g,
		[](const dd_cascade& a) { return expm1(a); },
		[](const elreal& a) { return expm1(a); });

	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::log",   p,
		[](const dd_cascade& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::log2",  p,
		[](const dd_cascade& a) { return log2(a); },
		[](const elreal& a) { return log2(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::log10", p,
		[](const dd_cascade& a) { return log10(a); },
		[](const elreal& a) { return log10(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::log1p", log1p_inputs(),
		[](const dd_cascade& a) { return log1p(a); },
		[](const elreal& a) { return log1p(a); });

	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::sin", g,
		[](const dd_cascade& a) { return sin(a); },
		[](const elreal& a) { return sin(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::cos", g,
		[](const dd_cascade& a) { return cos(a); },
		[](const elreal& a) { return cos(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::tan", g,
		[](const dd_cascade& a) { return tan(a); },
		[](const elreal& a) { return tan(a); });

	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::asin", b,
		[](const dd_cascade& a) { return asin(a); },
		[](const elreal& a) { return asin(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::acos", b,
		[](const dd_cascade& a) { return acos(a); },
		[](const elreal& a) { return acos(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::atan", g,
		[](const dd_cascade& a) { return atan(a); },
		[](const elreal& a) { return atan(a); });

	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::sinh", g,
		[](const dd_cascade& a) { return sinh(a); },
		[](const elreal& a) { return sinh(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::cosh", g,
		[](const dd_cascade& a) { return cosh(a); },
		[](const elreal& a) { return cosh(a); });
	nrOfFailedTestCases += sweep_unary<dd_cascade>("dd_cascade::tanh", g,
		[](const dd_cascade& a) { return tanh(a); },
		[](const elreal& a) { return tanh(a); });

	std::vector<std::pair<double, double>> pow_inputs = {
		{2.0, 3.0}, {2.0, 0.5}, {3.0, 2.5}, {0.5, 4.0}, {10.0, 0.25}
	};
	nrOfFailedTestCases += sweep_binary<dd_cascade>("dd_cascade::pow", pow_inputs,
		[](const dd_cascade& a, const dd_cascade& bb) { return pow(a, bb); },
		[](const elreal& a, const elreal& bb) { return pow(a, bb); });

	nrOfFailedTestCases += anchor_check<dd_cascade>("exp(0)=1", 0.0, 1.0,
		[](const dd_cascade& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += anchor_check<dd_cascade>("log(1)=0", 1.0, 0.0,
		[](const dd_cascade& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += anchor_check<dd_cascade>("sqrt(1)=1", 1.0, 1.0,
		[](const dd_cascade& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	{
		dd_cascade target = exp(dd_cascade(1.0));
		elreal     wrong  = elreal(2.0) * exp(elreal(1.0));
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: dd_cascade oracle helper accepted a 2x-off reference\n";
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
