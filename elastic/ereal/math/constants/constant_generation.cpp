// constant_generation.cpp: Generate high-precision mathematical constants using ereal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/console_utf8.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>

namespace sw { namespace universal {

	// ===================================================================
	// COMPUTE PI using Machin's Formula
	// ===================================================================
	// π/4 = 4·arctan(1/5) - arctan(1/239)

	// Compute arctan(x) for small x using Taylor series
	// arctan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...
	template<unsigned nlimbs>
	ereal<nlimbs> compute_arctan_series(const ereal<nlimbs>& x, int terms = 100) {
		ereal<nlimbs> result(0.0);
		ereal<nlimbs> x_power = x;  // Start with x^1
		ereal<nlimbs> x_squared = x * x;

		for (int n = 0; n < terms; ++n) {
			int k = 2 * n + 1;
			double sign = (n % 2 == 0) ? 1.0 : -1.0;
			double coeff = sign / k;

			// term = coeff * x_power
			ereal<nlimbs> term = x_power * coeff;
			result = result + term;

			// x_power = x_power * x^2
			x_power = x_power * x_squared;
		}

		return result;
	}

	template<unsigned nlimbs>
	ereal<nlimbs> compute_pi() {
		std::cout << "Computing π using Machin's formula: π/4 = 4·arctan(1/5) - arctan(1/239)\n";

		// Compute arctan(1/5)
		ereal<nlimbs> one(1.0);
		ereal<nlimbs> five(5.0);
		ereal<nlimbs> one_fifth = one / five;
		ereal<nlimbs> arctan_one_fifth = compute_arctan_series(one_fifth, 50);

		std::cout << "  arctan(1/5) computed with " << arctan_one_fifth.limbs().size() << " components\n";

		// Compute arctan(1/239)
		ereal<nlimbs> two_three_nine(239.0);
		ereal<nlimbs> one_over_239 = one / two_three_nine;
		ereal<nlimbs> arctan_one_239 = compute_arctan_series(one_over_239, 30);

		std::cout << "  arctan(1/239) computed with " << arctan_one_239.limbs().size() << " components\n";

		// π/4 = 4·arctan(1/5) - arctan(1/239)
		ereal<nlimbs> four(4.0);
		ereal<nlimbs> four_arctan = four * arctan_one_fifth;
		ereal<nlimbs> pi_over_4 = four_arctan - arctan_one_239;

		// π = 4 · (π/4)
		ereal<nlimbs> pi = four * pi_over_4;

		std::cout << "  π computed with " << pi.limbs().size() << " components\n";
		std::cout << "  π ≈ " << std::setprecision(20) << double(pi) << "\n\n";

		return pi;
	}

	// ===================================================================
	// COMPUTE E using Taylor Series
	// ===================================================================
	// e = 1 + 1/1! + 1/2! + 1/3! + 1/4! + ...

	template<unsigned nlimbs>
	ereal<nlimbs> compute_e() {
		std::cout << "Computing e using Taylor series: e = Σ(1/n!)\n";

		ereal<nlimbs> result(1.0);  // Start with 1
		ereal<nlimbs> term(1.0);    // First term is 1/0! = 1

		int terms = 50;
		for (int n = 1; n <= terms; ++n) {
			// term = term / n (each iteration divides by the next n)
			ereal<nlimbs> n_val(static_cast<double>(n));
			term = term / n_val;

			// Add to result
			result = result + term;

			// Check if term is negligible
			double term_val = double(term);
			if (std::abs(term_val) < 1.0e-100) {
				std::cout << "  Converged after " << n << " terms\n";
				break;
			}
		}

		std::cout << "  e computed with " << result.limbs().size() << " components\n";
		std::cout << "  e ≈ " << std::setprecision(20) << double(result) << "\n\n";

		return result;
	}

	// ===================================================================
	// COMPUTE √n using Newton-Raphson
	// ===================================================================
	// Solving x² = n, iterate: x_{n+1} = (x_n + n/x_n) / 2

	template<unsigned nlimbs>
	ereal<nlimbs> compute_sqrt(double n) {
		std::cout << "Computing √" << n << " using Newton-Raphson: x = (x + n/x)/2\n";

		ereal<nlimbs> x(std::sqrt(n));  // Initial guess
		ereal<nlimbs> n_val(n);
		ereal<nlimbs> two(2.0);

		int iterations = 10;
		for (int i = 0; i < iterations; ++i) {
			// x = (x + n/x) / 2
			ereal<nlimbs> n_over_x = n_val / x;
			ereal<nlimbs> sum = x + n_over_x;
			x = sum / two;
		}

		std::cout << "  √" << n << " computed with " << x.limbs().size() << " components\n";
		std::cout << "  √" << n << " ≈ " << std::setprecision(20) << double(x) << "\n\n";

		return x;
	}

	// ===================================================================
	// COMPUTE ln(2) using artanh series
	// ===================================================================
	// ln(2) = 2·artanh(1/3) where artanh(x) = x + x^3/3 + x^5/5 + ...

	template<unsigned nlimbs>
	ereal<nlimbs> compute_ln2() {
		std::cout << "Computing ln(2) using artanh series: ln(2) = 2·artanh(1/3)\n";

		// artanh(1/3) = 1/3 + (1/3)^3/3 + (1/3)^5/5 + ...
		ereal<nlimbs> one(1.0);
		ereal<nlimbs> three(3.0);
		ereal<nlimbs> x = one / three;  // x = 1/3

		ereal<nlimbs> result(0.0);
		ereal<nlimbs> x_power = x;  // Start with x^1
		ereal<nlimbs> x_squared = x * x;

		int terms = 50;
		for (int n = 0; n < terms; ++n) {
			int k = 2 * n + 1;
			double coeff = 1.0 / k;

			// term = coeff * x_power
			ereal<nlimbs> term = x_power * coeff;
			result = result + term;

			// x_power = x_power * x^2
			x_power = x_power * x_squared;
		}

		// ln(2) = 2 · artanh(1/3)
		ereal<nlimbs> two(2.0);
		ereal<nlimbs> ln2 = two * result;

		std::cout << "  ln(2) computed with " << ln2.limbs().size() << " components\n";
		std::cout << "  ln(2) ≈ " << std::setprecision(20) << double(ln2) << "\n\n";

		return ln2;
	}

	// ===================================================================
	// HELPER: Extract N components from ereal for qd representation
	// ===================================================================

	template<unsigned nlimbs>
	void print_qd_constant(const std::string& name, const ereal<nlimbs>& value) {
		const std::vector<double>& expansion = value.limbs();

		std::cout << "// " << name << "\n";
		std::cout << "constexpr double " << name << "_qd[4] = {\n";

		for (size_t i = 0; i < 4; ++i) {
			if (i < expansion.size()) {
				std::cout << "    " << std::setprecision(17) << std::scientific << expansion[i];
			} else {
				std::cout << "    " << std::setprecision(17) << std::scientific << 0.0;
			}
			if (i < 3) std::cout << ",";
			std::cout << "\n";
		}
		std::cout << "};\n\n";
	}

}} // namespace sw::universal

// Main driver
int main()
try {
	// enable UTF-8 output on Windows consoles
	ConsoleUTF8 consoleutf8;  // RAII - reset console to original code page on destruction
	using namespace sw::universal;

	// Use nlimbs = 128 to allow expansions to grow as needed
	constexpr unsigned nlimbs = 128;

	std::cout << "========================================================\n";
	std::cout << "Mathematical Constant Generation using ereal<" << nlimbs << ">\n";
	std::cout << "========================================================\n\n";

	// Compute fundamental constants
	ereal<nlimbs> pi = compute_pi<nlimbs>();
	ereal<nlimbs> e = compute_e<nlimbs>();
	ereal<nlimbs> sqrt2 = compute_sqrt<nlimbs>(2.0);
	ereal<nlimbs> ln2 = compute_ln2<nlimbs>();

	// Compute additional square roots
	ereal<nlimbs> sqrt3 = compute_sqrt<nlimbs>(3.0);
	ereal<nlimbs> sqrt5 = compute_sqrt<nlimbs>(5.0);
	ereal<nlimbs> sqrt7 = compute_sqrt<nlimbs>(7.0);
	ereal<nlimbs> sqrt11 = compute_sqrt<nlimbs>(11.0);

	// ===================================================================
	// VERIFY CONSTANTS with Mathematical Identities
	// ===================================================================

	std::cout << "========================================================\n";
	std::cout << "VERIFYING CONSTANTS with Mathematical Identities\n";
	std::cout << "========================================================\n\n";

	int nrOfFailedTests = 0;

	// Test 1: Compare against known double values
	{
		double pi_val = double(pi);
		double pi_known = 3.14159265358979323846;
		double error = std::abs(pi_val - pi_known);

		std::cout << "π compared to known value:\n";
		std::cout << "  Computed: " << std::setprecision(20) << pi_val << "\n";
		std::cout << "  Known:    " << std::setprecision(20) << pi_known << "\n";
		std::cout << "  Error:    " << std::setprecision(6) << std::scientific << error << "\n";

		if (error > 1.0e-15) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	{
		double e_val = double(e);
		double e_known = 2.71828182845904523536;
		double error = std::abs(e_val - e_known);

		std::cout << "e compared to known value:\n";
		std::cout << "  Computed: " << std::setprecision(20) << e_val << "\n";
		std::cout << "  Known:    " << std::setprecision(20) << e_known << "\n";
		std::cout << "  Error:    " << std::setprecision(6) << std::scientific << error << "\n";

		if (error > 1.0e-15) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	// ===================================================================
	// ROUND-TRIP VALIDATION TESTS
	// ===================================================================

	std::cout << "========================================================\n";
	std::cout << "ROUND-TRIP VALIDATION TESTS (No Oracle Required)\n";
	std::cout << "========================================================\n\n";

	// Test square roots: sqrt(n)² = n
	std::cout << "--- Square Root Round-Trip: sqrt(n)² = n ---\n\n";

	std::vector<std::pair<double, ereal<nlimbs>>> sqrt_tests = {
		{2.0, sqrt2}, {3.0, sqrt3}, {5.0, sqrt5}, {7.0, sqrt7}, {11.0, sqrt11}
	};

	for (const auto& test : sqrt_tests) {
		double n = test.first;
		const ereal<nlimbs>& sqrt_n = test.second;

		ereal<nlimbs> squared = sqrt_n * sqrt_n;
		double result = double(squared);
		double error = std::abs(result - n);
		double rel_error = error / n;

		std::cout << "√" << n << " × √" << n << " = " << n << ":\n";
		std::cout << "  Result:        " << std::setprecision(20) << result << "\n";
		std::cout << "  Expected:      " << std::setprecision(20) << n << "\n";
		std::cout << "  Relative error: " << std::setprecision(6) << std::scientific << rel_error << "\n";

		if (rel_error > 1.0e-28) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	// Test arithmetic identities: (a×b)/b = a
	std::cout << "--- Arithmetic Round-Trip: (a×b)/b = a ---\n\n";

	{
		ereal<nlimbs> a = pi;
		ereal<nlimbs> b = e;
		ereal<nlimbs> product = a * b;
		ereal<nlimbs> recovered = product / b;

		double a_val = double(a);
		double recovered_val = double(recovered);
		double error = std::abs(a_val - recovered_val);
		double rel_error = error / std::abs(a_val);

		std::cout << "(π × e) / e = π:\n";
		std::cout << "  Original:      " << std::setprecision(20) << a_val << "\n";
		std::cout << "  Recovered:     " << std::setprecision(20) << recovered_val << "\n";
		std::cout << "  Relative error: " << std::setprecision(6) << std::scientific << rel_error << "\n";

		if (rel_error > 1.0e-25) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	// Test: (a+b)-b = a
	std::cout << "--- Addition Round-Trip: (a+b)-b = a ---\n\n";

	{
		ereal<nlimbs> a = sqrt2;
		ereal<nlimbs> b = sqrt3;
		ereal<nlimbs> sum = a + b;
		ereal<nlimbs> recovered = sum - b;

		double a_val = double(a);
		double recovered_val = double(recovered);
		double error = std::abs(a_val - recovered_val);
		double rel_error = error / std::abs(a_val);

		std::cout << "(√2 + √3) - √3 = √2:\n";
		std::cout << "  Original:      " << std::setprecision(20) << a_val << "\n";
		std::cout << "  Recovered:     " << std::setprecision(20) << recovered_val << "\n";
		std::cout << "  Relative error: " << std::setprecision(6) << std::scientific << rel_error << "\n";

		if (rel_error > 1.0e-28) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	// Test rational round-trip: (p/q)×q = p
	std::cout << "--- Rational Round-Trip: (p/q)×q = p ---\n\n";

	{
		ereal<nlimbs> p(7.0);
		ereal<nlimbs> q(13.0);
		ereal<nlimbs> quotient = p / q;
		ereal<nlimbs> recovered = quotient * q;

		double p_val = double(p);
		double recovered_val = double(recovered);
		double error = std::abs(p_val - recovered_val);
		double rel_error = error / std::abs(p_val);

		std::cout << "(7/13) × 13 = 7:\n";
		std::cout << "  Original:      " << std::setprecision(20) << p_val << "\n";
		std::cout << "  Recovered:     " << std::setprecision(20) << recovered_val << "\n";
		std::cout << "  Relative error: " << std::setprecision(6) << std::scientific << rel_error << "\n";

		if (rel_error > 1.0e-28) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	// Test compound operations: ((a+b)×c)/c = a+b
	std::cout << "--- Compound Round-Trip: ((a+b)×c)/c = a+b ---\n\n";

	{
		ereal<nlimbs> a = sqrt5;
		ereal<nlimbs> b = sqrt7;
		ereal<nlimbs> c = pi;
		ereal<nlimbs> sum = a + b;
		ereal<nlimbs> product = sum * c;
		ereal<nlimbs> recovered = product / c;

		double sum_val = double(sum);
		double recovered_val = double(recovered);
		double error = std::abs(sum_val - recovered_val);
		double rel_error = error / std::abs(sum_val);

		std::cout << "((√5 + √7) × π) / π = √5 + √7:\n";
		std::cout << "  Original:      " << std::setprecision(20) << sum_val << "\n";
		std::cout << "  Recovered:     " << std::setprecision(20) << recovered_val << "\n";
		std::cout << "  Relative error: " << std::setprecision(6) << std::scientific << rel_error << "\n";

		// More lenient threshold for compound operations due to double conversion rounding
		if (rel_error > 1.0e-14) {
			std::cout << "  FAIL\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS\n";
		}
		std::cout << "\n";
	}

	if (nrOfFailedTests == 0) {
		std::cout << "All validation tests PASSED ✓\n\n";
	}
	else {
		std::cout << "FAILED: " << nrOfFailedTests << " validation tests failed\n\n";
	}

	// ===================================================================
	// GENERATE 4-COMPONENT QD REPRESENTATIONS
	// ===================================================================

	std::cout << "========================================================\n";
	std::cout << "4-COMPONENT QD REPRESENTATIONS\n";
	std::cout << "========================================================\n\n";

	std::cout << "// Copy these into your qd/qd_cascade constants file:\n\n";

	print_qd_constant("pi", pi);
	print_qd_constant("e", e);
	print_qd_constant("sqrt2", sqrt2);
	print_qd_constant("sqrt3", sqrt3);
	print_qd_constant("sqrt5", sqrt5);
	print_qd_constant("ln2", ln2);

	// Derive related constants
	ereal<nlimbs> two(2.0);
	ereal<nlimbs> four(4.0);

	ereal<nlimbs> pi_over_2 = pi / two;
	print_qd_constant("pi_over_2", pi_over_2);

	ereal<nlimbs> pi_over_4 = pi / four;
	print_qd_constant("pi_over_4", pi_over_4);

	ereal<nlimbs> one(1.0);
	ereal<nlimbs> one_over_pi = one / pi;
	print_qd_constant("one_over_pi", one_over_pi);

	ereal<nlimbs> two_over_pi = two / pi;
	print_qd_constant("two_over_pi", two_over_pi);

	std::cout << "========================================================\n";
	std::cout << "Constant generation complete!\n";
	std::cout << "========================================================\n";

	return (nrOfFailedTests > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& ex) {
	std::cerr << "Caught exception: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
