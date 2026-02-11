// rump.cpp: Siegfried Rump's Royal Pain - demonstrating precision requirements
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.

/*
 * Rump's Example (1988):
 *
 * f(a,b) = 333.75*b^6 + a^2*(11*a^2*b^2 - b^6 - 121*b^4 - 2) + 5.5*b^8 + a/(2*b)
 *
 * For a = 77617, b = 33096:
 * - IEEE single precision:  ~1.17e+21  (WRONG - not even the right sign!)
 * - IEEE double precision:  ~1.17e+21  (WRONG - same wrong answer!)
 * - IEEE quad precision:    ~1.17e+21  (WRONG - still wrong!)
 * - Correct answer:        -0.827396059946821...
 *
 * This example was specifically designed to show that:
 * 1. More precision doesn't help if the algorithm is numerically unstable
 * 2. All IEEE precisions give the SAME confidently wrong answer
 * 3. Extended precision (dd, td, qd cascade) is needed to get the right answer
 *
 * The key insight about unums/areals:
 * - They should indicate uncertainty (ubit=1) when precision is lost
 * - They should NOT give a confidently wrong answer like IEEE floats do
 */

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <iomanip>
#include <cmath>

namespace sw { namespace universal {

// Rump's polynomial: f(a,b) = 333.75*b^6 + a^2*(11*a^2*b^2 - b^6 - 121*b^4 - 2) + 5.5*b^8 + a/(2*b)
template<typename Scalar>
Scalar rump(Scalar a, Scalar b) {
	Scalar b2 = b * b;
	Scalar b4 = b2 * b2;
	Scalar b6 = b4 * b2;
	Scalar b8 = b4 * b4;
	Scalar a2 = a * a;

	Scalar term1 = Scalar(333.75) * b6;
	Scalar term2 = a2 * (Scalar(11) * a2 * b2 - b6 - Scalar(121) * b4 - Scalar(2));
	Scalar term3 = Scalar(5.5) * b8;
	Scalar term4 = a / (Scalar(2) * b);

	return term1 + term2 + term3 + term4;
}

// Specialization for dd_cascade
dd_cascade rump_dd(dd_cascade a, dd_cascade b) {
	dd_cascade b2 = b * b;
	dd_cascade b4 = b2 * b2;
	dd_cascade b6 = b4 * b2;
	dd_cascade b8 = b4 * b4;
	dd_cascade a2 = a * a;

	dd_cascade c333_75(333.75);
	dd_cascade c11(11.0);
	dd_cascade c121(121.0);
	dd_cascade c2(2.0);
	dd_cascade c5_5(5.5);

	dd_cascade term1 = c333_75 * b6;
	dd_cascade term2 = a2 * (c11 * a2 * b2 - b6 - c121 * b4 - c2);
	dd_cascade term3 = c5_5 * b8;
	dd_cascade term4 = a / (c2 * b);

	return term1 + term2 + term3 + term4;
}

// Specialization for td_cascade
td_cascade rump_td(td_cascade a, td_cascade b) {
	td_cascade b2 = b * b;
	td_cascade b4 = b2 * b2;
	td_cascade b6 = b4 * b2;
	td_cascade b8 = b4 * b4;
	td_cascade a2 = a * a;

	td_cascade c333_75(333.75);
	td_cascade c11(11.0);
	td_cascade c121(121.0);
	td_cascade c2(2.0);
	td_cascade c5_5(5.5);

	td_cascade term1 = c333_75 * b6;
	td_cascade term2 = a2 * (c11 * a2 * b2 - b6 - c121 * b4 - c2);
	td_cascade term3 = c5_5 * b8;
	td_cascade term4 = a / (c2 * b);

	return term1 + term2 + term3 + term4;
}

// Specialization for qd_cascade
qd_cascade rump_qd(qd_cascade a, qd_cascade b) {
	qd_cascade b2 = b * b;
	qd_cascade b4 = b2 * b2;
	qd_cascade b6 = b4 * b2;
	qd_cascade b8 = b4 * b4;
	qd_cascade a2 = a * a;

	qd_cascade c333_75(333.75);
	qd_cascade c11(11.0);
	qd_cascade c121(121.0);
	qd_cascade c2(2.0);
	qd_cascade c5_5(5.5);

	qd_cascade term1 = c333_75 * b6;
	qd_cascade term2 = a2 * (c11 * a2 * b2 - b6 - c121 * b4 - c2);
	qd_cascade term3 = c5_5 * b8;
	qd_cascade term4 = a / (c2 * b);

	return term1 + term2 + term3 + term4;
}

constexpr double TRUE_ANSWER = -0.827396059946821368141165095479816291999033115784384819;

template<typename Scalar>
void test_rump(const std::string& type_name, int precision_bits = 0) {
	Scalar a = Scalar(77617);
	Scalar b = Scalar(33096);
	Scalar result = rump(a, b);

	double result_d = double(result);
	double error = std::abs(result_d - TRUE_ANSWER);
	double rel_error = error / std::abs(TRUE_ANSWER);

	std::cout << std::setw(35) << type_name << ": ";
	std::cout << std::setw(25) << std::setprecision(15) << result_d;

	// Check if result is correct (within 1%)
	bool correct = (rel_error < 0.01);
	bool close = (rel_error < 0.1);

	if (std::isinf(result_d) || std::isnan(result_d)) {
		std::cout << "  [OVERFLOW/NaN]";
	} else if (correct) {
		std::cout << "  [CORRECT!]";
	} else if (close) {
		std::cout << "  [close, err=" << std::scientific << rel_error << std::fixed << "]";
	} else {
		std::cout << "  [WRONG by " << std::scientific << rel_error << std::fixed << "]";
	}

	// For areal types, check the ubit
	if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
	              std::is_same_v<Scalar, areal<64, 11, uint64_t>> ||
	              std::is_same_v<Scalar, areal<128, 15, uint32_t>>) {
		bool uncertain = result.at(0); // ubit is LSB
		if (uncertain) {
			std::cout << " ubit=1";
		}
	}

	std::cout << '\n';
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Rump's Royal Pain: f(77617, 33096)\n";
	std::cout << std::setprecision(20);
	std::cout << "Correct answer: " << TRUE_ANSWER << '\n';
	std::cout << std::string(90, '=') << '\n';

	std::cout << "\n--- IEEE Floating-Point (all give the SAME wrong answer) ---\n";
	test_rump<float>("float (~24 bits)");
	test_rump<double>("double (~53 bits)");
	if constexpr (sizeof(long double) > sizeof(double)) {
		long double a = 77617.0L, b = 33096.0L;
		long double b2 = b * b, b4 = b2 * b2, b6 = b4 * b2, b8 = b4 * b4, a2 = a * a;
		long double result = 333.75L * b6 + a2 * (11.0L * a2 * b2 - b6 - 121.0L * b4 - 2.0L) + 5.5L * b8 + a / (2.0L * b);
		double result_d = double(result);
		double error = std::abs(result_d - TRUE_ANSWER);
		double rel_error = error / std::abs(TRUE_ANSWER);
		std::cout << std::setw(35) << "long double (~64-80 bits)" << ": ";
		std::cout << std::setw(25) << std::setprecision(15) << result_d;
		if (rel_error < 0.01) std::cout << "  [CORRECT!]";
		else std::cout << "  [WRONG by " << std::scientific << rel_error << std::fixed << "]";
		std::cout << '\n';
	}

	std::cout << "\n--- Extended Precision Cascade Types ---\n";
	{
		dd_cascade a(77617.0), b(33096.0);
		dd_cascade result = rump_dd(a, b);
		double result_d = double(result);
		double error = std::abs(result_d - TRUE_ANSWER);
		double rel_error = error / std::abs(TRUE_ANSWER);
		std::cout << std::setw(35) << "dd_cascade (~106 bits)" << ": ";
		std::cout << std::setw(25) << std::setprecision(15) << result_d;
		if (std::isinf(result_d) || std::isnan(result_d)) {
			std::cout << "  [OVERFLOW/NaN]";
		} else if (rel_error < 0.01) {
			std::cout << "  [CORRECT!]";
		} else {
			std::cout << "  [WRONG by " << std::scientific << rel_error << std::fixed << "]";
		}
		std::cout << '\n';
	}
	{
		td_cascade a(77617.0), b(33096.0);
		td_cascade result = rump_td(a, b);
		double result_d = double(result);
		double error = std::abs(result_d - TRUE_ANSWER);
		double rel_error = error / std::abs(TRUE_ANSWER);
		std::cout << std::setw(35) << "td_cascade (~159 bits)" << ": ";
		std::cout << std::setw(25) << std::setprecision(15) << result_d;
		if (std::isinf(result_d) || std::isnan(result_d)) {
			std::cout << "  [OVERFLOW/NaN]";
		} else if (rel_error < 0.01) {
			std::cout << "  [CORRECT!]";
		} else {
			std::cout << "  [WRONG by " << std::scientific << rel_error << std::fixed << "]";
		}
		std::cout << '\n';
	}
	{
		qd_cascade a(77617.0), b(33096.0);
		qd_cascade result = rump_qd(a, b);
		double result_d = double(result);
		double error = std::abs(result_d - TRUE_ANSWER);
		double rel_error = error / std::abs(TRUE_ANSWER);
		std::cout << std::setw(35) << "qd_cascade (~212 bits)" << ": ";
		std::cout << std::setw(25) << std::setprecision(15) << result_d;
		if (std::isinf(result_d) || std::isnan(result_d)) {
			std::cout << "  [OVERFLOW/NaN]";
		} else if (rel_error < 0.01) {
			std::cout << "  [CORRECT!]";
		} else {
			std::cout << "  [WRONG by " << std::scientific << rel_error << std::fixed << "]";
		}
		std::cout << '\n';
	}

	std::cout << "\n--- cfloat (IEEE-style, no ubit) ---\n";
	test_rump<cfloat<32, 8, uint32_t>>("cfloat<32,8> (~24 bits)");
	test_rump<cfloat<64, 11, uint64_t>>("cfloat<64,11> (~53 bits)");
	test_rump<cfloat<128, 15, uint32_t>>("cfloat<128,15> (~113 bits)");

	std::cout << "\n--- areal (with ubit uncertainty tracking) ---\n";
	test_rump<areal<32, 8, uint32_t>>("areal<32,8> (~23 bits + ubit)");
	test_rump<areal<64, 11, uint64_t>>("areal<64,11> (~52 bits + ubit)");
	test_rump<areal<128, 15, uint32_t>>("areal<128,15> (~112 bits + ubit)");

	std::cout << "\n" << std::string(90, '=') << '\n';
	std::cout << "Key insights:\n";
	std::cout << "  1. IEEE float, double, and even quad give the SAME wrong answer (~1e21)\n";
	std::cout << "  2. The correct answer is -0.827... (negative, not 10^21!)\n";
	std::cout << "  3. Extended precision (dd/td/qd_cascade) may get the right answer\n";
	std::cout << "  4. areal's ubit should flag uncertainty when precision is insufficient\n";
	std::cout << "  5. A ubit=1 means 'don't trust this result' - better than a confident wrong answer\n";

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
