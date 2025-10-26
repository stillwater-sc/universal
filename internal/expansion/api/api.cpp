// api.cpp: API usage examples for expansion operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <universal/internal/expansion/expansion_ops.hpp>

// Helper to print an expansion
void print_expansion(const std::string& label, const std::vector<double>& e) {
	std::cout << label << " [" << e.size() << " components]: ";
	if (e.empty()) {
		std::cout << "(empty)";
	}
	else {
		std::cout << std::scientific << std::setprecision(17);
		std::cout << "{";
		for (size_t i = 0; i < e.size(); ++i) {
			if (i > 0) std::cout << ", ";
			std::cout << e[i];
		}
		std::cout << "}";
	}
	std::cout << '\n';
}

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::expansion_ops;

	std::cout << "Expansion Operations API Examples\n";
	std::cout << "==================================\n\n";

	// Example 1: Error-free transformations
	std::cout << "Example 1: Error-Free Transformations\n";
	std::cout << "--------------------------------------\n";
	{
		double a = 1.0e16;
		double b = 1.0;
		double sum, error;

		two_sum(a, b, sum, error);
		std::cout << std::setprecision(17);
		std::cout << "TWO-SUM(" << a << ", " << b << "):\n";
		std::cout << "  sum   = " << sum << "\n";
		std::cout << "  error = " << error << "\n";
		std::cout << "  Verification: sum + error = " << (sum + error) << "\n";
		std::cout << "  Original    : a + b       = " << (a + b) << "\n\n";
	}

	// Example 2: FAST-TWO-SUM (when |a| >= |b|)
	{
		double a = 100.0;
		double b = 0.5;
		double sum, error;

		fast_two_sum(a, b, sum, error);
		std::cout << "FAST-TWO-SUM(" << a << ", " << b << "):\n";
		std::cout << "  sum   = " << sum << "\n";
		std::cout << "  error = " << error << "\n\n";
	}

	// Example 3: TWO-PROD (error-free multiplication)
	{
		double a = 1.5;
		double b = 0.3;
		double product, error;

		two_prod(a, b, product, error);
		std::cout << "TWO-PROD(" << a << ", " << b << "):\n";
		std::cout << "  product = " << product << "\n";
		std::cout << "  error   = " << error << "\n";
		std::cout << "  Verification: product + error = " << (product + error) << "\n";
		std::cout << "  Original    : a * b           = " << (a * b) << "\n\n";
	}

	// Example 4: GROW-EXPANSION
	std::cout << "Example 2: GROW-EXPANSION\n";
	std::cout << "-------------------------\n";
	{
		std::vector<double> e = { 3.0, 5.0e-16 };  // Initial 2-component expansion
		double b = 1.0;  // Value to add

		print_expansion("Initial expansion e", e);
		std::cout << "Adding b = " << b << '\n';

		std::vector<double> h = grow_expansion(e, b);
		print_expansion("Result h = GROW(e, b)", h);
		std::cout << '\n';
	}

	// Example 5: FAST-EXPANSION-SUM
	std::cout << "Example 3: FAST-EXPANSION-SUM\n";
	std::cout << "------------------------------\n";
	{
		std::vector<double> e = { 3.0, 5.0e-16 };
		std::vector<double> f = { 2.0, 3.0e-16 };

		print_expansion("Expansion e", e);
		print_expansion("Expansion f", f);

		std::vector<double> h = fast_expansion_sum(e, f);
		print_expansion("Result h = FAST-SUM(e, f)", h);

		// Verify the result
		double e_sum = 0.0, f_sum = 0.0, h_sum = 0.0;
		for (auto v : e) e_sum += v;
		for (auto v : f) f_sum += v;
		for (auto v : h) h_sum += v;

		std::cout << std::setprecision(17);
		std::cout << "Verification:\n";
		std::cout << "  sum(e)   = " << e_sum << '\n';
		std::cout << "  sum(f)   = " << f_sum << '\n';
		std::cout << "  sum(h)   = " << h_sum << '\n';
		std::cout << "  e + f    = " << (e_sum + f_sum) << '\n';
		std::cout << '\n';
	}

	// Example 6: LINEAR-EXPANSION-SUM
	std::cout << "Example 4: LINEAR-EXPANSION-SUM\n";
	std::cout << "--------------------------------\n";
	{
		std::vector<double> e = { 10.0, 1.0e-15 };
		std::vector<double> f = { 5.0, 2.0e-15 };

		print_expansion("Expansion e", e);
		print_expansion("Expansion f", f);

		std::vector<double> h = linear_expansion_sum(e, f);
		print_expansion("Result h = LINEAR-SUM(e, f)", h);
		std::cout << '\n';
	}

	// Example 7: Expansion estimate
	std::cout << "Example 5: Expansion Estimation\n";
	std::cout << "--------------------------------\n";
	{
		std::vector<double> e = { 1.0, 5.0e-16, 3.0e-32, 1.0e-48 };

		print_expansion("Expansion e", e);

		double est = estimate(e);
		std::cout << "Estimate: " << std::setprecision(17) << est << '\n';

		// Compare with actual sum
		double actual = 0.0;
		for (auto v : e) actual += v;
		std::cout << "Actual sum (loses precision): " << actual << '\n';
		std::cout << '\n';
	}

	// Example 8: Invariant verification
	std::cout << "Example 6: Invariant Verification\n";
	std::cout << "----------------------------------\n";
	{
		std::vector<double> e1 = { 10.0, 1.0, 0.1 };  // Decreasing magnitude
		std::vector<double> e2 = { 10.0, 0.1, 1.0 };  // NOT decreasing

		print_expansion("e1", e1);
		std::cout << "  is_decreasing_magnitude: " << (is_decreasing_magnitude(e1) ? "YES" : "NO") << '\n';
		std::cout << "  is_nonoverlapping: " << (is_nonoverlapping(e1) ? "YES" : "NO") << '\n';

		print_expansion("e2", e2);
		std::cout << "  is_decreasing_magnitude: " << (is_decreasing_magnitude(e2) ? "YES" : "NO") << '\n';
		std::cout << '\n';
	}

	std::cout << "All API examples completed successfully.\n";

	return EXIT_SUCCESS;
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
