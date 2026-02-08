// quadratic.cpp: Quadratic Formula with near-zero discriminant - demonstrating ubit advantage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.

/*
 * The Quadratic Formula Problem:
 *
 * For ax^2 + bx + c = 0, the roots are:
 *   x = (-b +/- sqrt(b^2 - 4ac)) / (2a)
 *
 * When the discriminant (b^2 - 4ac) is close to zero, catastrophic cancellation
 * can occur in the subtraction b^2 - 4ac.
 *
 * Example: a = 1, b = 200, c = 10000 - epsilon
 * When epsilon is small, b^2 - 4ac = 40000 - 40000 + 4*epsilon is very small.
 *
 * IEEE behavior: Silent loss of precision, potentially returning wrong roots
 * or even returning that there are no real roots when there are.
 *
 * areal with ubit: Should indicate uncertainty when catastrophic cancellation
 * occurs, warning that the computed discriminant may be unreliable.
 *
 * This demonstrates: ubit flags when cancellation corrupts the discriminant.
 */

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <iomanip>
#include <cmath>

namespace sw { namespace universal {

template<typename Scalar>
struct QuadraticResult {
	Scalar discriminant;
	Scalar root1;
	Scalar root2;
	bool discriminant_uncertain;
	bool roots_uncertain;
};

template<typename Scalar>
QuadraticResult<Scalar> solve_quadratic(Scalar a, Scalar b, Scalar c) {
	QuadraticResult<Scalar> result;

	// Compute discriminant: b^2 - 4ac
	Scalar b_squared = b * b;
	Scalar four_ac = Scalar(4) * a * c;
	result.discriminant = b_squared - four_ac;

	// Check uncertainty for areal types
	result.discriminant_uncertain = false;
	if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
	              std::is_same_v<Scalar, areal<64, 11, uint64_t>>) {
		result.discriminant_uncertain = result.discriminant.at(0);
	}

	// Compute roots (assuming non-negative discriminant for this test)
	if (double(result.discriminant) >= 0) {
		Scalar sqrt_disc = Scalar(std::sqrt(double(result.discriminant)));
		Scalar two_a = Scalar(2) * a;
		result.root1 = (-b + sqrt_disc) / two_a;
		result.root2 = (-b - sqrt_disc) / two_a;

		result.roots_uncertain = false;
		if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
		              std::is_same_v<Scalar, areal<64, 11, uint64_t>>) {
			result.roots_uncertain = result.root1.at(0) || result.root2.at(0);
		}
	} else {
		result.root1 = Scalar(0);
		result.root2 = Scalar(0);
		result.roots_uncertain = true;
	}

	return result;
}

template<typename Scalar>
void test_quadratic(const std::string& type_name, double a, double b, double c,
                    double true_root1, double true_root2) {
	auto result = solve_quadratic(Scalar(a), Scalar(b), Scalar(c));

	std::cout << "\n" << type_name << ":\n";
	std::cout << "  Discriminant: " << std::setprecision(15) << double(result.discriminant);
	if (result.discriminant_uncertain) std::cout << " [UNCERTAIN]";
	std::cout << '\n';

	std::cout << "  Root 1: " << double(result.root1);
	std::cout << "  (true: " << true_root1 << ", error: "
	          << std::scientific << std::abs(double(result.root1) - true_root1) << std::fixed << ")";
	if (result.roots_uncertain) std::cout << " [UNCERTAIN]";
	std::cout << '\n';

	std::cout << "  Root 2: " << double(result.root2);
	std::cout << "  (true: " << true_root2 << ", error: "
	          << std::scientific << std::abs(double(result.root2) - true_root2) << std::fixed << ")";
	if (result.roots_uncertain) std::cout << " [UNCERTAIN]";
	std::cout << '\n';
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Quadratic Formula: Catastrophic Cancellation in the Discriminant\n";
	std::cout << std::string(80, '=') << '\n';

	// Test case 1: Well-conditioned problem (no cancellation)
	{
		std::cout << "\n=== Test 1: Well-conditioned (x^2 - 5x + 6 = 0) ===\n";
		std::cout << "Roots should be x = 2 and x = 3\n";
		double a = 1, b = -5, c = 6;

		test_quadratic<float>("float", a, b, c, 2.0, 3.0);
		test_quadratic<double>("double", a, b, c, 2.0, 3.0);
		test_quadratic<areal<32, 8, uint32_t>>("areal<32,8>", a, b, c, 2.0, 3.0);
		test_quadratic<areal<64, 11, uint64_t>>("areal<64,11>", a, b, c, 2.0, 3.0);
	}

	// Test case 2: Near-zero discriminant (catastrophic cancellation)
	{
		std::cout << "\n=== Test 2: Near-zero discriminant (x^2 + 200x + 9999.9999 = 0) ===\n";
		// b^2 = 40000, 4ac = 39999.9996, discriminant ~ 0.0004
		double a = 1, b = 200, c = 9999.9999;
		double disc = b*b - 4*a*c;
		double true_root1 = (-b + std::sqrt(disc)) / (2*a);
		double true_root2 = (-b - std::sqrt(disc)) / (2*a);
		std::cout << "True discriminant: " << disc << '\n';
		std::cout << "True roots: " << true_root1 << " and " << true_root2 << '\n';

		test_quadratic<float>("float", a, b, c, true_root1, true_root2);
		test_quadratic<double>("double", a, b, c, true_root1, true_root2);
		test_quadratic<areal<32, 8, uint32_t>>("areal<32,8>", a, b, c, true_root1, true_root2);
		test_quadratic<areal<64, 11, uint64_t>>("areal<64,11>", a, b, c, true_root1, true_root2);
	}

	// Test case 3: Very small discriminant (severe cancellation)
	{
		std::cout << "\n=== Test 3: Very small discriminant (x^2 + 200x + 9999.999999 = 0) ===\n";
		double a = 1, b = 200, c = 9999.999999;
		double disc = b*b - 4*a*c;
		double true_root1 = (-b + std::sqrt(disc)) / (2*a);
		double true_root2 = (-b - std::sqrt(disc)) / (2*a);
		std::cout << "True discriminant: " << std::setprecision(15) << disc << '\n';
		std::cout << "True roots: " << true_root1 << " and " << true_root2 << '\n';

		test_quadratic<float>("float", a, b, c, true_root1, true_root2);
		test_quadratic<double>("double", a, b, c, true_root1, true_root2);
		test_quadratic<areal<32, 8, uint32_t>>("areal<32,8>", a, b, c, true_root1, true_root2);
		test_quadratic<areal<64, 11, uint64_t>>("areal<64,11>", a, b, c, true_root1, true_root2);
	}

	std::cout << "\n" << std::string(80, '=') << '\n';
	std::cout << "Key insight:\n";
	std::cout << "  - When b^2 ~ 4ac, catastrophic cancellation corrupts the discriminant\n";
	std::cout << "  - IEEE floats silently lose precision\n";
	std::cout << "  - areal's ubit flags when the discriminant becomes unreliable\n";
	std::cout << "  - This allows the programmer to take corrective action\n";

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
