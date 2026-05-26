// constant_generation.cpp: regression tests for high-precision constant
// generation with ereal.
//
// Verifies that the series/iterative generators (Machin pi, Taylor e,
// Newton-Raphson sqrt, artanh ln2) produce expansions that
//   1. project to the known double value, and
//   2. satisfy Priest's non-overlap invariant |e[i+1]| <= ulp(e[i])/2 with no
//      interior zero components,
// and that the generated constants honor the algebraic round-trip identities.
//
// REGRESSION_LEVEL convention: the generators are deterministic, so a single
// pass exercises every case at all levels.
//
// Pattern: elastic/ereal/arithmetic/addition.cpp (PR #943).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::ereal;

	// -------------------------------------------------------------------------
	// Generators (pure compute, no I/O)
	// -------------------------------------------------------------------------

	// arctan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...  (small x)
	template<unsigned nlimbs>
	ereal<nlimbs> compute_arctan_series(const ereal<nlimbs>& x, int terms) {
		ereal<nlimbs> result(0.0);
		ereal<nlimbs> x_power = x;        // x^1
		ereal<nlimbs> x_squared = x * x;
		for (int n = 0; n < terms; ++n) {
			int k = 2 * n + 1;
			double coeff = ((n % 2 == 0) ? 1.0 : -1.0) / k;
			result = result + x_power * coeff;
			x_power = x_power * x_squared;
		}
		return result;
	}

	// Machin's formula: pi/4 = 4*arctan(1/5) - arctan(1/239)
	template<unsigned nlimbs>
	ereal<nlimbs> compute_pi() {
		ereal<nlimbs> one(1.0), five(5.0), ttn(239.0), four(4.0);
		ereal<nlimbs> arctan_1_5   = compute_arctan_series(one / five, 50);
		ereal<nlimbs> arctan_1_239 = compute_arctan_series(one / ttn, 30);
		ereal<nlimbs> pi_over_4 = four * arctan_1_5 - arctan_1_239;
		return four * pi_over_4;
	}

	// Taylor series: e = sum 1/n!
	template<unsigned nlimbs>
	ereal<nlimbs> compute_e() {
		ereal<nlimbs> result(1.0);   // 1/0!
		ereal<nlimbs> term(1.0);
		for (int n = 1; n <= 50; ++n) {
			ereal<nlimbs> n_val(static_cast<double>(n));
			term = term / n_val;
			result = result + term;
			if (std::abs(double(term)) < 1.0e-100) break;
		}
		return result;
	}

	// Newton-Raphson: x_{n+1} = (x_n + v/x_n) / 2 for sqrt(v).
	template<unsigned nlimbs>
	ereal<nlimbs> compute_sqrt(double v) {
		ereal<nlimbs> x(std::sqrt(v)), v_val(v), two(2.0);
		for (int i = 0; i < 10; ++i) {
			x = (x + v_val / x) / two;
		}
		return x;
	}

	// ln(2) = 2*artanh(1/3), artanh(x) = x + x^3/3 + x^5/5 + ...
	template<unsigned nlimbs>
	ereal<nlimbs> compute_ln2() {
		ereal<nlimbs> one(1.0), three(3.0), two(2.0);
		ereal<nlimbs> x = one / three;
		ereal<nlimbs> result(0.0);
		ereal<nlimbs> x_power = x;
		ereal<nlimbs> x_squared = x * x;
		for (int n = 0; n < 50; ++n) {
			int k = 2 * n + 1;
			result = result + x_power * (1.0 / k);
			x_power = x_power * x_squared;
		}
		return two * result;
	}

	// -------------------------------------------------------------------------
	// Priest non-overlap invariant: |e[i+1]| <= ulp(e[i])/2, and no interior
	// zero components (a renormalised non-zero expansion has none).
	// -------------------------------------------------------------------------
	bool priest_nonoverlap(const ereal<19>& v) {
		const auto& L = v.limbs();
		for (std::size_t i = 0; i + 1 < L.size(); ++i) {
			if (L[i] == 0.0) return false;  // interior zero component
			double half_ulp = std::ldexp(1.0, std::ilogb(L[i]) - 53);  // ulp(e[i])/2
			if (std::abs(L[i + 1]) > half_ulp) return false;
		}
		return true;
	}

	// Relative-error check on the projected double.
	bool close_rel(const ereal<19>& x, double expected, double relTol) {
		double v = double(x);
		double diff = std::abs(v - expected);
		double scale = std::max(std::abs(v), std::abs(expected));
		return diff <= std::max(1.0e-300, relTol * scale);
	}

	// =========================================================================
	// LEVEL 1: generator value + non-overlap, and algebraic round trips
	// =========================================================================
	int VerifyConstantGeneration(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		ereal<19> pi    = compute_pi<19>();
		ereal<19> e     = compute_e<19>();
		ereal<19> ln2   = compute_ln2<19>();
		ereal<19> sqrt2 = compute_sqrt<19>(2.0);
		ereal<19> sqrt3 = compute_sqrt<19>(3.0);
		ereal<19> sqrt5 = compute_sqrt<19>(5.0);
		ereal<19> sqrt7 = compute_sqrt<19>(7.0);

		// --- Known-value projection ---
		if (reportTestCases) std::cout << "  Generated values match known doubles...\n";
		{
			struct { const char* name; ereal<19> v; double known; } cases[] = {
				{ "pi",    pi,    3.141592653589793 },
				{ "e",     e,     2.718281828459045 },
				{ "ln2",   ln2,   0.6931471805599453 },
				{ "sqrt2", sqrt2, std::sqrt(2.0) },
				{ "sqrt3", sqrt3, std::sqrt(3.0) },
				{ "sqrt5", sqrt5, std::sqrt(5.0) },
			};
			for (const auto& c : cases) {
				if (!close_rel(c.v, c.known, 1.0e-15)) {
					if (reportTestCases) std::cout << "    FAIL " << c.name << " = "
						<< double(c.v) << " expected " << c.known << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// --- Priest non-overlap invariant on every generated expansion ---
		if (reportTestCases) std::cout << "  Generated expansions satisfy Priest non-overlap...\n";
		{
			struct { const char* name; ereal<19> v; } cases[] = {
				{ "pi", pi }, { "e", e }, { "ln2", ln2 },
				{ "sqrt2", sqrt2 }, { "sqrt3", sqrt3 }, { "sqrt5", sqrt5 }, { "sqrt7", sqrt7 },
			};
			for (const auto& c : cases) {
				if (!priest_nonoverlap(c.v)) {
					if (reportTestCases) std::cout << "    FAIL " << c.name
						<< " violates non-overlap (" << c.v.limbs().size() << " limbs)\n";
					++nrOfFailedTestCases;
				}
			}
		}

		// --- Round-trip identities on generated constants ---

		// sqrt(n)^2 == n (relative, generated sqrt is many-limb accurate)
		if (reportTestCases) std::cout << "  Square-root round trip sqrt(n)^2 == n...\n";
		{
			struct { double n; ereal<19> r; } cases[] = {
				{ 2.0, sqrt2 }, { 3.0, sqrt3 }, { 5.0, sqrt5 }, { 7.0, sqrt7 },
			};
			for (const auto& c : cases) {
				double result = double(c.r * c.r);
				if (std::abs(result - c.n) / c.n > 1.0e-28) {
					if (reportTestCases) std::cout << "    FAIL sqrt(" << c.n << ")^2 = " << result << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// (a*b)/b == a
		if (reportTestCases) std::cout << "  Multiplicative round trip (a*b)/b == a...\n";
		{
			ereal<19> recovered = (pi * e) / e;
			if (std::abs(double(pi) - double(recovered)) / std::abs(double(pi)) > 1.0e-25) {
				if (reportTestCases) std::cout << "    FAIL (pi*e)/e != pi\n";
				++nrOfFailedTestCases;
			}
		}

		// (a+b)-b == a (exact)
		if (reportTestCases) std::cout << "  Additive round trip (a+b)-b == a...\n";
		{
			ereal<19> recovered = (sqrt2 + sqrt3) - sqrt3;
			if (recovered != sqrt2) {
				if (reportTestCases) std::cout << "    FAIL (sqrt2+sqrt3)-sqrt3 != sqrt2\n";
				++nrOfFailedTestCases;
			}
		}

		// (p/q)*q == p
		if (reportTestCases) std::cout << "  Rational round trip (p/q)*q == p...\n";
		{
			ereal<19> p(7.0), q(13.0);
			ereal<19> recovered = (p / q) * q;
			if (std::abs(double(p) - double(recovered)) / double(p) > 1.0e-28) {
				if (reportTestCases) std::cout << "    FAIL (7/13)*13 != 7\n";
				++nrOfFailedTestCases;
			}
		}

		// ((a+b)*c)/c == a+b
		if (reportTestCases) std::cout << "  Compound round trip ((a+b)*c)/c == a+b...\n";
		{
			ereal<19> sum = sqrt5 + sqrt7;
			ereal<19> recovered = (sum * pi) / pi;
			if (std::abs(double(sum) - double(recovered)) / std::abs(double(sum)) > 1.0e-14) {
				if (reportTestCases) std::cout << "    FAIL ((sqrt5+sqrt7)*pi)/pi != sqrt5+sqrt7\n";
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

}  // namespace

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal constant generation";
	bool        reportTestCases = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyConstantGeneration(reportTestCases), "ereal", "constant generation manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

	// Deterministic generators -- one pass covers every regression level.
	nrOfFailedTestCases += ReportTestResult(VerifyConstantGeneration(reportTestCases), "ereal", "constant generation");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& ex) {
	std::cerr << "Caught exception: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
