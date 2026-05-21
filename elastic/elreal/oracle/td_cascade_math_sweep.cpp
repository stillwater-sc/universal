// td_cascade_math_sweep.cpp: cross-validate td_cascade math suite against elreal (Phase J, #904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include "math_sweep_common.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::elreal_oracle_sweep;

	std::string test_suite = "elreal Phase J: td_cascade math oracle sweep";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// td_cascade exposes a smaller math suite than dd / qd / dd_cascade /
	// qd_cascade today (no exp2 / exp10 / expm1 / log2 / log10 / log1p),
	// so this sweep is correspondingly narrower.
	const auto& g = general_inputs();
	const auto& p = positive_inputs();
	const auto& b = bounded_inputs();

	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::sqrt", p,
		[](const td_cascade& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::exp", g,
		[](const td_cascade& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::log", p,
		[](const td_cascade& a) { return log(a); },
		[](const elreal& a) { return log(a); });

	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::sin", g,
		[](const td_cascade& a) { return sin(a); },
		[](const elreal& a) { return sin(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::cos", g,
		[](const td_cascade& a) { return cos(a); },
		[](const elreal& a) { return cos(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::tan", g,
		[](const td_cascade& a) { return tan(a); },
		[](const elreal& a) { return tan(a); });

	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::asin", b,
		[](const td_cascade& a) { return asin(a); },
		[](const elreal& a) { return asin(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::acos", b,
		[](const td_cascade& a) { return acos(a); },
		[](const elreal& a) { return acos(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::atan", g,
		[](const td_cascade& a) { return atan(a); },
		[](const elreal& a) { return atan(a); });

	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::sinh", g,
		[](const td_cascade& a) { return sinh(a); },
		[](const elreal& a) { return sinh(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::cosh", g,
		[](const td_cascade& a) { return cosh(a); },
		[](const elreal& a) { return cosh(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::tanh", g,
		[](const td_cascade& a) { return tanh(a); },
		[](const elreal& a) { return tanh(a); });

	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::asinh", g,
		[](const td_cascade& a) { return asinh(a); },
		[](const elreal& a) { return asinh(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::acosh", acosh_inputs(),
		[](const td_cascade& a) { return acosh(a); },
		[](const elreal& a) { return acosh(a); });
	nrOfFailedTestCases += sweep_unary<td_cascade>("td_cascade::atanh", b,
		[](const td_cascade& a) { return atanh(a); },
		[](const elreal& a) { return atanh(a); });

	std::vector<std::pair<double, double>> pow_inputs = {
		{2.0, 3.0}, {2.0, 0.5}, {3.0, 2.5}, {0.5, 4.0}, {10.0, 0.25}
	};
	nrOfFailedTestCases += sweep_binary<td_cascade>("td_cascade::pow", pow_inputs,
		[](const td_cascade& a, const td_cascade& bb) { return pow(a, bb); },
		[](const elreal& a, const elreal& bb) { return pow(a, bb); });

	nrOfFailedTestCases += anchor_check<td_cascade>("exp(0)=1", 0.0, 1.0,
		[](const td_cascade& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += anchor_check<td_cascade>("sqrt(1)=1", 1.0, 1.0,
		[](const td_cascade& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	{
		td_cascade target = exp(td_cascade(1.0));
		elreal     wrong  = elreal(2.0) * exp(elreal(1.0));
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: td_cascade oracle helper accepted a 2x-off reference\n";
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
