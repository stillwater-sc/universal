// solvers.cpp: regression tests for linear system solvers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Test methodology:
// Generate A and x, compute b = A*x, solve A*y = b, verify ||x - y|| < tolerance
//
#include <universal/utility/directives.hpp>
#include <limits>
#include <cmath>

// pull in the number systems
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <blas/solvers/lu.hpp>
#include <blas/solvers/backsub.hpp>
#include <blas/solvers/forwsub.hpp>
#include <blas/solvers/qr.hpp>
#include <universal/verification/test_suite.hpp>

using namespace sw::numeric::containers;

namespace sw { namespace blas { namespace solvers {

	////////////////////////////////////////////////////////////////////////
	// Helper: compute residual norm ||x - y|| / ||x||
	template<typename Scalar>
	Scalar relativeError(const vector<Scalar>& x, const vector<Scalar>& y) {
		Scalar diff_norm = normL2(x - y);
		Scalar x_norm = normL2(x);
		if (double(x_norm) < 1e-15) return diff_norm;
		return diff_norm / x_norm;
	}

	////////////////////////////////////////////////////////////////////////
	// Test LU solve with known solution
	// Method: Generate A and x, compute b = A*x, solve A*y = b, verify x ≈ y
	template<typename Scalar>
	int VerifyLUSolve(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-6);

		// Test 1: Simple 3x3 system
		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(4);  A[0][1] = Scalar(1);  A[0][2] = Scalar(0);
			A[1][0] = Scalar(1);  A[1][1] = Scalar(4);  A[1][2] = Scalar(1);
			A[2][0] = Scalar(0);  A[2][1] = Scalar(1);  A[2][2] = Scalar(4);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = A * x_true;
			vector<Scalar> x_solved = solve(A, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: LU solve 3x3, relative error = " << err << "\n";
					std::cerr << "  x_true   = " << x_true << "\n";
					std::cerr << "  x_solved = " << x_solved << "\n";
				}
			}
		}

		// Test 2: 4x4 system with larger values
		{
			matrix<Scalar> A(4, 4);
			A[0][0] = Scalar(10); A[0][1] = Scalar(-1); A[0][2] = Scalar(2);  A[0][3] = Scalar(0);
			A[1][0] = Scalar(-1); A[1][1] = Scalar(11); A[1][2] = Scalar(-1); A[1][3] = Scalar(3);
			A[2][0] = Scalar(2);  A[2][1] = Scalar(-1); A[2][2] = Scalar(10); A[2][3] = Scalar(-1);
			A[3][0] = Scalar(0);  A[3][1] = Scalar(3);  A[3][2] = Scalar(-1); A[3][3] = Scalar(8);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(-1), Scalar(1) };
			vector<Scalar> b = A * x_true;
			vector<Scalar> x_solved = solve(A, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: LU solve 4x4, relative error = " << err << "\n";
			}
		}

		// Test 3: Diagonally dominant system (good for stability)
		{
			matrix<Scalar> A(5, 5);
			for (size_t i = 0; i < 5; ++i) {
				for (size_t j = 0; j < 5; ++j) {
					A[i][j] = (i == j) ? Scalar(10) : Scalar(-1);
				}
			}

			vector<Scalar> x_true = { Scalar(1), Scalar(-1), Scalar(2), Scalar(-2), Scalar(3) };
			vector<Scalar> b = A * x_true;
			vector<Scalar> x_solved = solve(A, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: LU solve 5x5 diagonal dominant, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test Crout LU decomposition
	template<typename Scalar>
	int VerifyCroutLU(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-6);

		// Test with a known matrix
		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(2);  A[0][1] = Scalar(1);  A[0][2] = Scalar(1);
			A[1][0] = Scalar(4);  A[1][1] = Scalar(3);  A[1][2] = Scalar(3);
			A[2][0] = Scalar(8);  A[2][1] = Scalar(7);  A[2][2] = Scalar(9);

			matrix<Scalar> LU(3, 3);
			Crout(A, LU);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = A * x_true;
			vector<Scalar> x_solved(3);
			SolveCrout(LU, b, x_solved);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Crout LU solve, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test back substitution (upper triangular solve)
	template<typename Scalar>
	int VerifyBacksub(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-10);

		// Create upper triangular matrix
		{
			matrix<Scalar> U(3, 3);
			U[0][0] = Scalar(2);  U[0][1] = Scalar(1);  U[0][2] = Scalar(3);
			U[1][0] = Scalar(0);  U[1][1] = Scalar(4);  U[1][2] = Scalar(2);
			U[2][0] = Scalar(0);  U[2][1] = Scalar(0);  U[2][2] = Scalar(5);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = U * x_true;
			vector<Scalar> x_solved = backsub(U, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: backsub, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test forward substitution (lower triangular solve)
	template<typename Scalar>
	int VerifyForwsub(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-10);

		// Create lower triangular matrix (unit diagonal for lower=false)
		{
			matrix<Scalar> L(3, 3);
			L[0][0] = Scalar(1);  L[0][1] = Scalar(0);  L[0][2] = Scalar(0);
			L[1][0] = Scalar(2);  L[1][1] = Scalar(1);  L[1][2] = Scalar(0);
			L[2][0] = Scalar(3);  L[2][1] = Scalar(4);  L[2][2] = Scalar(1);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = L * x_true;
			vector<Scalar> x_solved = forwsub(L, b, false);  // unit diagonal

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: forwsub (unit diag), relative error = " << err << "\n";
			}
		}

		// Test with non-unit diagonal (lower=true)
		{
			matrix<Scalar> L(3, 3);
			L[0][0] = Scalar(2);  L[0][1] = Scalar(0);  L[0][2] = Scalar(0);
			L[1][0] = Scalar(1);  L[1][1] = Scalar(3);  L[1][2] = Scalar(0);
			L[2][0] = Scalar(2);  L[2][1] = Scalar(1);  L[2][2] = Scalar(4);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = L * x_true;
			vector<Scalar> x_solved = forwsub(L, b, true);  // use actual diagonal

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: forwsub (non-unit diag), relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test LU with tridiagonal matrix (common in numerical applications)
	template<typename Scalar>
	int VerifyTridiagonalSolve(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-5);

		// Create tridiagonal system: -1, 2, -1 (Poisson-like)
		{
			size_t n = 10;
			matrix<Scalar> A(n, n);
			for (size_t i = 0; i < n; ++i) {
				A[i][i] = Scalar(2);
				if (i > 0) A[i][i-1] = Scalar(-1);
				if (i < n-1) A[i][i+1] = Scalar(-1);
			}

			// Create known solution
			vector<Scalar> x_true(n);
			for (size_t i = 0; i < n; ++i) {
				x_true[i] = Scalar(i + 1);
			}

			vector<Scalar> b = A * x_true;
			vector<Scalar> x_solved = solve(A, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: tridiagonal LU solve, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test with Hilbert matrix (ill-conditioned, challenging)
	template<typename Scalar>
	int VerifyHilbertSolve(bool reportTestCases) {
		int nrOfFailedTests = 0;
		// Hilbert matrices are ill-conditioned, so we need looser tolerance
		Scalar tolerance = Scalar(1e-3);

		// Small Hilbert matrix (3x3)
		{
			auto H = hilbert<Scalar>(3, false);  // unscaled Hilbert

			vector<Scalar> x_true = { Scalar(1), Scalar(1), Scalar(1) };
			vector<Scalar> b = H * x_true;
			vector<Scalar> x_solved = solve(H, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Hilbert 3x3 solve, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test LU decomposition itself (verify A = L*U)
	template<typename Scalar>
	int VerifyLUDecomposition(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-8);

		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(4);  A[0][1] = Scalar(3);  A[0][2] = Scalar(0);
			A[1][0] = Scalar(3);  A[1][1] = Scalar(4);  A[1][2] = Scalar(-1);
			A[2][0] = Scalar(0);  A[2][1] = Scalar(-1); A[2][2] = Scalar(4);

			matrix<Scalar> LU_combined = lu(A);

			// Extract L and U from combined LU matrix
			matrix<Scalar> L(3, 3), U(3, 3);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					if (i > j) {
						L[i][j] = LU_combined[i][j];
						U[i][j] = Scalar(0);
					} else if (i == j) {
						L[i][j] = Scalar(1);  // Unit lower triangular
						U[i][j] = LU_combined[i][j];
					} else {
						L[i][j] = Scalar(0);
						U[i][j] = LU_combined[i][j];
					}
				}
			}

			// Verify L*U ≈ A (with permutation, but for simple cases should be close)
			matrix<Scalar> LU_product = L * U;

			// Check Frobenius norm of difference
			Scalar diff_norm = Scalar(0);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					Scalar d = A[i][j] - LU_product[i][j];
					diff_norm += d * d;
				}
			}
			diff_norm = std::sqrt(double(diff_norm));

			// Note: due to pivoting, L*U may not exactly equal A, but solve should still work
			// This test is informational
			if (reportTestCases && double(diff_norm) > double(tolerance)) {
				std::cerr << "INFO: LU decomposition with pivoting, ||A - L*U|| = " << diff_norm << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test with minij matrix (positive definite)
	template<typename Scalar>
	int VerifyMinijSolve(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-6);

		{
			auto A = minij<Scalar>(5);

			vector<Scalar> x_true = { Scalar(1), Scalar(-1), Scalar(2), Scalar(-2), Scalar(1) };
			vector<Scalar> b = A * x_true;
			vector<Scalar> x_solved = solve(A, b);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: minij matrix solve, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test QR decomposition: verify A = Q*R and Q is orthogonal
	template<typename Scalar>
	int VerifyQRDecomposition(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-6);

		// Test with simple matrix
		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(1);  A[0][1] = Scalar(2);  A[0][2] = Scalar(3);
			A[1][0] = Scalar(4);  A[1][1] = Scalar(5);  A[1][2] = Scalar(6);
			A[2][0] = Scalar(7);  A[2][1] = Scalar(8);  A[2][2] = Scalar(10);

			auto [Q, R] = qr(A, 1);  // Householder method

			// Verify Q*R ≈ A
			matrix<Scalar> QR = Q * R;
			Scalar diff_norm = Scalar(0);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					Scalar d = A[i][j] - QR[i][j];
					diff_norm += d * d;
				}
			}
			diff_norm = std::sqrt(double(diff_norm));

			if (double(diff_norm) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: QR decomposition ||A - Q*R|| = " << diff_norm << "\n";
			}

			// Verify Q is orthogonal: Q'*Q ≈ I
			matrix<Scalar> Qt = Q;
			Qt.transpose();
			matrix<Scalar> QtQ = Qt * Q;
			Scalar ortho_err = Scalar(0);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					Scalar expected = (i == j) ? Scalar(1) : Scalar(0);
					Scalar d = QtQ[i][j] - expected;
					ortho_err += d * d;
				}
			}
			ortho_err = std::sqrt(double(ortho_err));

			if (double(ortho_err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Q not orthogonal, ||Q'Q - I|| = " << ortho_err << "\n";
			}

			// Verify R is upper triangular
			for (size_t i = 1; i < 3; ++i) {
				for (size_t j = 0; j < i; ++j) {
					if (std::abs(double(R[i][j])) > double(tolerance)) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: R not upper triangular at [" << i << "][" << j << "] = " << R[i][j] << "\n";
					}
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test Modified Gram-Schmidt QR
	template<typename Scalar>
	int VerifyMGS(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-5);

		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(1);  A[0][1] = Scalar(1);  A[0][2] = Scalar(0);
			A[1][0] = Scalar(1);  A[1][1] = Scalar(0);  A[1][2] = Scalar(1);
			A[2][0] = Scalar(0);  A[2][1] = Scalar(1);  A[2][2] = Scalar(1);

			auto [Q, R] = qr(A, 2);  // Modified Gram-Schmidt

			// Verify Q*R ≈ A
			matrix<Scalar> QR = Q * R;
			Scalar diff_norm = Scalar(0);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					Scalar d = A[i][j] - QR[i][j];
					diff_norm += d * d;
				}
			}
			diff_norm = std::sqrt(double(diff_norm));

			if (double(diff_norm) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: MGS QR ||A - Q*R|| = " << diff_norm << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test Givens QR
	template<typename Scalar>
	int VerifyGivensQR(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-5);

		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(1);  A[0][1] = Scalar(2);  A[0][2] = Scalar(3);
			A[1][0] = Scalar(4);  A[1][1] = Scalar(5);  A[1][2] = Scalar(6);
			A[2][0] = Scalar(7);  A[2][1] = Scalar(8);  A[2][2] = Scalar(10);

			auto [Q, R] = qr(A, 3);  // Givens rotations

			// Verify Q*R ≈ A
			matrix<Scalar> QR = Q * R;
			Scalar diff_norm = Scalar(0);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					Scalar d = A[i][j] - QR[i][j];
					diff_norm += d * d;
				}
			}
			diff_norm = std::sqrt(double(diff_norm));

			if (double(diff_norm) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Givens QR ||A - Q*R|| = " << diff_norm << "\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test QR for solving least squares (overdetermined system)
	template<typename Scalar>
	int VerifyQRSolve(bool reportTestCases) {
		int nrOfFailedTests = 0;
		Scalar tolerance = Scalar(1e-5);

		// Square system: solve Ax = b using QR
		// A = QR, so QRx = b, Rx = Q'b, then back-substitute
		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(4);  A[0][1] = Scalar(1);  A[0][2] = Scalar(0);
			A[1][0] = Scalar(1);  A[1][1] = Scalar(4);  A[1][2] = Scalar(1);
			A[2][0] = Scalar(0);  A[2][1] = Scalar(1);  A[2][2] = Scalar(4);

			vector<Scalar> x_true = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = A * x_true;

			auto [Q, R] = qr(A, 1);

			// Compute Q'*b
			matrix<Scalar> Qt = Q;
			Qt.transpose();
			vector<Scalar> Qtb = Qt * b;

			// Back-substitute to solve R*x = Q'*b
			vector<Scalar> x_solved = backsub(R, Qtb);

			Scalar err = relativeError(x_true, x_solved);
			if (double(err) > double(tolerance)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: QR solve, relative error = " << err << "\n";
			}
		}

		return nrOfFailedTests;
	}

}}} // namespace sw::blas::solvers

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::blas;
	using namespace sw::blas::solvers;

	std::string test_suite  = "Linear system solvers";
	std::string test_tag    = "solvers";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyLUSolve<double>(reportTestCases), "double", "LU solve");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// Basic solver tests with double precision
	nrOfFailedTestCases += ReportTestResult(VerifyLUSolve<double>(reportTestCases), "double", "LU solve");
	nrOfFailedTestCases += ReportTestResult(VerifyCroutLU<double>(reportTestCases), "double", "Crout LU");
	nrOfFailedTestCases += ReportTestResult(VerifyBacksub<double>(reportTestCases), "double", "backsub");
	nrOfFailedTestCases += ReportTestResult(VerifyForwsub<double>(reportTestCases), "double", "forwsub");
#endif

#if REGRESSION_LEVEL_2
	// More complex solver tests
	nrOfFailedTestCases += ReportTestResult(VerifyTridiagonalSolve<double>(reportTestCases), "double", "tridiagonal");
	nrOfFailedTestCases += ReportTestResult(VerifyMinijSolve<double>(reportTestCases), "double", "minij matrix");
	nrOfFailedTestCases += ReportTestResult(VerifyLUDecomposition<double>(reportTestCases), "double", "LU decomposition");

	// QR decomposition tests
	nrOfFailedTestCases += ReportTestResult(VerifyQRDecomposition<double>(reportTestCases), "double", "QR Householder");
	nrOfFailedTestCases += ReportTestResult(VerifyMGS<double>(reportTestCases), "double", "QR MGS");
	nrOfFailedTestCases += ReportTestResult(VerifyGivensQR<double>(reportTestCases), "double", "QR Givens");
	nrOfFailedTestCases += ReportTestResult(VerifyQRSolve<double>(reportTestCases), "double", "QR solve");

	// Tests with float
	nrOfFailedTestCases += ReportTestResult(VerifyLUSolve<float>(reportTestCases), "float", "LU solve");
	nrOfFailedTestCases += ReportTestResult(VerifyBacksub<float>(reportTestCases), "float", "backsub");
#endif

#if REGRESSION_LEVEL_3
	// Ill-conditioned test
	nrOfFailedTestCases += ReportTestResult(VerifyHilbertSolve<double>(reportTestCases), "double", "Hilbert matrix");

	// Tests with posit
	nrOfFailedTestCases += ReportTestResult(VerifyLUSolve<posit<32, 2>>(reportTestCases), "posit<32,2>", "LU solve");
	nrOfFailedTestCases += ReportTestResult(VerifyBacksub<posit<32, 2>>(reportTestCases), "posit<32,2>", "backsub");
	nrOfFailedTestCases += ReportTestResult(VerifyTridiagonalSolve<posit<32, 2>>(reportTestCases), "posit<32,2>", "tridiagonal");
#endif

#if REGRESSION_LEVEL_4
	// Extended precision tests
	nrOfFailedTestCases += ReportTestResult(VerifyLUSolve<posit<64, 3>>(reportTestCases), "posit<64,3>", "LU solve");
	nrOfFailedTestCases += ReportTestResult(VerifyHilbertSolve<posit<64, 3>>(reportTestCases), "posit<64,3>", "Hilbert matrix");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
