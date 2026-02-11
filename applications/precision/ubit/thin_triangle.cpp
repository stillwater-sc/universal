// thin_triangle.cpp: Kahan's Thin Triangle - demonstrating ubit detection of cancellation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.

/*
 * Kahan's Thin Triangle Problem (presented 1986, Goldberg paper 1991):
 *
 * Compute the area of a very thin triangle using Heron's formula:
 *   s = (a + b + c) / 2
 *   Area = sqrt(s * (s-a) * (s-b) * (s-c))
 *
 * For a thin triangle where b ~ c ~ a/2:
 *   a = 7
 *   b = c = (a + 3*ulp(a)) / 2
 *
 * The problem: When computing (s-a), catastrophic cancellation occurs
 * because s ~ a, leading to massive relative error.
 *
 * IEEE behavior: Computes an area that can be off by 30% or more,
 * with no warning to the programmer.
 *
 * areal with ubit: Should flag uncertainty when the cancellation
 * in (s-a) corrupts the result.
 *
 * Note: Kahan provided a numerically stable alternative formula for
 * triangle area, but the point here is that the ubit warns you when
 * the naive formula fails.
 */

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <iomanip>
#include <cmath>
#include <limits>

namespace sw { namespace universal {

// Heron's formula for triangle area (naive, unstable for thin triangles)
template<typename Scalar>
struct HeronResult {
	Scalar s;          // semi-perimeter
	Scalar s_minus_a;
	Scalar s_minus_b;
	Scalar s_minus_c;
	Scalar area;
	bool uncertain;
};

template<typename Scalar>
HeronResult<Scalar> heron_area(Scalar a, Scalar b, Scalar c) {
	HeronResult<Scalar> result;

	// Semi-perimeter
	result.s = (a + b + c) / Scalar(2);

	// The critical subtractions
	result.s_minus_a = result.s - a;
	result.s_minus_b = result.s - b;
	result.s_minus_c = result.s - c;

	// Product under the square root
	Scalar product = result.s * result.s_minus_a * result.s_minus_b * result.s_minus_c;

	// Area
	if (double(product) >= 0) {
		result.area = Scalar(std::sqrt(double(product)));
	} else {
		result.area = Scalar(0);
	}

	result.uncertain = false;
	if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
	              std::is_same_v<Scalar, areal<64, 11, uint64_t>>) {
		result.uncertain = result.s_minus_a.at(0) || result.s_minus_b.at(0) ||
		                   result.s_minus_c.at(0) || result.area.at(0);
	}

	return result;
}

// Kahan's numerically stable formula for triangle area
template<typename Scalar>
Scalar kahan_area(Scalar a, Scalar b, Scalar c) {
	// Sort sides so that a >= b >= c
	if (double(a) < double(b)) std::swap(a, b);
	if (double(a) < double(c)) std::swap(a, c);
	if (double(b) < double(c)) std::swap(b, c);

	// Kahan's stable formula
	Scalar term = (a + (b + c)) * (c - (a - b)) * (c + (a - b)) * (a + (b - c));
	if (double(term) >= 0) {
		return Scalar(std::sqrt(double(term))) / Scalar(4);
	}
	return Scalar(0);
}

template<typename Scalar>
void test_thin_triangle(const std::string& type_name, double a_val, double b_val, double c_val,
                        double true_area) {
	Scalar a(a_val), b(b_val), c(c_val);
	auto result = heron_area(a, b, c);
	Scalar kahan = kahan_area(a, b, c);

	std::cout << "\n" << type_name << ":\n";
	std::cout << "  s = " << std::setprecision(15) << double(result.s) << '\n';
	std::cout << "  s-a = " << double(result.s_minus_a);
	if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
	              std::is_same_v<Scalar, areal<64, 11, uint64_t>>) {
		if (result.s_minus_a.at(0)) std::cout << " [UNCERTAIN]";
	}
	std::cout << "  (this is where cancellation occurs!)\n";

	std::cout << "  Heron area:  " << double(result.area);
	double rel_error = std::abs(double(result.area) - true_area) / true_area;
	std::cout << "  (relative error: " << std::scientific << rel_error << std::fixed << ")";
	if (result.uncertain) std::cout << " [UNCERTAIN]";
	std::cout << '\n';

	std::cout << "  Kahan area:  " << double(kahan);
	rel_error = std::abs(double(kahan) - true_area) / true_area;
	std::cout << "  (relative error: " << std::scientific << rel_error << std::fixed << ")\n";

	std::cout << "  True area:   " << true_area << '\n';
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Kahan's Thin Triangle: Catastrophic Cancellation in Heron's Formula\n";
	std::cout << std::string(80, '=') << '\n';

	// Create a thin triangle
	// a = 7, b = c = (a + 3*ulp(a)) / 2
	float a_f = 7.0f;
	float ulp_a = std::nextafter(a_f, std::numeric_limits<float>::infinity()) - a_f;
	float b_f = (a_f + 3.0f * ulp_a) / 2.0f;
	float c_f = b_f;

	double a_d = double(a_f);
	double b_d = double(b_f);
	double c_d = double(c_f);

	// Compute true area using high-precision Kahan formula
	long double a_ld = a_d, b_ld = b_d, c_ld = c_d;
	if (a_ld < b_ld) std::swap(a_ld, b_ld);
	if (a_ld < c_ld) std::swap(a_ld, c_ld);
	if (b_ld < c_ld) std::swap(b_ld, c_ld);
	long double term_ld = (a_ld + (b_ld + c_ld)) * (c_ld - (a_ld - b_ld)) *
	                      (c_ld + (a_ld - b_ld)) * (a_ld + (b_ld - c_ld));
	double true_area = double(std::sqrt(term_ld) / 4.0L);

	std::cout << "\nThin triangle with sides:\n";
	std::cout << "  a = " << std::setprecision(15) << a_d << '\n';
	std::cout << "  b = " << b_d << '\n';
	std::cout << "  c = " << c_d << '\n';
	std::cout << "  True area ~ " << true_area << '\n';

	std::cout << "\n=== IEEE Floating-Point (Heron's formula has large error) ===";
	test_thin_triangle<float>("float", a_d, b_d, c_d, true_area);
	test_thin_triangle<double>("double", a_d, b_d, c_d, true_area);

	std::cout << "\n=== cfloat (IEEE-style, no ubit) ===";
	test_thin_triangle<cfloat<32, 8, uint32_t, true>>("cfloat<32,8>", a_d, b_d, c_d, true_area);
	test_thin_triangle<cfloat<64, 11, uint64_t, true>>("cfloat<64,11>", a_d, b_d, c_d, true_area);

	std::cout << "\n=== areal (with ubit uncertainty tracking) ===";
	test_thin_triangle<areal<32, 8, uint32_t>>("areal<32,8>", a_d, b_d, c_d, true_area);
	test_thin_triangle<areal<64, 11, uint64_t>>("areal<64,11>", a_d, b_d, c_d, true_area);

	std::cout << "\n" << std::string(80, '=') << '\n';
	std::cout << "Key insight:\n";
	std::cout << "  - Heron's formula has catastrophic cancellation in (s-a) for thin triangles\n";
	std::cout << "  - IEEE floats silently compute area with ~30% or more error\n";
	std::cout << "  - areal's ubit should flag the cancellation in (s-a)\n";
	std::cout << "  - Kahan's stable formula avoids the problem entirely\n";
	std::cout << "  - The ubit warns: 'Use a different algorithm!'\n";

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
