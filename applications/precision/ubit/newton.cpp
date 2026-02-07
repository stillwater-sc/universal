// newton.cpp: Newton-Raphson Convergence - demonstrating ubit as convergence indicator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.

/*
 * Newton-Raphson Iteration and the Ubit:
 *
 * Newton's method: x[n+1] = x[n] - f(x[n]) / f'(x[n])
 *
 * For finding sqrt(a): f(x) = x^2 - a, f'(x) = 2x
 *   x[n+1] = (x[n] + a/x[n]) / 2
 *
 * Convergence behavior:
 * - When converging: the ubit should stabilize (become 0 when converged)
 * - When diverging: the ubit should remain 1 or grow in interval width
 *
 * This demonstrates a UNIQUE capability of ubit arithmetic:
 * - The ubit naturally indicates when an iterative algorithm has converged
 * - ubit=0 means "this is the exact floating-point answer"
 * - ubit=1 means "we're still refining" or "we can't do better"
 *
 * IEEE floats provide no such indicator - you must use an external
 * convergence test (comparing successive iterates).
 */

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <iomanip>
#include <cmath>
#include <vector>

namespace sw { namespace universal {

template<typename Scalar>
struct NewtonStep {
	int iteration;
	Scalar x;
	Scalar error;  // |x - true_value|
	bool uncertain;
};

// Newton-Raphson for sqrt(a): x[n+1] = (x[n] + a/x[n]) / 2
template<typename Scalar>
std::vector<NewtonStep<Scalar>> newton_sqrt(Scalar a, Scalar x0, int max_iter) {
	std::vector<NewtonStep<Scalar>> steps;
	double true_sqrt = std::sqrt(double(a));

	Scalar x = x0;
	for (int i = 0; i <= max_iter; ++i) {
		bool uncertain = false;
		if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
		              std::is_same_v<Scalar, areal<64, 11, uint64_t>> ||
		              std::is_same_v<Scalar, areal<16, 5, uint16_t>>) {
			uncertain = x.at(0);
		}

		Scalar error_val(std::abs(double(x) - true_sqrt));
		steps.push_back({i, x, error_val, uncertain});

		if (i < max_iter) {
			x = (x + a / x) / Scalar(2);
		}
	}

	return steps;
}

// Newton-Raphson for a function with no real root (to show divergence)
// f(x) = x^2 + 1, which has no real roots
// x[n+1] = x[n] - (x[n]^2 + 1) / (2*x[n]) = (x[n]^2 - 1) / (2*x[n])
template<typename Scalar>
std::vector<NewtonStep<Scalar>> newton_no_root(Scalar x0, int max_iter) {
	std::vector<NewtonStep<Scalar>> steps;

	Scalar x = x0;
	for (int i = 0; i <= max_iter; ++i) {
		bool uncertain = false;
		if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
		              std::is_same_v<Scalar, areal<64, 11, uint64_t>> ||
		              std::is_same_v<Scalar, areal<16, 5, uint16_t>>) {
			uncertain = x.at(0);
		}

		steps.push_back({i, x, Scalar(0), uncertain});

		if (i < max_iter) {
			Scalar x2 = x * x;
			x = (x2 - Scalar(1)) / (Scalar(2) * x);
		}
	}

	return steps;
}

template<typename Scalar>
void test_newton_sqrt(const std::string& type_name, double a, double x0, int max_iter) {
	auto steps = newton_sqrt(Scalar(a), Scalar(x0), max_iter);

	std::cout << "\n" << type_name << " - Newton sqrt(" << a << "), starting x0 = " << x0 << ":\n";
	std::cout << std::setw(5) << "iter" << std::setw(25) << "x" << std::setw(20) << "error"
	          << std::setw(15) << "ubit" << '\n';
	std::cout << std::string(65, '-') << '\n';

	for (const auto& step : steps) {
		std::cout << std::setw(5) << step.iteration
		          << std::setw(25) << std::setprecision(15) << double(step.x)
		          << std::setw(20) << std::scientific << double(step.error) << std::fixed;
		if (step.uncertain) {
			std::cout << std::setw(15) << "[UNCERTAIN]";
		} else {
			std::cout << std::setw(15) << "[EXACT]";
		}
		std::cout << '\n';
	}

	std::cout << "True sqrt(" << a << ") = " << std::sqrt(a) << '\n';
}

template<typename Scalar>
void test_newton_diverge(const std::string& type_name, double x0, int max_iter) {
	auto steps = newton_no_root(Scalar(x0), max_iter);

	std::cout << "\n" << type_name << " - Newton for x^2+1=0 (NO REAL ROOT), x0 = " << x0 << ":\n";
	std::cout << std::setw(5) << "iter" << std::setw(25) << "x" << std::setw(15) << "ubit" << '\n';
	std::cout << std::string(45, '-') << '\n';

	for (const auto& step : steps) {
		std::cout << std::setw(5) << step.iteration
		          << std::setw(25) << std::setprecision(15) << double(step.x);
		if (step.uncertain) {
			std::cout << std::setw(15) << "[UNCERTAIN]";
		} else {
			std::cout << std::setw(15) << "[EXACT]";
		}
		std::cout << '\n';
	}
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Newton-Raphson: Ubit as Convergence Indicator\n";
	std::cout << std::string(80, '=') << '\n';

	// Test 1: Converging case - sqrt(2)
	{
		std::cout << "\n=== Test 1: Converging Case - sqrt(2) ===\n";
		std::cout << "Starting from x0 = 1.0, should converge to " << std::sqrt(2.0) << '\n';

		test_newton_sqrt<float>("float", 2.0, 1.0, 8);
		test_newton_sqrt<areal<32, 8, uint32_t>>("areal<32,8>", 2.0, 1.0, 8);
	}

	// Test 2: Converging case - sqrt(10)
	{
		std::cout << "\n=== Test 2: Converging Case - sqrt(10) ===\n";
		std::cout << "Starting from x0 = 3.0, should converge to " << std::sqrt(10.0) << '\n';

		test_newton_sqrt<double>("double", 10.0, 3.0, 8);
		test_newton_sqrt<areal<64, 11, uint64_t>>("areal<64,11>", 10.0, 3.0, 8);
	}

	// Test 3: Non-converging case - x^2 + 1 = 0 has no real roots
	{
		std::cout << "\n=== Test 3: Diverging Case - x^2 + 1 = 0 (no real root) ===\n";
		std::cout << "Newton's method oscillates/diverges since there's no real solution\n";

		test_newton_diverge<float>("float", 0.5, 10);
		test_newton_diverge<areal<32, 8, uint32_t>>("areal<32,8>", 0.5, 10);
	}

	std::cout << "\n" << std::string(80, '=') << '\n';
	std::cout << "Key insight:\n";
	std::cout << "  - When converging: ubit becomes EXACT (0) when we've found the answer\n";
	std::cout << "  - When diverging: ubit stays UNCERTAIN (1) warning of instability\n";
	std::cout << "  - IEEE floats require external convergence tests\n";
	std::cout << "  - areal's ubit provides a BUILT-IN convergence indicator\n";
	std::cout << "  - This is particularly useful for iterative solvers\n";

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
