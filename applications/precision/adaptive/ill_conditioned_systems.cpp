// ill_conditioned_systems.cpp: solve an ill-conditioned Hilbert system with
//                              double vs efloat, as an accuracy oracle (issue #1097)
//
// The Hilbert matrix H(i,j) = 1/(i+j-1) is a classic ill-conditioned matrix:
// its condition number grows like ~e^(3.5 n), so for n = 12 it is ~1e16, right
// at the edge of double precision. Given the exact solution x = [1,1,...,1] we
// form b = Hx and solve Hx_hat = b by Gaussian elimination with partial
// pivoting. The relative error ||x - x_hat|| / ||x|| exposes the conditioning:
// double loses all its digits, while efloat -- running the SAME templated
// solver at 512 bits -- recovers x almost exactly and serves as the oracle.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <universal/number/efloat/efloat.hpp>

// Relative L2 error ||x_hat - x|| / ||x|| of solving the n x n Hilbert system
// H x = b (with x = all-ones) by Gaussian elimination with partial pivoting.
// The same code runs on `double` and on `efloat`.
template<typename Real>
Real hilbert_solve_relerr(int n) {
	using std::abs;
	using std::sqrt;
	using Matrix = std::vector<std::vector<Real>>;
	using Vector = std::vector<Real>;

	// Hilbert matrix H(i,j) = 1/(i+j+1) (0-based) and exact solution x = ones.
	Matrix H(n, Vector(n));
	Vector x(n, Real(1.0));
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			H[i][j] = Real(1.0) / Real(static_cast<double>(i + j + 1));

	// Right-hand side b = H x.
	Vector b(n, Real(0.0));
	for (int i = 0; i < n; ++i) {
		Real s(0.0);
		for (int j = 0; j < n; ++j) s = s + H[i][j] * x[j];
		b[i] = s;
	}

	// Gaussian elimination with partial pivoting on a working copy.
	Matrix A = H;
	Vector c = b;
	for (int k = 0; k < n; ++k) {
		int    piv  = k;
		Real   best = abs(A[k][k]);
		for (int i = k + 1; i < n; ++i) {
			Real v = abs(A[i][k]);
			if (v > best) { best = v; piv = i; }
		}
		std::swap(A[k], A[piv]);
		std::swap(c[k], c[piv]);
		for (int i = k + 1; i < n; ++i) {
			Real factor = A[i][k] / A[k][k];
			for (int j = k; j < n; ++j) A[i][j] = A[i][j] - factor * A[k][j];
			c[i] = c[i] - factor * c[k];
		}
	}

	// Back substitution.
	Vector xhat(n, Real(0.0));
	for (int i = n - 1; i >= 0; --i) {
		Real s = c[i];
		for (int j = i + 1; j < n; ++j) s = s - A[i][j] * xhat[j];
		xhat[i] = s / A[i][i];
	}

	// Relative L2 error.
	Real num(0.0), den(0.0);
	for (int i = 0; i < n; ++i) {
		Real e = xhat[i] - x[i];
		num = num + e * e;
		den = den + x[i] * x[i];
	}
	return sqrt(num) / sqrt(den);
}

int main()
try {
	using namespace sw::universal;
	using efloat512 = efloat<16>;   // 16 limbs * 32 = 512 bits (~154 digits)

	std::cout << "Ill-conditioned linear system: Hilbert matrix H(i,j) = 1/(i+j-1)\n";
	std::cout << "Exact solution x = [1, 1, ..., 1];  b = Hx;  solve Hx_hat = b (Gaussian elim).\n";
	std::cout << "Relative error ||x_hat - x|| / ||x||.  efloat working precision: 512 bits.\n\n";

	// -------------------------------------------------------------------------
	// Primary result at n = 12 (condition number ~1e16, at double's limit).
	// -------------------------------------------------------------------------
	{
		const int n = 12;
		double    de = double(hilbert_solve_relerr<double>(n));
		efloat512 ee = hilbert_solve_relerr<efloat512>(n);
		std::cout << std::scientific << std::setprecision(3);
		std::cout << "At n = " << n << ":\n";
		std::cout << "  double  relative error = " << de       << "   <- solution is essentially garbage\n";
		std::cout << "  efloat  relative error = " << double(ee) << "   <- oracle: x recovered almost exactly\n\n";
	}

	// -------------------------------------------------------------------------
	// Breakdown across n: double degrades toward O(1) error as conditioning
	// outruns 53-bit precision; efloat stays near machine-zero for 512 bits.
	// -------------------------------------------------------------------------
	std::cout << "  " << std::left
	          << std::setw(6)  << "n"
	          << std::setw(22) << "double rel.error"
	          << "efloat<512b> rel.error\n";
	std::cout << "  " << std::string(50, '-') << "\n";
	std::cout << std::right << std::scientific << std::setprecision(3);

	for (int n = 4; n <= 14; n += 2) {
		double    de = double(hilbert_solve_relerr<double>(n));
		efloat512 ee = hilbert_solve_relerr<efloat512>(n);
		std::cout << "  " << std::left << std::setw(6) << n << std::right
		          << std::setw(18) << de << "    "
		          << std::setw(18) << double(ee) << "\n";
	}

	std::cout << "\nThe same Gaussian-elimination kernel runs on both types. double cannot survive\n";
	std::cout << "the Hilbert conditioning past n ~ 12; efloat at 512 bits does, so it doubles as\n";
	std::cout << "a high-precision oracle for verifying the double solver's accuracy.\n";

	return EXIT_SUCCESS;
}
catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
