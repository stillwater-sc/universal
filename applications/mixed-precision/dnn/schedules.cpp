// schedules.cpp: show different matmul schedules for MLIR compilers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the cfloat and lns environment
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/dnn/dnn.hpp>

void inner_product_method(unsigned N = 3) {
	std::cout << '\n';
	std::cout << "inner-product ijk order; dot product is unrolled to be executed in parallel\n";
	std::cout << "U[i, j] += W[i, k] * Y[k, j]\n";
	for (unsigned i = 0; i < N; ++i) {
		for (unsigned j = 0; j < N; ++j) {
			for (unsigned k = 0; k < N; ++k) {
				std::cout << "U[" << i << ", " << j << "] += W[" << i << ", " << k << "] * Y[" << k << ", " << j << "]\n";
			}
		}
	}
}

void middle_product_method(unsigned N = 3) {
	std::cout << '\n';
	std::cout << "middle-product jki order; N dot products can be executed in parallel\n";
	std::cout << "U[i, j] += W[i, k] * Y[k, j]\n";
	for (unsigned j = 0; j < N; ++j) {
		for (unsigned k = 0; k < N; ++k) {
			for (unsigned i = 0; i < N; ++i) {
				std::cout << "U[" << i << ", " << j << "] += W[" << i << ", " << k << "] * Y[" << k << ", " << j << "]\n";
			}
		}
	}
}

void outer_product_method(unsigned N = 3) {
	std::cout << '\n';
	std::cout << "outer-product kij order; each dot product can be executed in parallel\n";
	std::cout << "U[i, j] += W[i, k] * Y[k, j]\n";
	for (unsigned k = 0; k < N; ++k) {
		for (unsigned i = 0; i < N; ++i) {
			for (unsigned j = 0; j < N; ++j) {
				std::cout << "U[" << i << ", " << j << "] += W[" << i << ", " << k << "] * Y[" << k << ", " << j << "]\n";
			}
		}
	}
}



int main()
try {
	using namespace sw::universal;

	inner_product_method();
	middle_product_method();
	outer_product_method();

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
