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

template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const std::vector<Scalar>& vec) {
	for (auto v : vec) {
		ostr << ' ' << v;
	}
	return ostr;
}

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
size_t cg_dot_dot(const Matrix& M, const Matrix& A, Vector& b, typename Matrix::value_type tolerance = typename Matrix::value_type(0.00001)) {
	using Scalar = typename Matrix::value_type;
	Scalar residual = Scalar(std::numeric_limits<Scalar>::max());
	std::vector<Scalar> residuals; // store the residual trajectory
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
		matvec(q, A, p);  // regular matrix-vector without quire
		alpha = sigma_1 / dot(p, q);
		Vector x_1(x);
		x = x + alpha * p;
		rho = rho - alpha * q;
		// check for convergence of the system
		residual = norm1(x_1 - x);
//		std::cout << '[' << itr << "] " << std::setw(12) << x << " residual " << residual << std::endl;
		residuals.push_back(residual);
		++itr;
	}
	if (residual < tolerance) {
		std::cout << "solution in " << itr << " iterations\n";
	}
	else {
		std::cout << "failed to converge in " << itr << " iterations\n";
	}
//	std::cout << "solution is " << x << '\n';
//	std::cout << "final residual is " << residual << '\n';
//	std::cout << "validation\n" << A * x << " = " << b << '\n';
	std::cout << typeid(Scalar).name() << " " << residuals << std::endl;
	return itr;
}

template<typename Scalar, size_t MAX_ITERATIONS>
size_t Experiment(size_t DoF) {
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;
	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _

	Matrix A;
	tridiag(A, DoF);  // this does a resize of A
	Matrix M = sw::unum::blas::eye<Scalar>(DoF);
	Vector b(DoF);
	Vector x(DoF);
	x = Scalar(1);
	b = A * x;

	M = sw::unum::blas::inv(A);
	return cg_dot_dot<Matrix, Vector, MAX_ITERATIONS>(M, A, b);
	
}

#define MANUAL 0
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
//	using Scalar = float;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;


	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	constexpr size_t DoF = 8;
	Matrix A;
	tridiag(A, DoF);  // this does a resize of A
	Matrix M = eye<Scalar>(DoF);
	Vector b(DoF);
	Vector x(DoF);
	x = Scalar(1);
	b = A * x;

	if (DoF < 10) {
		cout << A << endl;
		cout << M << endl;
		cout << b << endl;
	}
	/*
	* for second order elliptical PDEs, the resulting coefficient matrix exhibits
	* a condition number k_2(A) = O(h^-2). Convergence rate of CG is sqrt(k_2)
	* so convergence is expected to be O(h^-1). 
	* The selected tridiagonal matrix has a discretization step given by DoF
	* and thus we expect the converge in sqrt(128) (h = 1/DoF -> h^-1 is Dof)
	*/
	constexpr size_t MAX_ITERATIONS = 100;
	size_t itr = cg_dot_dot<Matrix, Vector, MAX_ITERATIONS>(M, A, b);

	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}

#else
	// with a preconditioner M = A^-1
	Experiment<float,200>(64);
	Experiment<double, 100>(64);
#if STRESS
	Experiment<posit<32,2>, 200>(64);
	Experiment<posit<64, 3>, 100>(64);
	Experiment<posit<128, 4>, 100>(64);
	Experiment<posit<256, 5>, 100>(64);
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
