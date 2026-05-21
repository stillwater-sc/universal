// qd_math_sweep.cpp: cross-validate qd math suite against elreal (Phase J, #904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Companion of dd_math_sweep.cpp for the qd (quad-double) type. See
// dd_math_sweep.cpp for the design rationale and the precision-ceiling
// caveat that applies to all multi-component oracle sweeps today.

#include <universal/utility/directives.hpp>

#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>
#include "math_sweep_common.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::elreal_oracle_sweep;

	std::string test_suite = "elreal Phase J: qd math oracle sweep";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	const auto& g = general_inputs();
	const auto& p = positive_inputs();
	const auto& b = bounded_inputs();

	// sqrt
	nrOfFailedTestCases += sweep_unary<qd>("qd::sqrt", p,
		[](const qd& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	// exp family
	nrOfFailedTestCases += sweep_unary<qd>("qd::exp",   g,
		[](const qd& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::exp2",  g,
		[](const qd& a) { return exp2(a); },
		[](const elreal& a) { return exp2(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::expm1", g,
		[](const qd& a) { return expm1(a); },
		[](const elreal& a) { return expm1(a); });

	// log family
	nrOfFailedTestCases += sweep_unary<qd>("qd::log",   p,
		[](const qd& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::log2",  p,
		[](const qd& a) { return log2(a); },
		[](const elreal& a) { return log2(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::log10", p,
		[](const qd& a) { return log10(a); },
		[](const elreal& a) { return log10(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::log1p", log1p_inputs(),
		[](const qd& a) { return log1p(a); },
		[](const elreal& a) { return log1p(a); });

	// trig
	nrOfFailedTestCases += sweep_unary<qd>("qd::sin", g,
		[](const qd& a) { return sin(a); },
		[](const elreal& a) { return sin(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::cos", g,
		[](const qd& a) { return cos(a); },
		[](const elreal& a) { return cos(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::tan", g,
		[](const qd& a) { return tan(a); },
		[](const elreal& a) { return tan(a); });

	// inverse trig
	nrOfFailedTestCases += sweep_unary<qd>("qd::asin", b,
		[](const qd& a) { return asin(a); },
		[](const elreal& a) { return asin(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::acos", b,
		[](const qd& a) { return acos(a); },
		[](const elreal& a) { return acos(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::atan", g,
		[](const qd& a) { return atan(a); },
		[](const elreal& a) { return atan(a); });

	// hyperbolic
	nrOfFailedTestCases += sweep_unary<qd>("qd::sinh", g,
		[](const qd& a) { return sinh(a); },
		[](const elreal& a) { return sinh(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::cosh", g,
		[](const qd& a) { return cosh(a); },
		[](const elreal& a) { return cosh(a); });
	nrOfFailedTestCases += sweep_unary<qd>("qd::tanh", g,
		[](const qd& a) { return tanh(a); },
		[](const elreal& a) { return tanh(a); });

	// pow
	std::vector<std::pair<double, double>> pow_inputs = {
		{2.0, 3.0}, {2.0, 0.5}, {3.0, 2.5}, {0.5, 4.0}, {10.0, 0.25}
	};
	nrOfFailedTestCases += sweep_binary<qd>("qd::pow", pow_inputs,
		[](const qd& a, const qd& bb) { return pow(a, bb); },
		[](const elreal& a, const elreal& bb) { return pow(a, bb); });

	// Anchors
	nrOfFailedTestCases += anchor_check<qd>("exp(0)=1", 0.0, 1.0,
		[](const qd& a) { return exp(a); },
		[](const elreal& a) { return exp(a); });
	nrOfFailedTestCases += anchor_check<qd>("log(1)=0", 1.0, 0.0,
		[](const qd& a) { return log(a); },
		[](const elreal& a) { return log(a); });
	nrOfFailedTestCases += anchor_check<qd>("sqrt(1)=1", 1.0, 1.0,
		[](const qd& a) { return sqrt(a); },
		[](const elreal& a) { return sqrt(a); });

	// Reject path
	{
		qd     target = exp(qd(1.0));
		elreal wrong  = elreal(2.0) * exp(elreal(1.0));
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: qd oracle helper accepted a 2x-off reference\n";
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
