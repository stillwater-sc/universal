// gauss-seidel.cpp: Gauss-Seidel iterative method
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
// Authors: Theodore Omtzigt, Allan Leal
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4514)   // unreferenced inline function has been removed
#pragma warning(disable : 4710)   // 'int sprintf_s(char *const ,const size_t,const char *const ,...)': function not inlined
#pragma warning(disable : 4820)   // 'sw::unum::value<23>': '3' bytes padding added after data member 'sw::unum::value<23>::_sign'
#pragma warning(disable : 5045)   // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif

// standard library
#include <limits>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

// Gauss-Seidel: Solution of x in Ax=b using Gauss-Seidel Method
template<typename Matrix, typename Vector, size_t MAX_ITERATIONS = 100>
size_t GaussSeidel(const Matrix& A, Vector& b, typename Matrix::value_type tolerance = typename Matrix::value_type(0.00001)) {
	using Scalar = typename Matrix::value_type;
	Scalar residual = Scalar(std::numeric_limits<Scalar>::max());
	size_t m = num_rows(A);
	size_t n = num_cols(A);
	Vector x(m);
	size_t itr = 0;
	while (residual > tolerance && itr < MAX_ITERATIONS) {
		Vector x_old = x;
		for (size_t i = 1; i <= m; ++i) {
			Scalar sigma = 0;
			for (size_t j = 1; j <= i-1; ++j) {
				sigma += A(i-1, j-1) * x(j-1);
			}
			for (size_t j = i + 1; j <= n; ++j) {
				sigma += A(i-1, j-1) * x_old(j-1);
			}
			x(i-1) = (b(i-1) - sigma) / A(i-1, i-1);
		}
		residual = norm1(x_old - x);
		std::cout << '[' << itr << "] " << x << " residual " << residual << std::endl;
		++itr;
	}

	std::cout << "solution is " << x << '\n';
	std::cout << A * x << " = " << b << std::endl;
	return itr;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits, es>;
//	using Scalar = float;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	if (argc == 1) cout << argv[0] << '\n';
	int nrOfFailedTestCases = 0;

	// Initialize 'A' 'b' & intial guess 'x' * _
	Matrix A = {
		{ 5, -2,  3,  0},
		{-3,  9,  1, -2},
		{ 2, -1, -7,  1},
		{ 4,  3, -5,  7} };
	Vector b = { -1, 2, 3, 0.5 };
	Vector x = { 0, 0, 0, 0 };

	cout << A << endl;
	cout << b << endl;
	size_t iterations = GaussSeidel(A, b);
	cout << "solution in " << iterations << " iterations" << endl;

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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
