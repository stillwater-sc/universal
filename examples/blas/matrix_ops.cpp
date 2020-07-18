// matrix_ops.cpp: matrix API for sw::unum::blas
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>

template<typename Matrix>
void generateIdentity() {
	Matrix A(4, 4);
	A = 1;
	std::cout << A << std::endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum::blas;

	using p8_0  = sw::unum::posit< 8,0>;
	using p16_1 = sw::unum::posit<16,1>;
	using p32_2 = sw::unum::posit<32,2>;

	using MatrixP08 = sw::unum::blas::matrix<p8_0>;
	using MatrixP16 = sw::unum::blas::matrix<p16_1>;
	using MatrixP32 = sw::unum::blas::matrix<p32_2>;

	generateIdentity<MatrixP08>();
	generateIdentity<MatrixP16>();
	generateIdentity<MatrixP32>();

	return EXIT_SUCCESS;
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
