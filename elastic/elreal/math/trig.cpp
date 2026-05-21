// trig.cpp: tests for elreal sin/cos/tan (Phase E.6)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase E.6 scope reminder: forward trig ships with std-lib-based depth-0 +
// derivative-based depth-1 refinement. Range reduction modulo pi for
// large-magnitude arguments (Payne-Hanek) is documented in the
// elreal_impl.hpp header as future work. Tests therefore exercise
// "reasonable magnitude" inputs only, per the acceptance criteria of #892.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

static int check_close(const char* label, double got, double expected, double tol = 1e-14) {
	double diff = std::abs(got - expected);
	double mag  = std::max(std::abs(expected), 1.0);
	if (diff / mag > tol) {
		std::cerr << "FAIL: " << label << ": got " << got
			<< " expected " << expected << " (rel err " << diff / mag << ")\n";
		return 1;
	}
	return 0;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase E.6 forward trig";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	const double pi   = std::numbers::pi_v<double>;
	const double pi_2 = pi / 2.0;
	const double pi_4 = pi / 4.0;

	// --- Anchor values --------------------------------------------------
	{
		if (double(sin(elreal(0.0))) != 0.0) { std::cerr << "FAIL: sin(0) != 0\n"; ++nrOfFailedTestCases; }
		if (double(cos(elreal(0.0))) != 1.0) { std::cerr << "FAIL: cos(0) != 1\n"; ++nrOfFailedTestCases; }
		if (double(tan(elreal(0.0))) != 0.0) { std::cerr << "FAIL: tan(0) != 0\n"; ++nrOfFailedTestCases; }

		nrOfFailedTestCases += check_close("sin(pi/2) ~= 1",   double(sin(elreal(pi_2))),  1.0, 1e-15);
		nrOfFailedTestCases += check_close("cos(pi/2) ~= 0",   double(cos(elreal(pi_2))),  0.0, 1e-15);
		nrOfFailedTestCases += check_close("sin(pi) ~= 0",     double(sin(elreal(pi))),    0.0, 1e-14);
		nrOfFailedTestCases += check_close("cos(pi) ~= -1",    double(cos(elreal(pi))),   -1.0, 1e-15);
		nrOfFailedTestCases += check_close("tan(pi/4) ~= 1",   double(tan(elreal(pi_4))),  1.0, 1e-15);
	}

	// --- Pythagorean identity sin^2(x) + cos^2(x) == 1 -----------------
	{
		for (double v : {-2.0, -0.5, 0.1, 0.5, 1.0, 1.5, 3.0}) {
			elreal x(v);
			elreal s = sin(x);
			elreal c = cos(x);
			elreal sum_sq = s*s + c*c;
			nrOfFailedTestCases += check_close(
				(std::string("sin^2 + cos^2 (x=") + std::to_string(v) + ")").c_str(),
				double(sum_sq), 1.0, 1e-13);
		}
	}

	// --- tan(x) ~= sin(x) / cos(x) -------------------------------------
	{
		for (double v : {-1.0, -0.5, 0.5, 1.0, 1.2}) {
			elreal x(v);
			elreal lhs = tan(x);
			elreal rhs = sin(x) / cos(x);
			nrOfFailedTestCases += check_close(
				(std::string("tan == sin/cos (x=") + std::to_string(v) + ")").c_str(),
				double(lhs), double(rhs), 1e-13);
		}
	}

	// --- Cross-validation vs std lib for modest magnitude --------------
	// Reasonable magnitude: |x| < pi or thereabouts, where the std lib
	// internal range reduction is comfortably within double precision.
	{
		for (double v : {-pi, -1.5, -1.0, -0.5, 0.1, 0.5, 1.0, 1.5, pi}) {
			elreal x(v);
			if (double(sin(x)) != std::sin(v)) { std::cerr << "FAIL: sin(" << v << ") vs std::sin\n"; ++nrOfFailedTestCases; }
			if (double(cos(x)) != std::cos(v)) { std::cerr << "FAIL: cos(" << v << ") vs std::cos\n"; ++nrOfFailedTestCases; }
		}
		// tan: skip near singularities pi/2 + n*pi
		for (double v : {-1.0, -0.5, 0.0, 0.5, 1.0}) {
			elreal x(v);
			if (double(tan(x)) != std::tan(v)) { std::cerr << "FAIL: tan(" << v << ") vs std::tan\n"; ++nrOfFailedTestCases; }
		}
	}

	// --- Lazy-pi distinctness: sin(elreal_pi()) at full lazy precision is
	// observably closer to zero than std::sin(M_PI). std::sin(M_PI) is
	// ~1.22e-16 (because M_PI is slightly less than true pi); the depth-1
	// correction adds cos(M_PI) * (true_pi - M_PI) = -1 * 1.22e-16,
	// cancelling the std::sin error.
	//
	// NOTE: operator double() only sums *materialised* components. To make
	// the depth-1 contribution visible we have to walk the generator
	// explicitly via .at(1) first; otherwise double(s) returns depth-0
	// only and the comparison below is a no-op. (This is by design --
	// operator double() is the cheap "best estimate at current depth"
	// path; the caller decides when to spend a refinement step.)
	{
		elreal lazy_pi = elreal_pi();
		elreal s = sin(lazy_pi);
		(void)s.at(1);                     // force depth-1 materialisation
		double full  = double(s);          // now sums depth-0 + depth-1
		double naive = std::sin(pi);       // ~1.22e-16
		if (std::abs(full) >= std::abs(naive)) {
			std::cerr << "FAIL: sin(elreal_pi()) full = " << full
				<< " not closer to 0 than std::sin(M_PI) = " << naive
				<< " (lazy-pi correction did not improve the result)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- NaN / inf propagation ------------------------------------------
	{
		elreal nan_v(SpecificValue::qnan);
		if (!sin(nan_v).isnan()) { std::cerr << "FAIL: sin(NaN) != NaN\n"; ++nrOfFailedTestCases; }
		if (!cos(nan_v).isnan()) { std::cerr << "FAIL: cos(NaN) != NaN\n"; ++nrOfFailedTestCases; }
		if (!tan(nan_v).isnan()) { std::cerr << "FAIL: tan(NaN) != NaN\n"; ++nrOfFailedTestCases; }
		// sin/cos/tan of inf is NaN per IEEE-754 (no finite cycle)
		if (!sin(elreal(SpecificValue::infpos)).isnan()) {
			std::cerr << "FAIL: sin(+inf) != NaN\n"; ++nrOfFailedTestCases;
		}
	}

	// --- Round-trip with inverse trig: asin(sin(x)) ~= x for |x| < pi/2
	{
		for (double v : {-1.0, -0.5, 0.0, 0.5, 1.0}) {
			elreal x(v);
			elreal back = asin(sin(x));
			nrOfFailedTestCases += check_close(
				(std::string("asin(sin(") + std::to_string(v) + ")) ~= x").c_str(),
				double(back), v, 1e-14);
		}
	}

	// --- Depth-1 propagation on rational input ------------------------
	{
		elreal third(1LL, 3LL);
		if (sin(third).at(1) == 0.0) { std::cerr << "FAIL: sin(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		if (cos(third).at(1) == 0.0) { std::cerr << "FAIL: cos(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		if (tan(third).at(1) == 0.0) { std::cerr << "FAIL: tan(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
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
