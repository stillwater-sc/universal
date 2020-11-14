// cg_mvdot_cmpdot.cpp: multi-precision, preconditioned Conjugate Gradient iterative solver
// using matrix-vector dot product operator and compensation dot product operator
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
// and fast posits
//#define POSIT_FAST_SPECIALIZATION 1
#define POSIT_FAST_POSIT_32_2 1
#include <universal/posit/posit>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/blas/generators/fvm64x64.hpp>
#include <universal/blas/solvers/cg_dot_dot.hpp>

#define SOLUTION_FEEDBACK 0

// Finite Difference test: CG residual trajectory experiment for tridiag(-1, 2, -1)
template<typename Scalar, size_t MAX_ITERATIONS>
size_t fdTest(size_t DoF = 64) {
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	Matrix A = sw::unum::blas::tridiag<Scalar>(DoF);
	Vector b(DoF);
	Vector ones(DoF);
	ones = Scalar(1);
	b = A * ones;     // generate a known solution
	Matrix M = sw::unum::blas::inv(diag(diag(A)));
	Vector x(DoF);
	Vector residuals;
	size_t itr = sw::unum::blas::cg_dot_dot<Matrix, Vector, MAX_ITERATIONS>(M, A, b, x, residuals);
#if SOLUTION_FEEDBACK
	std::cout << "solution is " << x << '\n';
	std::cout << "final residual is " << residuals[size(residuals)-1] << '\n';
	std::cout << "validation\n" << A * x << " = " << b << '\n';
#endif
	std::cout << '\"' << typeid(Scalar).name() << "\" " << residuals << std::endl;

	return itr;
}

// Finite Volume test: CG residual trajectory experiment for a FVM test matrix
template<typename Scalar, size_t MAX_ITERATIONS>
size_t fvmTest() {
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	constexpr size_t DoF = 64;
	Matrix A = sw::unum::blas::fvm64x64<Scalar>();
	Vector b(DoF);
	Vector ones(DoF);
	ones = Scalar(1);
	b = A * ones;     // generate a known solution
	Matrix M = sw::unum::blas::inv(diag(diag(A)));
	Vector x(DoF);
	Vector residuals;
	size_t itr = sw::unum::blas::cg_dot_dot<Matrix, Vector, MAX_ITERATIONS>(M, A, b, x, residuals);
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

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	if (argc == 1) cout << argv[0] << '\n';
	int nrOfFailedTestCases = 0;

#if MANUAL
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits, es>;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	constexpr size_t DoF = 64;

	/*
	* for second order elliptical PDEs, the resulting coefficient matrix exhibits
	* a condition number k_2(A) = O(h^-2). Convergence rate of CG is sqrt(k_2)
	* so convergence is expected to be O(h^-1). 
	* The selected tridiagonal matrix has a discretization step given by DoF
	* and thus we expect the converge in sqrt(128) (h = 1/DoF -> h^-1 is Dof)
	*/
	constexpr size_t MAX_ITERATIONS = 100;
	size_t itr = fdTest<Scalar, MAX_ITERATIONS>(DoF);
	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}
	itr = fvmTest<Scalar, MAX_ITERATIONS>();
	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}

#else
	// with a preconditioner M = A^-1
	fdTest<float,200>(64);
	fdTest<double, 100>(64);
#if STRESS
	fdTest<posit<32,2>, 200>(64);
	fdTest<posit<64, 3>, 100>(64);
	fdTest<posit<128, 4>, 100>(64);
//	Experiment<posit<256, 5>, 100>(64);
#endif // STRESS

#endif // MANUAL

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