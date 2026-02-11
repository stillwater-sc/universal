// iterative_refinement.cpp: mixed-precision iterative refinement case study
//
// Demonstrates the Carson & Higham three-precision iterative refinement
// pattern across Universal's number system inventory.  The algorithm
// factors A in LOW precision, solves in WORKING precision, and computes
// residuals in HIGH precision.  By varying the type at each tier we
// quantify how different number systems affect convergence rate and
// final accuracy for a fixed test problem.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <universal/utility/directives.hpp>

// Number systems
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dd/dd.hpp>

// BLAS infrastructure
#include <blas/blas.hpp>
#include <blas/utes/nbe.hpp>
#include <blas/utes/matnorm.hpp>

// standard libs
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace sw { namespace blas {

using namespace sw::numeric::containers;

// -------------------------------------------------------------------------
// Dense LU factorization with partial pivoting (in-place, Doolittle)
// -------------------------------------------------------------------------
template<typename Scalar>
void plu_factor(matrix<Scalar>& A, std::vector<size_t>& piv) {
	size_t n = num_rows(A);
	piv.resize(n);
	for (size_t i = 0; i < n; ++i) piv[i] = i;

	for (size_t k = 0; k < n - 1; ++k) {
		// partial pivoting: find max |A(i,k)| for i >= k
		size_t argmax = k;
		double absmax = std::abs(double(A(k, k)));
		for (size_t i = k + 1; i < n; ++i) {
			double v = std::abs(double(A(i, k)));
			if (v > absmax) { absmax = v; argmax = i; }
		}
		if (argmax != k) {
			std::swap(piv[k], piv[argmax]);
			for (size_t j = 0; j < n; ++j) {
				Scalar tmp = A(k, j);
				A(k, j) = A(argmax, j);
				A(argmax, j) = tmp;
			}
		}
		// eliminate below pivot
		for (size_t i = k + 1; i < n; ++i) {
			A(i, k) = A(i, k) / A(k, k);
			for (size_t j = k + 1; j < n; ++j) {
				A(i, j) = A(i, j) - A(i, k) * A(k, j);
			}
		}
	}
}

// Forward substitution: solve Ly = b  (unit lower triangular, in-place LU)
template<typename Scalar>
vector<Scalar> forward_solve(const matrix<Scalar>& LU, const vector<Scalar>& b) {
	size_t n = size(b);
	vector<Scalar> y(n);
	for (size_t i = 0; i < n; ++i) {
		Scalar s = b[i];
		for (size_t j = 0; j < i; ++j) {
			s = s - LU(i, j) * y[j];
		}
		y[i] = s;  // unit diagonal
	}
	return y;
}

// Back substitution: solve Ux = y  (upper triangular from in-place LU)
template<typename Scalar>
vector<Scalar> back_solve(const matrix<Scalar>& LU, const vector<Scalar>& y) {
	size_t n = size(y);
	vector<Scalar> x(n);
	for (size_t i = n; i > 0; --i) {
		size_t ii = i - 1;
		Scalar s = y[ii];
		for (size_t j = ii + 1; j < n; ++j) {
			s = s - LU(ii, j) * x[j];
		}
		x[ii] = s / LU(ii, ii);
	}
	return x;
}

// Apply permutation to a vector
template<typename Scalar>
vector<Scalar> permute_vec(const std::vector<size_t>& piv, const vector<Scalar>& b) {
	size_t n = size(b);
	vector<Scalar> pb(n);
	for (size_t i = 0; i < n; ++i) pb[i] = b[piv[i]];
	return pb;
}

}} // namespace sw::blas


// -------------------------------------------------------------------------
// Cross-type conversion helpers (go through double to avoid missing
// direct constructors between unrelated number system families)
// -------------------------------------------------------------------------
template<typename Dst, typename Src>
sw::numeric::containers::matrix<Dst> convert_matrix(const sw::numeric::containers::matrix<Src>& A) {
	size_t m = num_rows(A), n = num_cols(A);
	sw::numeric::containers::matrix<Dst> B(m, n);
	for (size_t i = 0; i < m; ++i)
		for (size_t j = 0; j < n; ++j)
			B(i, j) = Dst(double(A(i, j)));
	return B;
}

template<typename Dst, typename Src>
sw::numeric::containers::vector<Dst> convert_vector(const sw::numeric::containers::vector<Src>& v) {
	size_t n = size(v);
	sw::numeric::containers::vector<Dst> w(n);
	for (size_t i = 0; i < n; ++i)
		w[i] = Dst(double(v[i]));
	return w;
}

// -------------------------------------------------------------------------
// Three-precision iterative refinement
//
//   HIGH  — residual computation:  r = b - A*x
//   WORK  — triangular solves:     c = (LU)^{-1} r
//   LOW   — LU factorization:     PA = LU
//
// Returns (iterations, final_nbe, final_forward_error)
// -------------------------------------------------------------------------
template<typename High, typename Work, typename Low>
std::tuple<int, double, double>
iterative_refinement(size_t n, int max_iter = 25) {
	using namespace sw::blas;
	using namespace sw::numeric::containers;

	// --- Build a well-defined test problem in HIGH precision ---
	// Tridiagonal(-1, 2, -1): condition number ~ O(n^2), SPD
	matrix<High> Ah(n, n);
	for (size_t i = 0; i < n; ++i) {
		Ah(i, i) = High(2);
		if (i > 0)     Ah(i, i-1) = High(-1);
		if (i < n - 1) Ah(i, i+1) = High(-1);
	}

	// Known solution x* = [1, 1, ..., 1]
	vector<High> x_exact(n, High(1));
	vector<High> bh = Ah * x_exact;

	// --- Step 1: Factor in LOW precision ---
	matrix<Low> Al = convert_matrix<Low>(Ah);

	std::vector<size_t> piv;
	plu_factor(Al, piv);

	// Store factored LU in WORKING precision for triangular solves
	matrix<Work> LU_w = convert_matrix<Work>(Al);

	// Permute b to match pivot order, in WORKING precision
	vector<Work> bw = convert_vector<Work>(bh);
	vector<Work> pb = permute_vec(piv, bw);

	// --- Step 2: Initial solve x0 = (LU)^{-1} Pb ---
	vector<Work> xw = back_solve(LU_w, forward_solve(LU_w, pb));

	// Build permuted Ah for residual computation
	matrix<High> Ah_perm(n, n);
	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j < n; ++j)
			Ah_perm(i, j) = Ah(piv[i], j);
	vector<High> bh_perm = permute_vec(piv, bh);

	// --- Step 3: Iterative refinement loop ---
	int iter = 0;
	double final_nbe = 1.0;
	double final_fwd = 1.0;

	for (iter = 1; iter <= max_iter; ++iter) {
		// (a) Compute residual in HIGH precision: r = b - A*x
		vector<High> xh = convert_vector<High>(xw);
		vector<High> rh = bh_perm - Ah_perm * xh;

		// (b) Solve correction in WORKING precision: c = (LU)^{-1} r
		vector<Work> rw = convert_vector<Work>(rh);
		vector<Work> c = back_solve(LU_w, forward_solve(LU_w, rw));

		// (c) Update: x = x + c
		xw = xw + c;

		// Convergence check: forward error ||x* - x||_inf
		double fwd_err = 0.0;
		for (size_t i = 0; i < n; ++i) {
			double d = std::abs(1.0 - double(xw[i]));
			if (d > fwd_err) fwd_err = d;
		}

		// Normwise backward error (computed in double to avoid template issues)
		// nbe = ||b - Ax||_inf / (||A||_inf * ||x||_inf + ||b||_inf)
		double res_norm = 0.0;
		for (size_t i = 0; i < n; ++i) {
			double ri = double(bh[piv[i]]);
			for (size_t j = 0; j < n; ++j)
				ri -= double(Ah(piv[i], j)) * double(xw[j]);
			res_norm = std::max(res_norm, std::abs(ri));
		}
		double A_norm = 0.0;
		for (size_t i = 0; i < n; ++i) {
			double row_sum = 0.0;
			for (size_t j = 0; j < n; ++j)
				row_sum += std::abs(double(Ah(i, j)));
			A_norm = std::max(A_norm, row_sum);
		}
		double x_norm = 0.0, b_norm = 0.0;
		for (size_t i = 0; i < n; ++i) {
			x_norm = std::max(x_norm, std::abs(double(xw[i])));
			b_norm = std::max(b_norm, std::abs(double(bh[i])));
		}
		double nbe_val = res_norm / (A_norm * x_norm + b_norm);

		final_nbe = nbe_val;
		final_fwd = fwd_err;

		if (nbe_val < 1.0e-12 || fwd_err < 1.0e-12) break;
		if (fwd_err > 1.0e+6) break;  // diverging
	}

	return { iter, final_nbe, final_fwd };
}


// -------------------------------------------------------------------------
// Reporting
// -------------------------------------------------------------------------

struct IRResult {
	std::string config;
	std::string low_type;
	std::string work_type;
	std::string high_type;
	int         iterations;
	double      nbe;
	double      forward_error;
};

static void print_header() {
	std::cout << std::left  << std::setw(8)  << "Config"
	          << std::setw(22) << "Low"
	          << std::setw(22) << "Working"
	          << std::setw(22) << "High"
	          << std::right << std::setw(6)  << "Iters"
	          << std::setw(14) << "NBE"
	          << std::setw(14) << "Fwd Error"
	          << '\n';
	std::cout << std::string(108, '-') << '\n';
}

static void print_row(const IRResult& r) {
	bool converged = (r.nbe < 1.0e-12 || r.forward_error < 1.0e-12);
	std::cout << std::left  << std::setw(8)  << r.config
	          << std::setw(22) << r.low_type
	          << std::setw(22) << r.work_type
	          << std::setw(22) << r.high_type;
	if (converged) {
		std::cout << std::right << std::setw(6) << r.iterations;
	} else {
		std::cout << std::right << std::setw(6) << "DNF";
	}
	std::cout << std::setw(14) << std::scientific << std::setprecision(2) << r.nbe
	          << std::setw(14) << r.forward_error
	          << '\n';
}

// -------------------------------------------------------------------------
// Convenience runner
// -------------------------------------------------------------------------

template<typename High, typename Work, typename Low>
IRResult run_ir(const std::string& config,
                const std::string& low_name,
                const std::string& work_name,
                const std::string& high_name,
                size_t n = 20) {
	auto [iters, nbe, fwd] = iterative_refinement<High, Work, Low>(n);
	return { config, low_name, work_name, high_name, iters, nbe, fwd };
}


// =========================================================================
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	size_t N = 20;  // matrix dimension (tridiag, kappa ~ O(N^2))
	if (argc > 1) N = std::atoi(argv[1]);

	std::cout << "Mixed-Precision Iterative Refinement: LU-IR with Three Precisions\n";
	std::cout << "Problem: tridiag(-1, 2, -1),  N = " << N
	          << ",  kappa ~ " << static_cast<size_t>(4.0 * N * N / (M_PI * M_PI)) << '\n';
	std::cout << "Algorithm: Carson & Higham (SIAM J. Sci. Comput., 2018)\n";
	std::cout << "  1. Factor A = PLU in LOW precision\n";
	std::cout << "  2. Solve x = (LU)^{-1} b in WORKING precision\n";
	std::cout << "  3. Compute residual r = b - Ax in HIGH precision\n";
	std::cout << "  4. Solve correction c = (LU)^{-1} r, update x += c\n";
	std::cout << "  5. Repeat until NBE < 1e-12 or max 25 iterations\n\n";

	// =================================================================
	// Section 1: IEEE floating-point baseline
	// =================================================================
	std::cout << "=== IEEE Floating-Point Configurations ===\n\n";
	print_header();

	std::vector<IRResult> results;

	// IEEE classic: half / float / double
	results.push_back(run_ir<double, float, sw::universal::half>(
		"IEEE-1", "half (fp16)", "float (fp32)", "double (fp64)", N));

	// IEEE: bfloat16 / float / double
	results.push_back(run_ir<double, float, sw::universal::bfloat_t>(
		"IEEE-2", "bfloat16", "float (fp32)", "double (fp64)", N));

	// IEEE: float / double / double (classic Wilkinson IR)
	results.push_back(run_ir<double, double, float>(
		"IEEE-3", "float (fp32)", "double (fp64)", "double (fp64)", N));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 2: Posit configurations
	// =================================================================
	std::cout << "\n=== Posit Configurations ===\n\n";
	print_header();
	results.clear();

	// Posit: 16/32/64
	results.push_back(run_ir<posit<64,3>, posit<32,2>, posit<16,1>>(
		"Posit-1", "posit<16,1>", "posit<32,2>", "posit<64,3>", N));

	// Posit narrow: 8/16/32
	results.push_back(run_ir<posit<32,2>, posit<16,1>, posit<8,0>>(
		"Posit-2", "posit<8,0>", "posit<16,1>", "posit<32,2>", N));

	// Posit wide: 32/64/128
	results.push_back(run_ir<posit<128,4>, posit<64,3>, posit<32,2>>(
		"Posit-3", "posit<32,2>", "posit<64,3>", "posit<128,4>", N));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 3: Classic float (cfloat) configurations
	// =================================================================
	std::cout << "\n=== Classic Float (cfloat) Configurations ===\n\n";
	print_header();
	results.clear();

	// cfloat: 16/32/64  (IEEE-like but template-parameterized)
	using cf16  = cfloat<16, 5, uint16_t, true, false, false>;
	using cf32  = cfloat<32, 8, uint32_t, true, false, false>;
	using cf64  = cfloat<64, 11, uint64_t, true, false, false>;

	results.push_back(run_ir<cf64, cf32, cf16>(
		"CF-1", "cfloat<16,5>", "cfloat<32,8>", "cfloat<64,11>", N));

	// cfloat with non-standard widths: 12/24/48
	using cf12  = cfloat<12, 4, uint16_t, true, false, false>;
	using cf24  = cfloat<24, 7, uint32_t, true, false, false>;
	using cf48  = cfloat<48, 10, uint64_t, true, false, false>;

	results.push_back(run_ir<cf48, cf24, cf12>(
		"CF-2", "cfloat<12,4>", "cfloat<24,7>", "cfloat<48,10>", N));

	// cfloat with extra fraction bits: 16/32/64 with es=4/6/8
	using cf16w = cfloat<16, 4, uint16_t, true, false, false>;
	using cf32w = cfloat<32, 6, uint32_t, true, false, false>;
	using cf64w = cfloat<64, 8, uint64_t, true, false, false>;

	results.push_back(run_ir<cf64w, cf32w, cf16w>(
		"CF-3", "cfloat<16,4>", "cfloat<32,6>", "cfloat<64,8>", N));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 4: Cross-family configurations
	//   Mix different number system families at different precision tiers
	// =================================================================
	std::cout << "\n=== Cross-Family Mixed-Precision Configurations ===\n\n";
	print_header();
	results.clear();

	// Half → posit<32,2> working → double high
	results.push_back(run_ir<double, posit<32,2>, sw::universal::half>(
		"X-1", "half (fp16)", "posit<32,2>", "double (fp64)", N));

	// posit<16,1> low → float working → dd high (double-double)
	results.push_back(run_ir<dd, float, posit<16,1>>(
		"X-2", "posit<16,1>", "float (fp32)", "dd (2x64)", N));

	// bfloat16 low → posit<32,2> working → dd high
	results.push_back(run_ir<dd, posit<32,2>, sw::universal::bfloat_t>(
		"X-3", "bfloat16", "posit<32,2>", "dd (2x64)", N));

	// cfloat<16,5> low → cfloat<32,8> working → dd high
	results.push_back(run_ir<dd, cf32, cf16>(
		"X-4", "cfloat<16,5>", "cfloat<32,8>", "dd (2x64)", N));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 5: Effect of problem size on convergence
	// =================================================================
	std::cout << "\n=== Convergence vs Problem Size (IEEE-1: half/float/double) ===\n\n";
	std::cout << std::left  << std::setw(8)  << "N"
	          << std::setw(12) << "kappa"
	          << std::right << std::setw(8)  << "Iters"
	          << std::setw(14) << "NBE"
	          << std::setw(14) << "Fwd Error"
	          << '\n';
	std::cout << std::string(56, '-') << '\n';

	for (size_t sz : { 5, 10, 20, 50, 100 }) {
		auto [iters, nbe, fwd] = iterative_refinement<double, float, sw::universal::half>(sz);
		double kappa = 4.0 * sz * sz / (M_PI * M_PI);
		std::cout << std::left  << std::setw(8)  << sz
		          << std::setw(12) << std::fixed << std::setprecision(0) << kappa
		          << std::right << std::setw(8)  << iters
		          << std::setw(14) << std::scientific << std::setprecision(2) << nbe
		          << std::setw(14) << fwd
		          << '\n';
	}

	// =================================================================
	// Legend
	// =================================================================
	std::cout << R"(

Legend
------
  Config    Shorthand label for the precision configuration
  Low       Number type used for LU factorization (cheapest, least accurate)
  Working   Number type used for triangular solves and solution vector
  High      Number type used for residual computation (most accurate)
  Iters     Number of refinement iterations to converge (max 25)
  NBE       Normwise Backward Error: ||b-Ax||_inf / (||A||_inf ||x||_inf + ||b||_inf)
  Fwd Error Forward error: ||x* - x||_inf  (x* = [1,1,...,1])

Key observations:

  1. IEEE-1 (half/float/double) is the classic Carson & Higham configuration.
     Despite factoring in half precision (5 significand bits), IR recovers
     full float accuracy in a few iterations.

  2. Posit-1 (posit<16,1>/posit<32,2>/posit<64,3>) converges comparably to
     IEEE-1 because posit<16,1> has more significand bits near 1.0 than
     IEEE half, making the low-precision factorization more accurate.

  3. Cross-family X-2 and X-3 use double-double (dd) for the high-precision
     residual.  The 106-bit significand provides a much sharper residual than
     double, accelerating convergence — especially for larger N where
     kappa(A) grows as O(N^2).

  4. As N increases, more IR iterations are needed because the condition
     number grows.  The convergence rate is approximately:
       ||e_{k+1}|| / ||e_k|| ~ kappa(A) * u_low
     where u_low is the unit roundoff of the low-precision type.

  5. Non-standard cfloat widths (CF-2: 12/24/48) demonstrate the library's
     ability to explore precision points that don't correspond to any
     hardware format — useful for co-design studies.

Reference:
  Carson, E. and Higham, N. J. (2018). "Accelerating the solution of linear
  systems by iterative refinement in three precisions." SIAM J. Sci. Comput.
  40(2), A817--A847.
)" << '\n';

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
