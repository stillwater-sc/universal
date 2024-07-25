// cg.cpp: multi-precision, preconditioned Conjugate Gradient iterative solver using Fused Dot Products
// using matrix-vector fused dot product operator, and compensation fused dot product operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// Authors: Theodore Omtzigt
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4514)   // unreferenced inline function has been removed
#pragma warning(disable : 4710)   // 'int sprintf_s(char *const ,const size_t,const char *const ,...)': function not inlined
#pragma warning(disable : 4820)   // 'sw::universal::value<23>': '3' bytes padding added after data member 'sw::universal::value<23>::_sign'
#pragma warning(disable : 5045)   // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif

// standard library
#include <limits>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// and fast posits
//#define POSIT_FAST_SPECIALIZATION 1
#define POSIT_FAST_POSIT_32_2 0
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/blas/solvers/cg.hpp> // adaptive CG, regular dot for IEEE, fused-dot for posits

#define SOLUTION_FEEDBACK 0

// CG residual trajectory experiment for tridiag(-1, 2, -1)
template<typename Scalar, size_t MAX_ITERATIONS = 100>
size_t fdTest(size_t DoF) {
	using namespace sw::universal::blas;
	using Matrix = matrix<Scalar>;
	using Vector = vector<Scalar>;

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	Matrix A = tridiag<Scalar>(DoF);
	Vector b(DoF);
	Vector ones(DoF);
	ones = Scalar(1);
	b = A * ones;     // generate a known solution
	//Matrix M = eye<Scalar>(DoF);    // no preconditioner
	Matrix M = inv(A);              // best possible preconditioner
	//Matrix M = inv(diag(diag(A)));  // inverse of the Jacobian
	//Matrix M = diag(diag(A));       // the Jacobian
	//Matrix M = A;				      // absolutely horrible preconditioner, just to prove that CG still works						
	Vector x(DoF);
	Vector residuals;
	size_t itr = cg(M, A, b, x, residuals);
#if SOLUTION_FEEDBACK
	std::cout << "solution is " << x << '\n';
	std::cout << "final residual is " << residuals[size(residuals) - 1] << '\n';
	std::cout << "validation\n" << A * x << " = " << b << '\n';
#endif
	std::cout << '\"' << typeid(Scalar).name() << "\" " << residuals << std::endl;

	return itr;
}


#define MANUAL 1
#define STRESS 1

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

	int nrOfFailedTestCases = 0;

#if MANUAL
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits, es>;
	using Matrix = matrix<Scalar>;
	using Vector = sw::universal::blas::vector<Scalar>;

	using Scalar2 = posit<2 * nbits, es>;
	using Vector2 = sw::universal::blas::vector<Scalar2>;


	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	constexpr size_t DoF = 8;
	Matrix A;
	tridiag(A, DoF);  // this does a resize of A
	// Matrix M = eye<Scalar>(DoF); // M = I, unpreconditioned
	Matrix M = inv(diag(diag(A)));  // Jacobi preconditioner for positive-definite, diagonally dominant systems
	Vector b(DoF);
	Vector ones(DoF);
	ones = Scalar(1);
	b = A * ones;

	Vector2 b2(DoF);
	b2 = A * ones;

	if (DoF < 10) {
		std::cout << "M^-1:\n" << M << '\n';  // it is customary to talk about the preconditioner M while understanding that it really is the inverse M^-1
		std::cout << "A:\n" << A << '\n';
		std::cout << "x:\n" << ones << '\n';
		std::cout << "b:\n" << b << '\n';
	}
	/*
	* for second order elliptical PDEs, the resulting coefficient matrix exhibits
	* a condition number k_2(A) = O(h^-2). Convergence rate of CG is sqrt(k_2)
	* so convergence is expected to be O(h^-1). 
	* The selected tridiagonal matrix has a discretization step given by DoF
	* and thus we expect the converge in sqrt(128) (h = 1/DoF -> h^-1 is Dof)
	*/
	Vector residuals;
	constexpr size_t MAX_ITERATIONS = 100;
	Vector x(DoF);
	size_t itr = cg(M, A, b, x, residuals);
	std::cout << "solution is " << x << '\n';
	std::cout << "final residual is " << residuals[size(residuals) - 1] << '\n';
	std::cout << "validation\n" << A * x << " = " << b << '\n';
	std::cout << "residuals \"" << typeid(Scalar).name() << "\" " << residuals << '\n';
	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}

#else
	// with a preconditioner M = Jacobian(A)^-1
	constexpr size_t MAX_ITERATIONS = 100;
	fdTest<float, MAX_ITERATIONS>(64);
	fdTest<double, MAX_ITERATIONS>(64);
	fdTest<long double, MAX_ITERATIONS>(64);

	fdTest<posit<16, 1>, MAX_ITERATIONS>(64);
	fdTest<posit<32, 2>, MAX_ITERATIONS>(64);
	fdTest<posit<64, 3>, MAX_ITERATIONS>(64);
	fdTest<posit<128, 4>, MAX_ITERATIONS>(64);
	fdTest<posit<256, 5>, MAX_ITERATIONS>(64);

#if STRESS

#endif // STRESS

#endif // MANUAL

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
