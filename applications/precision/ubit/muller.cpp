// muller.cpp: Muller's Recurrence - demonstrating ubit detection of numerical instability
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.

/*
 * Muller's Recurrence (Jean-Michel Muller):
 *
 * v[1] = 2
 * v[2] = -4
 * v[n] = 111 - 1130/v[n-1] + 3000/(v[n-1] * v[n-2])
 *
 * Mathematical limit: The sequence converges to 6.
 *
 * IEEE behavior: Due to rounding errors, the sequence converges to 100 instead!
 * The sequence appears stable across single, double, and even quad precision,
 * giving the programmer false confidence in a completely wrong result.
 *
 * areal with ubit: The uncertainty should grow with each iteration, warning
 * the programmer that the computed values are becoming unreliable.
 *
 * This demonstrates: IEEE gives a stable but WRONG answer.
 *                    areal's growing ubit warns of instability.
 */

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <iomanip>
#include <vector>

namespace sw { namespace universal {

template<typename Scalar>
struct MullerResult {
	Scalar value;
	bool uncertain;      // true if ubit is set (for areal)
	int iteration;
};

template<typename Scalar>
std::vector<MullerResult<Scalar>> muller_sequence(int n) {
	std::vector<MullerResult<Scalar>> results;

	Scalar v_prev2 = Scalar(2);   // v[1]
	Scalar v_prev1 = Scalar(-4);  // v[2]

	results.push_back({v_prev2, false, 1});
	results.push_back({v_prev1, false, 2});

	for (int i = 3; i <= n; ++i) {
		Scalar v = Scalar(111) - Scalar(1130) / v_prev1 + Scalar(3000) / (v_prev1 * v_prev2);

		bool uncertain = false;
		if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
		              std::is_same_v<Scalar, areal<64, 11, uint64_t>> ||
		              std::is_same_v<Scalar, areal<16, 5, uint16_t>>) {
			uncertain = v.at(0); // ubit is LSB
		}

		results.push_back({v, uncertain, i});

		v_prev2 = v_prev1;
		v_prev1 = v;
	}

	return results;
}

template<typename Scalar>
void test_muller(const std::string& type_name, int max_iter) {
	auto results = muller_sequence<Scalar>(max_iter);

	std::cout << "\n" << type_name << ":\n";
	std::cout << std::setw(6) << "n" << std::setw(25) << "v[n]" << std::setw(15) << "status" << '\n';
	std::cout << std::string(50, '-') << '\n';

	// Show selected iterations
	std::vector<int> show_iters = {1, 2, 3, 4, 5, 10, 15, 20, 25, 30};
	for (int iter : show_iters) {
		if (iter <= max_iter) {
			const auto& r = results[iter - 1];
			std::cout << std::setw(6) << r.iteration
			          << std::setw(25) << std::setprecision(12) << double(r.value);
			if (r.uncertain) {
				std::cout << std::setw(15) << "[UNCERTAIN]";
			} else {
				std::cout << std::setw(15) << "[exact]";
			}
			std::cout << '\n';
		}
	}

	// Show final value
	const auto& final_result = results.back();
	std::cout << "\nFinal v[" << max_iter << "] = " << double(final_result.value);
	if (final_result.uncertain) std::cout << " [UNCERTAIN]";
	std::cout << "\nCorrect limit = 6.0\n";
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Muller's Recurrence: v[n] = 111 - 1130/v[n-1] + 3000/(v[n-1]*v[n-2])\n";
	std::cout << "Starting values: v[1] = 2, v[2] = -4\n";
	std::cout << "Correct limit: 6.0\n";
	std::cout << "IEEE computes: 100.0 (WRONG!)\n";
	std::cout << std::string(80, '=') << '\n';

	constexpr int max_iterations = 30;

	// IEEE types - all converge to 100 (wrong!)
	std::cout << "\n=== IEEE Floating-Point (converges to 100 - WRONG) ===";
	test_muller<float>("float", max_iterations);
	test_muller<double>("double", max_iterations);

	// cfloat for comparison
	std::cout << "\n=== cfloat (IEEE-style, no ubit) ===";
	test_muller<cfloat<32, 8, uint32_t>>("cfloat<32,8>", max_iterations);
	test_muller<cfloat<64, 11, uint64_t>>("cfloat<64,11>", max_iterations);

	// areal with ubit - should show growing uncertainty
	std::cout << "\n=== areal (with ubit uncertainty tracking) ===";
	test_muller<areal<32, 8, uint32_t>>("areal<32,8>", max_iterations);
	test_muller<areal<64, 11, uint64_t>>("areal<64,11>", max_iterations);

	std::cout << "\n" << std::string(80, '=') << '\n';
	std::cout << "Key insight:\n";
	std::cout << "  - IEEE floats converge stably to 100 (completely wrong)\n";
	std::cout << "  - areal's ubit should grow, warning of accumulating uncertainty\n";
	std::cout << "  - A growing ubit tells you: 'Don't trust this result!'\n";

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
