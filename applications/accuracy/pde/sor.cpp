// sor.cpp: Successive Over-Relaxation (SOR) iterative method
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
#include <universal/number/posit/posit.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>

template<typename Matrix, typename Vector>
void report(const Matrix& A, const Vector& b, const Vector& x, size_t itr, typename Vector::value_type& w) {
	std::cout << "solution in " << itr << " iterations\n";
	std::cout << "solution is " << x << '\n';

	std::cout << "validation\n" << A * x << " = " << b << std::endl;
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using namespace sw::blas;
	using namespace sw::blas::solvers;
	using namespace sw::numeric::containers;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits, es>;
//	using Scalar = float;
	using Matrix = matrix<Scalar>;
	using Vector = vector<Scalar>;

	if (argc == 1) std::cout << argv[0] << '\n';
	int nrOfFailedTestCases = 0;

	// Initialize 'A' 'b' & intial guess 'x' * _
	Matrix A = { 
		{ 5, -2,  3,  0},
		{-3,  9,  1, -2},
		{ 2, -1, -7,  1},
		{ 4,  3, -5,  7} };
	Vector b = { -1, 2, 3, 0.5 };
	Vector x = {  0, 0, 0, 0 };
	Scalar w = 1.5f;
	size_t itr = 0;

	std::cout << A << std::endl;
	std::cout << b << std::endl;

	itr = sor(A, b, x, w); 
	report(A, b, x, itr, w);
	w = 1.25f;
	itr = sor(A, b, x, w); 
	report(A, b, x, itr, w);

	w = 1.125f;
	itr = sor(A, b, x, w); 
	report(A, b, x, itr, w);

	w = 1.0625f;
	itr = sor(A, b, x, w); 
	report(A, b, x, itr, w);

	//  in matrix form
	tridiag(A, 5);
	std::cout << A << std::endl;
	auto diagonal = diag(A);
	std::cout << "\nDiagonal vector\n" << diagonal << std::endl;
	auto D = diag(diag(A));
	auto L = tril(A) - D;
	auto U = triu(A) - D;
	auto B = (D + w * L);

	// check for convergence of the system
	auto e = 0.95; //  max(eig(inv(D + w * L) * (D * (1 - w) - w * U)));
	if (std::abs(e) >= 1) {
		std::cerr << "Not convergent: modulus of the largest eigen value is >= 1\n";
		return EXIT_FAILURE;
	}

	std::cout << "Inverse of (D + w*L)\n" << inv(B) << std::endl;

	// TODO eig() operator

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
