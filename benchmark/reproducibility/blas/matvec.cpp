// matvect.cpp: data flow performance measurement of mixed-precision matrix-vector product
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>

template<typename Scalar>
void catastrophicCancellationTest() {
	std::cout << "\nScalar type : " << typeid(Scalar).name() << '\n';
	using Matrix = sw::universal::blas::matrix<Scalar>;
	using Vector = sw::universal::blas::vector<Scalar>;

	Scalar a1 = 3.2e8;
	Scalar a2 = 1;
	Scalar a3 = -1;
	Scalar a4 = 8e7;
	Matrix A = { 
		{ a1, a2, a3, a4 }, 
		{ a1, a2, a3, a4 } 
	};
	std::cout << std::setprecision(10);
	std::cout << "matrix A: \n" << A << '\n';
	Vector x = { 4.0e7, 1, -1, -1.6e8 };
	std::cout << "vector x: \n" << x << '\n';
	Vector b(2);
	b = A * x;
	std::cout << "vector b: \n" << b << '\n';
	if (b[0] == 2 && b[1] == 2) {
		std::cout << "PASS\n";
	}
	else {
		std::cout << "FAIL\n";
	}
}

int main(int argc, char** argv)
try {
	catastrophicCancellationTest<float>();
	catastrophicCancellationTest<double>();
	catastrophicCancellationTest< sw::universal::posit<32,2> >();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
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
