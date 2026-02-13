// cg.cpp: multi-precision, preconditioned Conjugate Gradient iterative solver using Fused Dot Products
// using matrix-vector fused dot product operator, and compensation fused dot product operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// Authors: Theodore Omtzigt
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// standard library
#include <limits>
// Configure the posit library with arithmetic exceptions
// disable posit arithmetic exceptions: CG solver may encounter
// divide-by-zero when low-precision posits cause stalling
//#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// and fast posits
//#define POSIT_FAST_SPECIALIZATION 1
#define POSIT_FAST_POSIT_32_2 1
#include <universal/number/posit/posit.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>
#include <blas/solvers.hpp>
#include <universal/verification/test_suite.hpp>

// CG residual trajectory experiment for tridiag(-1, 2, -1)
template<typename Scalar, size_t MAX_ITERATIONS = 100>
size_t Experiment(size_t DoF) {
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	using Matrix = matrix<Scalar>;
	using Vector = vector<Scalar>;

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	Matrix A = tridiag<Scalar>(DoF);
	Vector b(DoF);
	Vector ones(DoF);
	ones = Scalar(1);
	b = A * ones;     // generate a known solution
	Matrix M = inv(diag(diag(A)));
	Vector x(DoF);
	Vector residuals;
	size_t itr = solvers::cg<Matrix, Vector, MAX_ITERATIONS>(M, A, b, x, residuals);
	//	std::cout << "solution is " << x << '\n';
	//	std::cout << "final residual is " << residual << '\n';
	//	std::cout << "validation\n" << A * x << " = " << b << '\n';
	std::cout << '\"' << typeid(Scalar).name() << "\" " << residuals << std::endl;

	return itr;
}

template<typename Scalar>
int VerifyCG(bool reportTestCases) {
	if (reportTestCases) std::cerr << "ignoring testcases\n";
	return 0;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;
	using namespace sw::blas::solvers;

	std::string test_suite         = "mixed-precision CG metho";
	std::string test_tag           = "cg";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits, es>;
	using Matrix = matrix<Scalar>;
	using Vector = vector<Scalar>;

	// Initialize 'A', preconditioner 'M', 'b' & intial guess 'x' * _
	constexpr size_t DoF = 8;
	Matrix A = tridiag<Scalar>(DoF);
	// Matrix M = eye<Scalar>(DoF); // M = I, unpreconditioned
	Matrix M = inv(diag(diag(A)));  // Jacobi preconditioner for positive-definite, diagonally dominant systems
	Vector b(DoF);
	Vector ones(DoF);
	ones = Scalar(1);
	b = A * ones;

	if (DoF < 10) {
		cout << "M^-1:\n" << M << endl;  // it is customary to talk about the preconditioner M while understanding that it really is the inverse M^-1
		cout << "A:\n" << A << endl;
		cout << "x:\n" << ones << endl;
		cout << "b:\n" << b << endl;
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
	size_t itr = cg<Matrix, Vector, MAX_ITERATIONS>(M, A, b, x, residuals);
	std::cout << "solution is " << x << '\n';
	std::cout << "final residual is " << residuals[size(residuals) - 1] << '\n';
	std::cout << "validation\n" << A * x << " = " << b << '\n';
	std::cout << typeid(Scalar).name() << " " << residuals << std::endl;
	if (itr == MAX_ITERATIONS) {
		std::cerr << "Solution failed to converge\n";
		++nrOfFailedTestCases;
	}

#else
	// with a preconditioner M = Jacobian^-1
	Experiment<float>(64);
	Experiment<double>(64);
	Experiment<long double>(64);

	Experiment<posit<16,1>>(64);
	Experiment<posit<20, 1>>(64);
	Experiment<posit<24, 1>>(64);
	Experiment<posit<28, 1>>(64);
	Experiment<posit<32,2>>(64);


/* results
* Native IEEE floating point
solution in 34 iterations
"float" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0.999999 1 0.999999 1 0.999999 33 3.83854e-05 1.43051e-06
solution in 33 iterations
"double" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 33 3.4972e-14
solution in 33 iterations
"long double" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 33 3.4972e-14

Different posit configurations
solution in 79 iterations
"class sw::universal::posit<16,1>" 1 0.999878 1 0.999878 1 1.00049 0.99939 1 1.00098 1.00171 1.00244 1.00098 0.998657 0.997803 0.999512 1.00244 1.00269 1.00488 1.00537 1.00781 1.00928 1.01123 1.01025 1.0105 1.01416 1.01416 1.01611 1.02197 1.02661 1.02441 1.01709 28.3594 4.89062 0.0266113 0.0319824 0.0200195 0.0117188 0.00585938 0.00488281 0.00585938 0.00292969 0.00341797 0.00341797 0.00244141 0.00244141 0.00341797 0.00537109 0.00732422 0.0180664 0.00927734 0.0209961 0.0195312 0.0102539 0.000976562 0.00146484 0.00292969 0.00341797 0.0117188 0.00976562 0.00390625 0.00439453 0.0117188 0.0166016 0.0185547 0.0180664 0.0151367 0.0131836 0.0078125 0.0136719 0.0161133 0.0175781 0.0175781 0.0151367 0.0112305 0.00830078 0.00830078 0.0078125 0.000488281 0
solution in 57 iterations
"class sw::universal::posit<20,1>" 1 0.999992 0.999985 1.00002 1.00003 1 0.999939 0.999954 0.999916 1 1 0.999855 0.999832 0.999924 0.999939 1.00006 1.00006 1.00018 1.00046 1.00058 1.00035 1.00034 1.0004 1.00037 1.00034 1.00012 1.00003 0.999939 0.999863 0.999496 0.999321 32.9648 0.0302734 0.00144958 0.00132751 0.000656128 0.000457764 0.000839233 0.000473022 0.000732422 0.000686646 0.000564575 0.000411987 0.000732422 0.000762939 0.00109863 0.00164795 0.0010376 0.00038147 0.000350952 0.000335693 0.000320435 0.000289917 0.000137329 0.000198364 1.52588e-05 0
solution in 40 iterations
"class sw::universal::posit<24,1>" 1 1 1 1 1 1 1 1 1 1 1.00001 1.00002 1.00002 1.00002 1.00002 1 1 0.999993 0.999993 1 0.99999 0.999987 0.999988 0.999995 0.999999 1 1.00001 1.00003 1.00003 1.00002 1.00003 33.0026 0.0011673 8.01086e-05 5.91278e-05 3.62396e-05 3.24249e-05 2.47955e-05 2.67029e-05 5.72205e-06
solution in 34 iterations
"class sw::universal::posit<28,1>" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0.999999 0.999999 1 1 1 1 1 1 1 1 1 0.999999 1 1 0.999999 0.999999 1 33 9.94205e-05 5.48363e-06
solution in 33 iterations
"class sw::universal::posit<32,2>" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 33 1.8999e-06
solution in 33 iterations
"class sw::universal::posit<64,3>" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 33 3.85109e-16
solution in 33 iterations
"class sw::universal::posit<128,4>" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 33 1.44445e-34
solution in 33 iterations
"class sw::universal::posit<256,5>" 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 33 4.02376e-73

The posit with nbits = 28 is a functional replacement for IEEE single precision floats
*/

#if STRESS_TESTING
	Experiment<posit<64,3>>(64);
	Experiment<posit<128,4>>(64);
	Experiment<posit<256,5>>(64);
#endif // STRESS_TESTING

	using Scalar = float;
	nrOfFailedTestCases += ReportTestResult(VerifyCG<Scalar>(reportTestCases), "tag", "test_id");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
