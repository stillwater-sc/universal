// takum_iterative_refinement.cpp: mixed-precision iterative refinement comparing
// takum, posit, and cfloat as the low-precision working type.
//
// Iterative refinement (Wilkinson, 1963) solves Ax = b by:
//   1. Factor A and solve Ax0 = b in low precision
//   2. Compute residual r = b - A*x in high precision (double)
//   3. Solve A*d = r in low precision
//   4. Update x = x + d
//   5. Repeat until ||r|| / ||b|| < tolerance or max iterations
//
// We test two problems that highlight different format strengths:
//   Problem 1: Hilbert matrix -- entries in [0,1], posit's sweet spot
//   Problem 2: Dense wide-range matrix -- entries span 10^-20 to 10^20,
//              where takum's bounded dynamic range provides an advantage
//
// Reference: Hunhold et al., arXiv:2412.20268 (sparse linear solvers)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/takum/takum.hpp>

namespace {

using dvec = std::vector<double>;
using dmat = std::vector<dvec>;

dmat hilbert(int n) {
	dmat H(n, dvec(n));
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			H[i][j] = 1.0 / (i + j + 1);
	return H;
}

// Generate a dense matrix with wide dynamic range: A[i][j] = di*dj/n + delta(i,j)
// where di = 10^(scales[i]).  The identity term makes it diagonally dominant.
dmat wide_range_matrix(const dvec& scales) {
	int n = static_cast<int>(scales.size());
	dmat A(n, dvec(n, 0.0));
	for (int i = 0; i < n; ++i) {
		double di = std::pow(10.0, scales[i]);
		for (int j = 0; j < n; ++j) {
			double dj = std::pow(10.0, scales[j]);
			// A[i][j] = di * dj / n + (i == j ? 1 : 0)
			A[i][j] = di * dj / n;
		}
		A[i][i] += 1.0; // make it diagonally dominant for stability
	}
	return A;
}

dvec matvec(const dmat& A, const dvec& x) {
	int n = static_cast<int>(A.size());
	dvec y(n, 0.0);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			y[i] += A[i][j] * x[j];
	return y;
}

dvec vecsub(const dvec& a, const dvec& b) {
	dvec r(a.size());
	for (size_t i = 0; i < a.size(); ++i) r[i] = a[i] - b[i];
	return r;
}

dvec vecadd(const dvec& a, const dvec& b) {
	dvec c(a.size());
	for (size_t i = 0; i < a.size(); ++i) c[i] = a[i] + b[i];
	return c;
}

double norm2(const dvec& v) {
	double s = 0.0;
	for (auto x : v) s += x * x;
	return std::sqrt(s);
}

// LU solve in low precision T with partial pivoting
template<typename T>
dvec lu_solve_lowprec(const dmat& A, const dvec& b) {
	int n = static_cast<int>(A.size());
	std::vector<std::vector<T>> LU(n, std::vector<T>(n));
	std::vector<T> bp(n);
	for (int i = 0; i < n; ++i) {
		bp[i] = T(b[i]);
		for (int j = 0; j < n; ++j)
			LU[i][j] = T(A[i][j]);
	}

	std::vector<int> piv(n);
	for (int i = 0; i < n; ++i) piv[i] = i;

	for (int k = 0; k < n; ++k) {
		double maxval = 0;
		int maxrow = k;
		for (int i = k; i < n; ++i) {
			double v = std::abs(double(LU[i][k]));
			if (v > maxval) { maxval = v; maxrow = i; }
		}
		if (maxrow != k) {
			std::swap(LU[k], LU[maxrow]);
			std::swap(piv[k], piv[maxrow]);
		}
		T pivot = LU[k][k];
		if (double(pivot) == 0.0) {
			// Singular matrix: return a zero solution (refinement will detect via residual)
			dvec zero_result(n, 0.0);
			return zero_result;
		}

		for (int i = k + 1; i < n; ++i) {
			T factor = LU[i][k] / pivot;
			LU[i][k] = factor;
			for (int j = k + 1; j < n; ++j)
				LU[i][j] = LU[i][j] - factor * LU[k][j];
		}
	}

	// Forward substitution
	std::vector<T> y(n);
	for (int i = 0; i < n; ++i) {
		T sum = bp[piv[i]];
		for (int j = 0; j < i; ++j) sum = sum - LU[i][j] * y[j];
		y[i] = sum;
	}

	// Back substitution
	std::vector<T> x(n);
	for (int i = n - 1; i >= 0; --i) {
		T sum = y[i];
		for (int j = i + 1; j < n; ++j) sum = sum - LU[i][j] * x[j];
		x[i] = sum / LU[i][i];
	}

	dvec result(n);
	for (int i = 0; i < n; ++i) result[i] = double(x[i]);
	return result;
}

struct RefinementResult {
	int iterations;
	double residual;   // final relative residual
	bool converged;
};

template<typename T>
RefinementResult iterative_refinement(
	const dmat& A, const dvec& b,
	double tol = 1e-10, int maxiter = 30)
{
	double bnorm = norm2(b);
	if (bnorm == 0.0) bnorm = 1.0;  // guard against zero RHS
	dvec x = lu_solve_lowprec<T>(A, b);

	RefinementResult result{0, 0.0, false};

	for (int iter = 0; iter < maxiter; ++iter) {
		dvec Ax = matvec(A, x);
		dvec r = vecsub(b, Ax);
		result.residual = norm2(r) / bnorm;
		result.iterations = iter;

		if (result.residual < tol) {
			result.converged = true;
			return result;
		}

		dvec d = lu_solve_lowprec<T>(A, r);
		x = vecadd(x, d);
	}

	dvec Ax = matvec(A, x);
	dvec r = vecsub(b, Ax);
	result.residual = norm2(r) / bnorm;
	result.iterations = maxiter;
	return result;
}

void report_header() {
	constexpr int W = 14;
	std::cout << std::setw(W) << "type"
	          << std::setw(W) << "iterations"
	          << std::setw(W) << "residual"
	          << std::setw(W) << "converged" << '\n';
	std::cout << std::string(4 * W, '-') << '\n';
}

void report_line(const std::string& name, const RefinementResult& res) {
	constexpr int W = 14;
	std::cout << std::setw(W) << name
	          << std::setw(W) << res.iterations
	          << std::setw(W) << std::scientific << std::setprecision(2) << res.residual
	          << std::setw(W) << (res.converged ? "yes" : "NO") << '\n';
}

} // anonymous namespace

int main()
try {
	using namespace sw::universal;

	using Takum16  = takum<16>;
	using Posit16  = posit<16, 2>;
	using Cfloat16 = cfloat<16, 5, std::uint16_t, true, false, false>;

	double tol = 1e-10;
	int maxiter = 30;

	std::cout << "Mixed-Precision Iterative Refinement: Takum vs Posit vs IEEE\n";
	std::cout << "=============================================================\n";
	std::cout << "LU factorization in low precision, residual in double.\n\n";

	// ---------------------------------------------------------------
	// Problem 1: Hilbert matrix (entries near 1.0 -- posit's strength)
	// ---------------------------------------------------------------
	std::cout << "Problem 1: Hilbert matrix (entries in [0.06, 1.0])\n";
	std::cout << "  Posit concentrates precision near 1.0 -- expect posit advantage.\n\n";

	for (int n : {4, 5, 6}) {
		dmat H = hilbert(n);
		dvec x_exact(n, 1.0);
		dvec b = matvec(H, x_exact);

		std::cout << "  Hilbert " << n << "x" << n << ":\n  ";
		report_header();
		std::cout << "  "; report_line("takum<16>",   iterative_refinement<Takum16>(H, b, tol, maxiter));
		std::cout << "  "; report_line("posit<16,2>", iterative_refinement<Posit16>(H, b, tol, maxiter));
		std::cout << "  "; report_line("cfloat<16>",  iterative_refinement<Cfloat16>(H, b, tol, maxiter));
		std::cout << "  "; report_line("double",      iterative_refinement<double>(H, b, tol, maxiter));
		std::cout << '\n';
	}

	// ---------------------------------------------------------------
	// Problem 2: Wide dynamic range system (takum's strength)
	// ---------------------------------------------------------------
	std::cout << "Problem 2: Wide-range matrix (entries span 10^-20 to 10^20)\n";
	std::cout << "  At 10^+/-20, takum has ~2.4 digits but posit has 0 digits.\n\n";

	for (int n : {4, 6, 8}) {
		// Create scales that push past posit<16,2>'s precision cliff (~10^18)
		double max_scale = 20.0;
		dvec scales(n);
		for (int i = 0; i < n; ++i)
			scales[i] = -max_scale + 2.0 * max_scale * i / (n - 1);

		dmat A = wide_range_matrix(scales);
		dvec x_exact(n, 1.0);
		dvec b = matvec(A, x_exact);

		std::cout << "  Wide-range " << n << "x" << n
		          << " (scales: " << std::fixed << std::setprecision(0)
		          << scales.front() << " to " << scales.back() << "):\n  ";
		report_header();
		std::cout << "  "; report_line("takum<16>",   iterative_refinement<Takum16>(A, b, tol, maxiter));
		std::cout << "  "; report_line("posit<16,2>", iterative_refinement<Posit16>(A, b, tol, maxiter));
		std::cout << "  "; report_line("cfloat<16>",  iterative_refinement<Cfloat16>(A, b, tol, maxiter));
		std::cout << "  "; report_line("double",      iterative_refinement<double>(A, b, tol, maxiter));
		std::cout << '\n';
	}

	std::cout << "Takeaway:\n";
	std::cout << "  - Near 1.0 (Hilbert): posit's concentrated precision wins\n";
	std::cout << "  - Wide range: takum's bounded regime preserves precision where\n";
	std::cout << "    posit and cfloat overflow, underflow, or lose all digits\n";
	std::cout << "  - The choice depends on your problem's value distribution\n";

	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
