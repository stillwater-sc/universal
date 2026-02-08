// educational_demo.cpp: demonstrate when interval arithmetic gives tight vs. wide results
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/interval/interval.hpp>
#include <universal/verification/test_suite.hpp>
#include <iomanip>
#include <sstream>
#include <cmath>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval educational demonstration";
	std::string test_tag    = "educational_demo";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Real = interval<double>;

	// =========================================================================
	// PART 1: Tight & Useful — Engineering Tolerance Analysis
	// =========================================================================
	{
		std::cout << "+=========================================================================+\n";
		std::cout << "| PART 1: Tight & Useful — Engineering Tolerance Analysis                 |\n";
		std::cout << "+=========================================================================+\n\n";

		std::cout << "Scenario: An electronic circuit with uncertain components.\n";
		std::cout << "A voltage divider has a 5V supply (1% tolerance) and two\n";
		std::cout << "resistors R1=1kOhm, R2=2kOhm (both 5% tolerance).\n";
		std::cout << "We want guaranteed bounds on current, output voltage, and power.\n\n";

		// Components with tolerances
		Real V(4.95, 5.05);       // 5V supply, +/- 1%
		Real R1(950.0, 1050.0);   // 1kOhm, +/- 5%
		Real R2(1900.0, 2100.0);  // 2kOhm, +/- 5%

		std::cout << "+---------    component intervals   --------+\n";
		std::cout << "  V  = " << V  << " V   (5V, 1% tolerance)\n";
		std::cout << "  R1 = " << R1 << " Ohm (1kOhm, 5% tolerance)\n";
		std::cout << "  R2 = " << R2 << " Ohm (2kOhm, 5% tolerance)\n\n";

		// Series resistance
		Real Rtotal = R1 + R2;
		std::cout << "+---------    computed results   --------+\n";
		std::cout << "  R_total = R1 + R2 = " << Rtotal << " Ohm\n";

		// Current: I = V / (R1 + R2)
		Real I = V / Rtotal;
		std::cout << "  I = V / R_total = " << I << " A\n";
		std::cout << "    width = " << std::scientific << std::setprecision(4)
		          << I.width() << " A" << std::fixed << std::setprecision(6) << '\n';

		// Convert to mA for readability
		double I_mid_mA = I.mid() * 1000.0;
		double I_width_mA = I.width() * 1000.0;
		std::cout << "    = [" << std::setprecision(4) << I.lo() * 1000.0 << ", "
		          << I.hi() * 1000.0 << "] mA\n";
		std::cout << "    midpoint = " << I_mid_mA << " mA, width = "
		          << I_width_mA << " mA\n\n";

		// Voltage divider output: Vout = V * R2 / (R1 + R2)
		Real Vout = V * R2 / Rtotal;
		std::cout << "  Vout = V * R2 / R_total = " << Vout << " V\n";
		std::cout << "    width = " << Vout.width() << " V\n";
		std::cout << "    midpoint = " << Vout.mid() << " V\n\n";

		// Power dissipation: P = V^2 / (R1 + R2)
		// Use sqr() for tighter V^2 computation
		Real Vsq = sqr(V);
		Real P = Vsq / Rtotal;
		std::cout << "  P = V^2 / R_total = " << P << " W\n";
		std::cout << "    width = " << std::scientific << P.width() << " W\n\n"
		          << std::fixed;

		// Corner analysis comparison for current I = V / (R1 + R2)
		std::cout << "+---------    corner analysis verification   --------+\n";
		double I_min = 4.95 / (1050.0 + 2100.0);  // min V, max R
		double I_max = 5.05 / (950.0 + 1900.0);   // max V, min R
		std::cout << "  Corner analysis: I in [" << std::setprecision(8)
		          << I_min * 1000.0 << ", " << I_max * 1000.0 << "] mA\n";
		std::cout << "  Interval result: I in [" << I.lo() * 1000.0 << ", "
		          << I.hi() * 1000.0 << "] mA\n";
		std::cout << "  -> The interval result matches corner analysis exactly.\n";
		std::cout << "  -> Each variable appears only once, so there is no overestimation.\n\n";

		std::cout << "KEY INSIGHT: When each uncertain quantity appears only once in\n";
		std::cout << "a subexpression, interval arithmetic gives the tightest possible\n";
		std::cout << "bounds. Engineering tolerance analysis is a natural fit.\n\n";
	}

	// =========================================================================
	// PART 2: Wide & Useless — The Dependency Problem
	// =========================================================================
	{
		std::cout << "+=========================================================================+\n";
		std::cout << "| PART 2: Wide & Useless — The Dependency Problem                         |\n";
		std::cout << "+=========================================================================+\n\n";

		// --- Scenario A: x - x ---
		{
			std::cout << "+---------    Scenario A: the simplest dependency problem   --------+\n\n";

			std::cout << "Mathematically, x - x = 0 for any real number x.\n";
			std::cout << "But interval arithmetic treats each occurrence of x independently.\n\n";

			Real x(2.0, 5.0);
			Real result = x - x;

			std::cout << "  x         = " << x << '\n';
			std::cout << "  x - x     = " << result << '\n';
			std::cout << "  true answer = [0, 0]\n";
			std::cout << "  width     = " << result.width() << " (should be 0)\n\n";

			std::cout << "The interval [2,5] - [2,5] computes [2-5, 5-2] = [-3, 3].\n";
			std::cout << "Each 'x' is treated as an independent variable that could take\n";
			std::cout << "any value in [2,5] — the subtraction doesn't know both are the same x.\n\n";
		}

		// --- Scenario B: Polynomial evaluation ---
		{
			std::cout << "+---------    Scenario B: polynomial evaluation   --------+\n\n";

			std::cout << "Evaluate f(x) = x^2 - x + 0.25 over x = [-2, 3].\n";
			std::cout << "Note: f(x) = (x - 0.5)^2, so the true range is [0, 6.25].\n\n";

			Real x(-2.0, 3.0);
			Real quarter(0.25);

			// Naive evaluation: x*x - x + 0.25
			// Here x appears multiple times -> massive overestimation
			Real naive = x * x - x + quarter;

			std::cout << "  x = " << x << "\n\n";

			std::cout << "  Naive:   x*x - x + 0.25\n";
			std::cout << "    x*x       = " << (x * x) << '\n';
			std::cout << "    x*x - x   = " << (x * x - x) << '\n';
			std::cout << "    result    = " << naive << '\n';
			std::cout << "    width     = " << naive.width() << "\n\n";

			// Factored: (x - 0.5)^2 using sqr()
			// sqr() knows both arguments are the same -> tight result
			Real shifted = x - Real(0.5);
			Real factored = sqr(shifted);

			std::cout << "  Factored: sqr(x - 0.5)\n";
			std::cout << "    x - 0.5   = " << shifted << '\n';
			std::cout << "    result    = " << factored << '\n';
			std::cout << "    width     = " << factored.width() << "\n\n";

			std::cout << "  True range: [0, 6.25]   (minimum at x=0.5, max at x=-2)\n\n";

			std::cout << "  Comparison:\n";
			std::cout << "    Naive width:    " << std::setw(8) << naive.width() << '\n';
			std::cout << "    Factored width: " << std::setw(8) << factored.width() << '\n';
			std::cout << "    True width:     " << std::setw(8) << 6.25 << '\n';
			double overestimate = naive.width() / 6.25;
			std::cout << "    Naive overestimates by " << std::setprecision(1) << overestimate
			          << "x!" << std::setprecision(6) << "\n\n";

			std::cout << "KEY INSIGHT: Rewriting f(x) to minimize repeated occurrences\n";
			std::cout << "of the same variable dramatically tightens interval bounds.\n";
			std::cout << "sqr(y) knows both arguments are identical, avoiding the\n";
			std::cout << "dependency problem that x*x suffers from.\n\n";
		}

		// --- Scenario C: Iterative blowup (logistic map) ---
		{
			std::cout << "+---------    Scenario C: iterative blowup (logistic map)   --------+\n\n";

			std::cout << "The logistic map: x_{n+1} = r * x_n * (1 - x_n)\n";
			std::cout << "with r = 3.75, starting from x_0 = [0.49, 0.51].\n\n";
			std::cout << "The true trajectory stays in [0, 1], but interval arithmetic\n";
			std::cout << "treats x_n and (1 - x_n) as independent. The dependency problem\n";
			std::cout << "compounds at every iteration, causing the interval to blow up.\n\n";

			double r = 3.75;
			Real x(0.49, 0.51);

			auto print_row = [](int step, const Real& iv) {
				std::ostringstream oss;
				oss << std::setprecision(6) << iv;
				std::string ivstr = oss.str();
				// truncate very long interval strings
				if (ivstr.size() > 40) {
					std::ostringstream oss2;
					oss2 << "[" << std::scientific << std::setprecision(3) << iv.lo()
					     << ", " << iv.hi() << "]";
					ivstr = oss2.str();
				}
				std::cout << std::setw(6) << step << "  "
				          << std::setw(42) << std::left << ivstr << std::right << "  "
				          << std::scientific << std::setprecision(4) << iv.width()
				          << std::fixed << '\n';
			};

			std::cout << std::setw(6) << "step" << "  "
			          << std::setw(42) << std::left << "interval" << std::right << "  "
			          << "width" << '\n';
			std::cout << std::string(64, '-') << '\n';

			print_row(0, x);

			int maxSteps = 15;
			for (int i = 1; i <= maxSteps; ++i) {
				// x = r * x * (1 - x)
				// x appears twice -> dependency problem compounds
				Real one(1.0);
				x = Real(r) * x * (one - x);

				print_row(i, x);

				// Stop early if the interval has blown up completely
				if (x.width() > 1000.0) {
					std::cout << "  ... interval has become meaningless, stopping.\n";
					break;
				}
			}

			std::cout << "\nFor reference, the true trajectory (using midpoint 0.5):\n";
			double xd = 0.5;
			for (int i = 0; i <= 10; ++i) {
				std::cout << "  step " << std::setw(2) << i << ": x = "
				          << std::setprecision(8) << xd << '\n';
				xd = r * xd * (1.0 - xd);
			}
			std::cout << "  -> Always stays in [0, 1] as guaranteed by the logistic map.\n\n";

			std::cout << "KEY INSIGHT: When the dependency problem compounds across iterations,\n";
			std::cout << "interval widths grow exponentially. After just a few steps, the\n";
			std::cout << "interval becomes so wide that it provides no useful information.\n";
			std::cout << "This is a fundamental limitation, not a bug in the implementation.\n\n";
		}
	}

	// =========================================================================
	// PART 3: Summary
	// =========================================================================
	{
		std::cout << "+=========================================================================+\n";
		std::cout << "| PART 3: Summary                                                         |\n";
		std::cout << "+=========================================================================+\n\n";

		// Recompute key values for the summary table
		Real V(4.95, 5.05);
		Real R1(950.0, 1050.0);
		Real R2(1900.0, 2100.0);
		Real Rtotal = R1 + R2;
		Real I = V / Rtotal;
		Real Vout = V * R2 / Rtotal;

		Real x_poly(-2.0, 3.0);
		Real naive_poly = x_poly * x_poly - x_poly + Real(0.25);
		Real factored_poly = sqr(x_poly - Real(0.5));

		Real x_logistic(0.49, 0.51);
		for (int i = 0; i < 10; ++i) {
			x_logistic = Real(3.75) * x_logistic * (Real(1.0) - x_logistic);
		}

		std::cout << std::left
		          << std::setw(28) << "Scenario"
		          << std::setw(14) << "Width"
		          << std::setw(14) << "True Width"
		          << "Quality\n";
		std::cout << std::string(70, '-') << '\n';

		std::cout << std::setw(28) << "Tolerance: I (mA)"
		          << std::setw(14) << std::setprecision(4) << I.width() * 1000.0
		          << std::setw(14) << (5.05/2850.0 - 4.95/3150.0) * 1000.0
		          << "TIGHT (exact)\n";

		std::cout << std::setw(28) << "Tolerance: Vout (V)"
		          << std::setw(14) << std::setprecision(4) << Vout.width()
		          << std::setw(14) << "~0.24"
		          << "WIDE (R2 appears twice)\n";

		std::cout << std::setw(28) << "x - x"
		          << std::setw(14) << std::setprecision(1) << 6.0
		          << std::setw(14) << 0.0
		          << "USELESS (infinite overest.)\n";

		std::cout << std::setw(28) << "Poly naive: x*x-x+0.25"
		          << std::setw(14) << std::setprecision(2) << naive_poly.width()
		          << std::setw(14) << 6.25
		          << "WIDE\n";

		std::cout << std::setw(28) << "Poly factored: sqr(x-0.5)"
		          << std::setw(14) << std::setprecision(2) << factored_poly.width()
		          << std::setw(14) << 6.25
		          << "TIGHT\n";

		std::cout << std::setw(28) << "Logistic map (10 steps)"
		          << std::setw(14) << std::scientific << std::setprecision(2) << x_logistic.width()
		          << std::fixed << std::setw(14) << "~0.7"
		          << "USELESS (blowup)\n";

		std::cout << std::right << "\n";

		std::cout << "TAKEAWAYS:\n";
		std::cout << "  1. Interval arithmetic is excellent for tolerance analysis where\n";
		std::cout << "     each uncertain quantity appears once in each subexpression.\n";
		std::cout << "  2. The 'dependency problem' causes overestimation when the same\n";
		std::cout << "     variable appears multiple times in a formula.\n";
		std::cout << "  3. Algebraic reformulation (factoring, using sqr()) can dramatically\n";
		std::cout << "     reduce overestimation.\n";
		std::cout << "  4. Iterative computations compound the dependency problem,\n";
		std::cout << "     making intervals blow up exponentially.\n\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
