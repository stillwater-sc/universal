// inverse_trig.cpp: tests for elreal asin/acos/atan/atan2 (Phase E.5)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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

	std::string test_suite = "elreal Phase E.5 inverse trig";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	const double pi    = std::numbers::pi_v<double>;
	const double pi_2  = pi / 2.0;
	const double pi_4  = pi / 4.0;

	// --- Anchor values --------------------------------------------------
	{
		if (double(atan(elreal(0.0))) != 0.0) { std::cerr << "FAIL: atan(0) != 0\n"; ++nrOfFailedTestCases; }
		nrOfFailedTestCases += check_close("atan(1) == pi/4", double(atan(elreal(1.0))), pi_4);

		if (double(asin(elreal(0.0))) != 0.0) { std::cerr << "FAIL: asin(0) != 0\n"; ++nrOfFailedTestCases; }
		nrOfFailedTestCases += check_close("asin(1) == pi/2",  double(asin(elreal( 1.0))),  pi_2);
		nrOfFailedTestCases += check_close("asin(-1) == -pi/2",double(asin(elreal(-1.0))), -pi_2);

		nrOfFailedTestCases += check_close("acos(1) == 0",     double(acos(elreal( 1.0))), 0.0);
		nrOfFailedTestCases += check_close("acos(0) == pi/2",  double(acos(elreal( 0.0))), pi_2);
		nrOfFailedTestCases += check_close("acos(-1) == pi",   double(acos(elreal(-1.0))), pi);
	}

	// --- atan2 four-quadrant -----------------------------------------
	{
		nrOfFailedTestCases += check_close("atan2(0, 1) == 0",      double(atan2(elreal( 0.0), elreal( 1.0))),  0.0);
		nrOfFailedTestCases += check_close("atan2(1, 0) == pi/2",   double(atan2(elreal( 1.0), elreal( 0.0))),  pi_2);
		nrOfFailedTestCases += check_close("atan2(0, -1) == pi",    double(atan2(elreal( 0.0), elreal(-1.0))),  pi);
		nrOfFailedTestCases += check_close("atan2(-1, 0) == -pi/2", double(atan2(elreal(-1.0), elreal( 0.0))), -pi_2);
		nrOfFailedTestCases += check_close("atan2(1, 1) == pi/4",   double(atan2(elreal( 1.0), elreal( 1.0))),  pi_4);
		nrOfFailedTestCases += check_close("atan2(1, -1) == 3pi/4", double(atan2(elreal( 1.0), elreal(-1.0))),  3.0 * pi_4);
	}

	// --- Cross-validation against std lib at depth 0 -----------------
	{
		for (double v : {-2.0, -1.5, -0.5, 0.1, 0.5, 1.5, 2.0, 100.0}) {
			if (double(atan(elreal(v))) != std::atan(v)) {
				std::cerr << "FAIL: atan(" << v << ") vs std::atan\n"; ++nrOfFailedTestCases;
			}
		}
		for (double v : {-0.99, -0.5, -0.1, 0.1, 0.5, 0.99}) {
			if (double(asin(elreal(v))) != std::asin(v)) {
				std::cerr << "FAIL: asin(" << v << ") vs std::asin\n"; ++nrOfFailedTestCases;
			}
			if (double(acos(elreal(v))) != std::acos(v)) {
				std::cerr << "FAIL: acos(" << v << ") vs std::acos\n"; ++nrOfFailedTestCases;
			}
		}
	}

	// --- asin/acos at |x| > 1 -> NaN ----------------------------------
	{
		if (!asin(elreal(2.0)).isnan()) {
			std::cerr << "FAIL: asin(2) != NaN\n"; ++nrOfFailedTestCases;
		}
		if (!acos(elreal(-1.5)).isnan()) {
			std::cerr << "FAIL: acos(-1.5) != NaN\n"; ++nrOfFailedTestCases;
		}
	}

	// --- atan(+/-inf) = +/-pi/2 --------------------------------------
	{
		nrOfFailedTestCases += check_close("atan(+inf) == pi/2",
			double(atan(elreal(SpecificValue::infpos))), pi_2);
		nrOfFailedTestCases += check_close("atan(-inf) == -pi/2",
			double(atan(elreal(SpecificValue::infneg))), -pi_2);
	}

	// --- Round-trip identities ---------------------------------------
	{
		for (double v : {-0.7, -0.3, 0.0, 0.3, 0.7}) {
			elreal x(v);
			// sin(asin(x)) == x. But we don't have sin yet (Phase E.6).
			// Instead verify via the inverse identity using tan.
			// tan(atan(x)) is NOT yet available either; use the trivial
			// asin/acos relationship: asin(x) + acos(x) == pi/2.
			elreal sum_id = asin(x) + acos(x);
			nrOfFailedTestCases += check_close(
				(std::string("asin(") + std::to_string(v) + ") + acos = pi/2").c_str(),
				double(sum_id), pi_2, 1e-13);
		}
	}

	// --- atan2(y, x) consistency with atan(y/x) for x > 0 -----------
	{
		for (auto p : { std::pair{1.0, 2.0}, std::pair{-3.0, 1.0}, std::pair{0.5, 4.0} }) {
			elreal y(p.first), x(p.second);
			elreal direct = atan2(y, x);
			elreal via_atan = atan(y / x);
			nrOfFailedTestCases += check_close("atan2(y, x>0) == atan(y/x)",
				double(direct), double(via_atan), 1e-14);
		}
	}

	// --- NaN propagation --------------------------------------------
	{
		elreal nan_v(SpecificValue::qnan);
		if (!asin(nan_v).isnan())  { std::cerr << "FAIL: asin(NaN) != NaN\n";  ++nrOfFailedTestCases; }
		if (!acos(nan_v).isnan())  { std::cerr << "FAIL: acos(NaN) != NaN\n";  ++nrOfFailedTestCases; }
		if (!atan(nan_v).isnan())  { std::cerr << "FAIL: atan(NaN) != NaN\n";  ++nrOfFailedTestCases; }
		if (!atan2(nan_v, elreal(1.0)).isnan()) {
			std::cerr << "FAIL: atan2(NaN, 1) != NaN\n"; ++nrOfFailedTestCases;
		}
	}

	// --- Depth-1 propagation on rational input ----------------------
	{
		elreal third(1LL, 3LL);
		if (asin(third).at(1) == 0.0) { std::cerr << "FAIL: asin(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		if (acos(third).at(1) == 0.0) { std::cerr << "FAIL: acos(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		if (atan(third).at(1) == 0.0) { std::cerr << "FAIL: atan(1/3).at(1) == 0\n"; ++nrOfFailedTestCases; }
		elreal half(1LL, 2LL);
		if (atan2(third, half).at(1) == 0.0) {
			std::cerr << "FAIL: atan2(1/3, 1/2).at(1) == 0 (generator not propagating)\n";
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
