// cg.cpp: multi-precision, preconditioned Conjugate Gradient iterative solver
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
// Authors: Theodore Omtzigt
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

// cg: Solution of x in Ax=b using preconditioned Conjugate Gradient algorithm 
// with different precision for matrix-vector multiply and residual calculation
// Input: 
//   preconditioner M
//   system matrix A
//   right hand side b
//   accuracy tolerance for target solution
// Output:
//   number of iterations to reach required accuracy
template<typename Matrix, typename Vector, size_t MAX_ITERATIONS = 10>
size_t cg(const Matrix& M, const Matrix& A, Vector& b, typename Matrix::value_type tolerance = typename Matrix::value_type(0.00001)) {
	using Scalar = typename Matrix::value_type;
	Scalar residual = Scalar(std::numeric_limits<Scalar>::max());
	//size_t m = num_rows(A);
	//size_t n = num_cols(A);
	Vector x(size(b));
	Vector rho(size(b));
	Vector zeta(size(b));
	Vector p(size(b));
	Vector q(size(b));
	Scalar sigma_1{ 0 }, sigma_2{ 0 }, alpha{ 0 }, beta{ 0 };
	rho = b;  // term is b - A * x, but if we use x(0) = 0 vector, rho = b is equivalent
	size_t itr = 0;
	bool firstIteration = true;
	while (residual > tolerance && itr < MAX_ITERATIONS) {
		zeta = sw::unum::blas::solve(M, rho);
		sigma_2 = sigma_1;
		sigma_1 = dot(zeta, rho); // dot product, fused dot product if Scalar is a posit type
		if (firstIteration) {
			firstIteration = false;
			p = zeta;
		}
		else {
			beta = sigma_1 / sigma_2;
			p = zeta + beta * p;
		}
		q = A * p;
		alpha = sigma_1 / dot(p, q);
		Vector x_1(x);
		x = x + alpha * p;
		rho = rho - alpha * q;
		// check for convergence of the system
		residual = norm1(x_1 - x);
		std::cout << '[' << itr << "] " << std::setw(12) << x << " residual " << residual << std::endl;
		++itr;
	}
	std::cout << "solution in " << itr << " iterations\n";
	std::cout << "solution is " << x << '\n';
	std::cout << "final residual is " << residual << '\n';
	std::cout << "validation\n" << A * x << " = " << b << std::endl;
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

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	constexpr size_t DoF = 8;
	Matrix A;
	tridiag(A, DoF);  // this does a resize of A
	Matrix M = eye<Scalar>(DoF);
	Vector b(DoF);
	Vector x(DoF);
	x = Scalar(1);
	b = A * x;

	cout << A << endl;
	cout << M << endl;
	cout << b << endl;
	constexpr size_t MAX_ITERATIONS = 10;
	size_t itr = cg<Matrix, Vector, MAX_ITERATIONS>(M, A, b);

	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}

	M = sw::unum::blas::inv(A);
	itr = cg<Matrix, Vector, MAX_ITERATIONS>(M, A, b);

	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}
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
