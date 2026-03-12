// ir.cpp: Iterative refinement with exact residual via quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ============================================================================
// Iterative Refinement with Exact Residuals
// ============================================================================
//
// Iterative refinement solves Ax = b by:
//   1. Compute an approximate solution x₀ via LU factorization
//   2. Compute the residual r = b − Ax  (the KEY step)
//   3. Solve the correction Ad = r
//   4. Update x ← x + d
//   5. Repeat until converged
//
// The critical insight: the residual r = b − Ax is a DOT PRODUCT (each
// component is b_i − sum_j(A_ij * x_j)). When the solution x is nearly
// correct, b − Ax involves catastrophic cancellation — exactly the problem
// the Kulisch super-accumulator was designed to solve.
//
// With naive floating-point, the residual is contaminated by rounding,
// and refinement stalls or oscillates. With the quire (fused dot product),
// the residual is computed exactly (single rounding), giving more accurate
// corrections and smoother convergence.
//
// This example uses:
//   - A tridiagonal matrix (-1, 2, -1) with κ ≈ 4n²/π²
//   - cfloat<32,8> (single-precision equivalent)
//   - Manufactured solution x* = [1, 2, 3, ..., n] for clear error measurement
//
// ============================================================================

#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

// ============================================================================
// Simple dense matrix/vector operations for self-contained example
// ============================================================================
template<typename T>
using Matrix = std::vector<std::vector<T>>;

template<typename T>
using Vector = std::vector<T>;

// Generate n×n tridiagonal matrix: A(i,i) = 2, A(i,i±1) = -1
// Condition number κ ≈ 4n²/π² (grows quadratically)
template<typename T>
Matrix<T> tridiagonal(size_t n) {
	Matrix<T> A(n, Vector<T>(n, T(0)));
	for (size_t i = 0; i < n; ++i) {
		A[i][i] = T(2);
		if (i > 0)     A[i][i - 1] = T(-1);
		if (i < n - 1) A[i][i + 1] = T(-1);
	}
	return A;
}

// Generate n×n matrix with controlled perturbation to create moderate condition number
// Base: tridiagonal + small dense perturbation to fill in off-diagonal terms
template<typename T>
Matrix<T> perturbed_dense(size_t n) {
	Matrix<T> A(n, Vector<T>(n, T(0)));
	for (size_t i = 0; i < n; ++i) {
		A[i][i] = T(double(n));  // strong diagonal
		for (size_t j = 0; j < n; ++j) {
			if (i != j) {
				// Small perturbation: creates a full matrix with moderate condition number
				A[i][j] = T(1.0 / double(1 + std::abs(int(i) - int(j))));
			}
		}
	}
	return A;
}

// Matrix-vector product: y = A*x (naive)
template<typename T>
Vector<T> matvec_naive(const Matrix<T>& A, const Vector<T>& x) {
	size_t n = A.size();
	Vector<T> y(n, T(0));
	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j < n; ++j)
			y[i] = y[i] + A[i][j] * x[j];
	return y;
}

// Residual r = b - A*x (naive)
template<typename T>
Vector<T> residual_naive(const Matrix<T>& A, const Vector<T>& x, const Vector<T>& b) {
	Vector<T> Ax = matvec_naive(A, x);
	Vector<T> r(b.size());
	for (size_t i = 0; i < b.size(); ++i)
		r[i] = b[i] - Ax[i];
	return r;
}

// Residual r = b - A*x using FDP for each row
// Computes b_i - sum(A_ij * x_j) as a single fused dot product
template<typename T>
Vector<T> residual_fdp(const Matrix<T>& A, const Vector<T>& x, const Vector<T>& b) {
	size_t n = A.size();
	Vector<T> r(n);
	for (size_t i = 0; i < n; ++i) {
		// Build augmented vectors: fdp([1, -A_i0, ..., -A_in], [b_i, x_0, ..., x_n])
		Vector<T> row_a(n + 1);
		Vector<T> row_b(n + 1);
		row_a[0] = T(1);   row_b[0] = b[i];
		for (size_t j = 0; j < n; ++j) {
			row_a[j + 1] = -A[i][j];
			row_b[j + 1] = x[j];
		}
		r[i] = sw::universal::fdp(row_a, row_b);
	}
	return r;
}

// LU factorization with partial pivoting (in-place, Doolittle)
template<typename T>
void lu_factor(Matrix<T>& A, std::vector<size_t>& piv) {
	size_t n = A.size();
	if (n == 0) throw std::invalid_argument("lu_factor: empty matrix");
	piv.resize(n);
	for (size_t i = 0; i < n; ++i) piv[i] = i;

	for (size_t k = 0; k < n - 1; ++k) {
		size_t argmax = k;
		double absmax = std::abs(double(A[k][k]));
		for (size_t i = k + 1; i < n; ++i) {
			double v = std::abs(double(A[i][k]));
			if (v > absmax) { absmax = v; argmax = i; }
		}
		if (argmax != k) {
			std::swap(piv[k], piv[argmax]);
			std::swap(A[k], A[argmax]);
		}
		if (absmax == 0.0)
			throw std::runtime_error("lu_factor: singular matrix");
		for (size_t i = k + 1; i < n; ++i) {
			A[i][k] = A[i][k] / A[k][k];
			for (size_t j = k + 1; j < n; ++j)
				A[i][j] = A[i][j] - A[i][k] * A[k][j];
		}
	}
}

// Solve LUx = Pb (forward + back substitution)
template<typename T>
Vector<T> lu_solve(const Matrix<T>& LU, const std::vector<size_t>& piv, const Vector<T>& b) {
	size_t n = b.size();

	Vector<T> pb(n);
	for (size_t i = 0; i < n; ++i) pb[i] = b[piv[i]];

	// Forward substitution (unit lower triangular)
	Vector<T> y(n);
	for (size_t i = 0; i < n; ++i) {
		T s = pb[i];
		for (size_t j = 0; j < i; ++j)
			s = s - LU[i][j] * y[j];
		y[i] = s;
	}

	// Back substitution (upper triangular)
	Vector<T> x(n);
	for (size_t i = n; i > 0; --i) {
		size_t ii = i - 1;
		T s = y[ii];
		for (size_t j = ii + 1; j < n; ++j)
			s = s - LU[ii][j] * x[j];
		x[ii] = s / LU[ii][ii];
	}
	return x;
}

// Forward error: max_i |x_exact_i - x_i| / max_i |x_exact_i|
template<typename T>
double forward_error(const Vector<T>& x, const Vector<T>& x_exact) {
	double max_err = 0, max_val = 0;
	for (size_t i = 0; i < x.size(); ++i) {
		double d = std::abs(double(x_exact[i]) - double(x[i]));
		double v = std::abs(double(x_exact[i]));
		if (d > max_err) max_err = d;
		if (v > max_val) max_val = v;
	}
	return max_val > 0 ? max_err / max_val : max_err;
}

// ============================================================================
// Iterative refinement comparing naive vs FDP residual computation
// ============================================================================
template<typename Scalar>
void RunIterativeRefinement(const std::string& matrix_name,
                            const Matrix<Scalar>& A_orig,
                            const Vector<Scalar>& x_exact,
                            int max_iter) {
	size_t n = A_orig.size();
	Vector<Scalar> b = matvec_naive(A_orig, x_exact);

	// LU factorization
	Matrix<Scalar> LU = A_orig;
	std::vector<size_t> piv;
	lu_factor(LU, piv);

	// Initial solve
	Vector<Scalar> x_naive = lu_solve(LU, piv, b);
	Vector<Scalar> x_fdp   = lu_solve(LU, piv, b);

	std::cout << "\n  " << matrix_name << ", n = " << n << "\n";
	std::cout << "    " << std::setw(6) << "Iter"
	          << std::setw(18) << "Naive ||e||/||x*||"
	          << std::setw(18) << "FDP ||e||/||x*||" << "\n";
	std::cout << "    " << std::string(42, '-') << "\n";

	for (int iter = 0; iter <= max_iter; ++iter) {
		double err_naive = forward_error(x_naive, x_exact);
		double err_fdp   = forward_error(x_fdp, x_exact);

		std::cout << "    " << std::setw(6) << iter
		          << std::setw(18) << std::scientific << std::setprecision(4) << err_naive
		          << std::setw(18) << err_fdp << "\n";

		if (iter == max_iter) break;
		if (err_naive < 1e-7 && err_fdp < 1e-7) break;

		// Naive refinement step
		{
			Vector<Scalar> r = residual_naive(A_orig, x_naive, b);
			Vector<Scalar> d = lu_solve(LU, piv, r);
			for (size_t i = 0; i < n; ++i)
				x_naive[i] = x_naive[i] + d[i];
		}
		// FDP refinement step
		{
			Vector<Scalar> r = residual_fdp(A_orig, x_fdp, b);
			Vector<Scalar> d = lu_solve(LU, piv, r);
			for (size_t i = 0; i < n; ++i)
				x_fdp[i] = x_fdp[i] + d[i];
		}
	}
}

// Regression testing guards
#ifndef MANUAL_TESTING
#define MANUAL_TESTING 0
#endif
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "Iterative refinement — quire exact residuals";
	std::string test_tag   = "ir";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	std::cout << "============================================================\n";
	std::cout << "Iterative Refinement: Naive vs FDP Residual (cfloat<32,8>)\n";
	std::cout << "============================================================\n";
	std::cout << "\nThe residual r = b - Ax is a dot product. When x is nearly\n";
	std::cout << "correct, this dot product suffers catastrophic cancellation.\n";
	std::cout << "The quire (FDP) computes the residual exactly, providing\n";
	std::cout << "more accurate corrections and smoother convergence.\n";

	// Test 1: Tridiagonal matrix, moderate condition number
	// κ ≈ 4n²/π² ≈ 405 for n=10, ≈ 10132 for n=50
	{
		size_t n = 50;
		Matrix<Scalar> A = tridiagonal<Scalar>(n);
		Vector<Scalar> x_exact(n);
		for (size_t i = 0; i < n; ++i) x_exact[i] = Scalar(double(i + 1));
		RunIterativeRefinement("Tridiagonal (-1,2,-1)", A, x_exact, 8);
	}

	// Test 2: Dense diagonally-dominant matrix
	{
		size_t n = 30;
		Matrix<Scalar> A = perturbed_dense<Scalar>(n);
		Vector<Scalar> x_exact(n);
		for (size_t i = 0; i < n; ++i) x_exact[i] = Scalar(double(i + 1));
		RunIterativeRefinement("Dense diag-dominant", A, x_exact, 8);
	}

	// Test 3: Larger tridiagonal to show scaling
	{
		size_t n = 100;
		Matrix<Scalar> A = tridiagonal<Scalar>(n);
		Vector<Scalar> x_exact(n);
		for (size_t i = 0; i < n; ++i) x_exact[i] = Scalar(double(i + 1));
		RunIterativeRefinement("Tridiagonal (-1,2,-1)", A, x_exact, 10);
	}

	std::cout << "\nLesson: The quire provides exact residual computation via FDP.\n";
	std::cout << "For moderately ill-conditioned systems, FDP-based iterative\n";
	std::cout << "refinement converges more smoothly and achieves better accuracy\n";
	std::cout << "than naive residual computation. The difference grows with\n";
	std::cout << "problem size and condition number.\n\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Verify that FDP-based IR converges on tridiagonal(20)
	{
		using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
		size_t n = 20;
		Matrix<Scalar> A = tridiagonal<Scalar>(n);
		Vector<Scalar> x_exact(n);
		for (size_t i = 0; i < n; ++i) x_exact[i] = Scalar(double(i + 1));
		Vector<Scalar> b = matvec_naive(A, x_exact);

		Matrix<Scalar> LU = A;
		std::vector<size_t> piv;
		lu_factor(LU, piv);
		Vector<Scalar> x = lu_solve(LU, piv, b);

		// 5 iterations of FDP refinement
		for (int iter = 0; iter < 5; ++iter) {
			Vector<Scalar> r = residual_fdp(A, x, b);
			Vector<Scalar> d = lu_solve(LU, piv, r);
			for (size_t i = 0; i < n; ++i)
				x[i] = x[i] + d[i];
		}

		double fwd_err = forward_error(x, x_exact);
		if (fwd_err > 1e-4) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: FDP IR on tridiag(20), fwd_err = " << fwd_err << '\n';
		}
	}
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
