// qd_cascade_math_sweep.cpp: cross-validate qd_cascade math suite against elreal (Phase J, #904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include "math_sweep_common.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::elreal_oracle_sweep;

	std::string test_suite = "elreal Phase J: qd_cascade math oracle sweep";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	const auto& g = general_inputs();
	const auto& p = positive_inputs();
	const auto& b = bounded_inputs();

	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::sqrt", p,
		[](const qd_cascade& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::exp",   g,
		[](const qd_cascade& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::exp2",  g,
		[](const qd_cascade& a) { return exp2(a); },
		[](const elreal& a) { return exp2(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::expm1", g,
		[](const qd_cascade& a) { return expm1(a); },
		[](const elreal& a) { return expm1(a); });

	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::log",   p,
		[](const qd_cascade& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::log2",  p,
		[](const qd_cascade& a) { return log2(a); },
		[](const elreal& a) { return log2(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::log10", p,
		[](const qd_cascade& a) { return log10(a); },
		[](const elreal& a) { return log10(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::log1p", log1p_inputs(),
		[](const qd_cascade& a) { return log1p(a); },
		[](const elreal& a) { return log1p(a); });

	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::sin", g,
		[](const qd_cascade& a) { return sin(a); },
		[](const elreal& a) { return sin(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::cos", g,
		[](const qd_cascade& a) { return cos(a); },
		[](const elreal& a) { return cos(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::tan", g,
		[](const qd_cascade& a) { return tan(a); },
		[](const elreal& a) { return tan(a); });

	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::asin", b,
		[](const qd_cascade& a) { return asin(a); },
		[](const elreal& a) { return asin(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::acos", b,
		[](const qd_cascade& a) { return acos(a); },
		[](const elreal& a) { return acos(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::atan", g,
		[](const qd_cascade& a) { return atan(a); },
		[](const elreal& a) { return atan(a); });

	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::sinh", g,
		[](const qd_cascade& a) { return sinh(a); },
		[](const elreal& a) { return sinh(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::cosh", g,
		[](const qd_cascade& a) { return cosh(a); },
		[](const elreal& a) { return cosh(a); });
	nrOfFailedTestCases += sweep_unary<qd_cascade>("qd_cascade::tanh", g,
		[](const qd_cascade& a) { return tanh(a); },
		[](const elreal& a) { return tanh(a); });

	std::vector<std::pair<double, double>> pow_inputs = {
		{2.0, 3.0}, {2.0, 0.5}, {3.0, 2.5}, {0.5, 4.0}, {10.0, 0.25}
	};
	nrOfFailedTestCases += sweep_binary<qd_cascade>("qd_cascade::pow", pow_inputs,
		[](const qd_cascade& a, const qd_cascade& bb) { return pow(a, bb); },
		[](const elreal& a, const elreal& bb) { return pow(a, bb); });

	nrOfFailedTestCases += anchor_check<qd_cascade>("exp(0)=1", 0.0, 1.0,
		[](const qd_cascade& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += anchor_check<qd_cascade>("log(1)=0", 1.0, 0.0,
		[](const qd_cascade& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += anchor_check<qd_cascade>("sqrt(1)=1", 1.0, 1.0,
		[](const qd_cascade& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	{
		qd_cascade target = exp(qd_cascade(1.0));
		elreal     wrong  = elreal(2.0) * exp(elreal(1.0));
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: qd_cascade oracle helper accepted a 2x-off reference\n";
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
