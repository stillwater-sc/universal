// l3_fused_mv.cpp example program to demonstrate BLAS L3 Reproducible Matrix-Matrix product
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#define BLAS_L2
#define BLAS_L3
#include <posit>
#include "blas_utils.hpp"

template<typename Scalar, size_t N>
void GenerateHilbertMatrixTest() {
	using namespace std;
	using namespace sw::unum;
	vector<Scalar> A(N*N), B(N*N), C(N*N);
	GenerateHilbertMatrix(N, A);
	GenerateHilbertMatrixInverse(N, B);
	init(C, Scalar(0.0));
	matmul(A, B, C);
	printMatrix(cout, "A matrix", A);
	printMatrix(cout, "B matrix", B);
	printMatrix(cout, "C matrix", C);
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	constexpr size_t N = 5;
	GenerateHilbertMatrixTest<posit<32, 2>, N>();
	GenerateHilbertMatrixTest<posit<64, 3>, N>();
	GenerateHilbertMatrixTest<posit<128, 4>, N>();

	GenerateHilbertMatrixTest<float, N>();
	GenerateHilbertMatrixTest<double, N>();
	GenerateHilbertMatrixTest<long double, N>();

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}