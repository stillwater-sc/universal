// hilbert.cpp: Hilbert matrix
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4100) // argc/argv unreferenced formal parameter
#pragma warning(disable : 4514 4571)
#pragma warning(disable : 4625 4626) // 4625: copy constructor was implicitly defined as deleted, 4626: assignment operator was implicitely defined as deleted
#pragma warning(disable : 5025 5026 5027 5045)
#pragma warning(disable : 4710 4774)
#pragma warning(disable : 4820)
#endif
// enable the following define to show the intermediate steps in the fused-dot product
// #define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

template<typename Scalar>
void HilbertMatrixTest(size_t N = 5) {
	using namespace sw::universal::blas;
//	using Vector = sw::universal::blas::vector<Scalar>;
	using Matrix = sw::universal::blas::matrix<Scalar>;
	Matrix H(N, N), Hscale(N, N), Hinv(N, N), Hscaleinv(N, N);

	std::cout << "HilbertMatrixTest for type: " << typeid(Scalar).name() << '\n';
	// first a non-scaled Hilbert matrix that suffers from representational error
	// as 1/3, 1/6, 1/7, etc cannot be represented in binary arithmetic
	GenerateHilbertMatrix<Scalar>(H, false);
	GenerateHilbertMatrixInverse<Scalar>(Hinv);
	std::cout << "Hilbert matrix\n" << H << '\n';
	std::cout << "Hilbert inverse\n" << Hinv << '\n';
	std::cout << "Validation: Hinv * H => I\n" << Hinv * H << '\n';

	Scalar lcm = (Scalar)GenerateHilbertMatrix<Scalar>(Hscale, true); // scale the Hilbert matrix entries to be binary representable
	//GenerateHilbertMatrixInverse(Hscaleinv, lcm); // <-- scale the inverse
	GenerateHilbertMatrixInverse(Hscaleinv);
	std::cout << "Scaled Hilbert matrix: lcm = " << lcm << "\n" << Hscale << '\n';
	std::cout << "Scaled Hilbert inverse\n" << Hscaleinv << '\n';
	std::cout << "Validation: Hinv * H => I\n" << Hscaleinv * Hscale << '\n';
	std::cout << "Rescaled with lcm = " << lcm << '\n' << (Hscaleinv * Hscale) / lcm << '\n';

	std::cout << "Computing a Hilbert matrix inverse through Gauss-Jordan\n";
	auto Hinvcomputed = inv(H);
	std::cout << "Hilbert inverse computed with Gauss-Jordan\n" << Hinvcomputed << '\n';
	std::cout << "Validation: Hinv * H => I\n" << Hinvcomputed * H << '\n';
	std::cout << "------------------------------------------------------\n";
}

int main(int argc, char* argv[])
try {
	using namespace sw::universal;

	if (argc == 1) std::cout << argv[0] << '\n';

	HilbertMatrixTest<float>();
	HilbertMatrixTest< posit<32, 2> >();
	HilbertMatrixTest< posit<256, 5> >();

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
