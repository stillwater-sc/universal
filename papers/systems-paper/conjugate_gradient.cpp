// conjugate_gradient.cpp: mixed-precision Conjugate Gradient case study
//
// Demonstrates how the choice of number system affects convergence
// of the preconditioned Conjugate Gradient method on SPD systems.
// CG is the canonical Krylov solver for symmetric positive-definite
// problems and is sensitive to rounding — small representational
// errors can break A-orthogonality of the search directions,
// causing stagnation or slow convergence.
//
// We test across Universal's number system inventory:
//   - IEEE:  half, bfloat16, float, double
//   - Posit: 8/16/32/64-bit configurations
//   - cfloat: standard and non-standard widths
//   - dd:    double-double (106-bit significand)
//   - Cross-family: mix different families in preconditioner vs solver
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <universal/utility/directives.hpp>

// Number systems
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dd/dd.hpp>

// BLAS infrastructure
#include <blas/blas.hpp>
#include <blas/generators.hpp>

// standard libs
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// -------------------------------------------------------------------------
// Cross-type conversion helpers
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
// Preconditioned Conjugate Gradient
//
// Solves Ax = b for SPD matrix A with Jacobi preconditioner M^{-1} = diag(A)^{-1}
//
// Template parameters allow the preconditioner and solver to use
// different scalar types, enabling mixed-precision exploration.
//
//   Scalar — working precision for the CG iteration
//
// Returns (iterations, final_residual_norm, forward_error)
// -------------------------------------------------------------------------
template<typename Scalar>
std::tuple<int, double, double>
run_cg(const sw::numeric::containers::matrix<Scalar>& A,
       const sw::numeric::containers::vector<Scalar>& b,
       const sw::numeric::containers::vector<Scalar>& x_exact,
       int max_iter, double tol) {
	using namespace sw::numeric::containers;
	size_t n = size(b);

	// Jacobi preconditioner: M^{-1} = diag(A)^{-1}
	vector<Scalar> Minv(n);
	for (size_t i = 0; i < n; ++i)
		Minv[i] = Scalar(1) / A(i, i);

	// CG iteration
	vector<Scalar> x(n);     // x0 = 0
	vector<Scalar> r = b;    // r0 = b - A*x0 = b
	vector<Scalar> z(n);
	for (size_t i = 0; i < n; ++i) z[i] = Minv[i] * r[i]; // z0 = M^{-1} r0
	vector<Scalar> p = z;    // p0 = z0

	double rz = 0.0;
	for (size_t i = 0; i < n; ++i) rz += double(r[i]) * double(z[i]);

	int iter = 0;
	double res_norm = 0.0;
	double fwd_err = 0.0;

	for (iter = 1; iter <= max_iter; ++iter) {
		// q = A * p
		vector<Scalar> q = A * p;

		// alpha = (r,z) / (p,q)
		double pq = 0.0;
		for (size_t i = 0; i < n; ++i) pq += double(p[i]) * double(q[i]);
		if (pq == 0.0) break;
		double alpha = rz / pq;

		// x = x + alpha * p
		for (size_t i = 0; i < n; ++i)
			x[i] = Scalar(double(x[i]) + alpha * double(p[i]));

		// r = r - alpha * q
		for (size_t i = 0; i < n; ++i)
			r[i] = Scalar(double(r[i]) - alpha * double(q[i]));

		// z = M^{-1} r
		for (size_t i = 0; i < n; ++i) z[i] = Minv[i] * r[i];

		// beta = (r_new, z_new) / (r_old, z_old)
		double rz_new = 0.0;
		for (size_t i = 0; i < n; ++i) rz_new += double(r[i]) * double(z[i]);
		double beta_val = rz_new / rz;
		rz = rz_new;

		// p = z + beta * p
		for (size_t i = 0; i < n; ++i)
			p[i] = Scalar(double(z[i]) + beta_val * double(p[i]));

		// Convergence check
		res_norm = 0.0;
		for (size_t i = 0; i < n; ++i)
			res_norm += double(r[i]) * double(r[i]);
		res_norm = std::sqrt(res_norm);

		fwd_err = 0.0;
		for (size_t i = 0; i < n; ++i) {
			double d = std::abs(double(x_exact[i]) - double(x[i]));
			if (d > fwd_err) fwd_err = d;
		}

		if (res_norm < tol) break;
	}

	return { iter, res_norm, fwd_err };
}

// -------------------------------------------------------------------------
// Two-precision CG: preconditioner in LOW, iteration in WORK
//
// The matrix is formed in WORK precision.  The Jacobi preconditioner
// is computed in LOW precision (simulating a cheap approximate inverse).
// This shows how a low-precision preconditioner affects convergence.
// -------------------------------------------------------------------------
template<typename Work, typename Low>
std::tuple<int, double, double>
run_cg_two_precision(size_t n, int max_iter, double tol) {
	using namespace sw::numeric::containers;

	// Build tridiag(-1, 2, -1) in WORK precision
	matrix<Work> A(n, n);
	for (size_t i = 0; i < n; ++i) {
		A(i, i) = Work(2);
		if (i > 0)     A(i, i-1) = Work(-1);
		if (i < n - 1) A(i, i+1) = Work(-1);
	}
	vector<Work> x_exact(n, Work(1));
	vector<Work> b = A * x_exact;

	// Jacobi preconditioner in LOW precision, then promote to WORK
	// This simulates computing M^{-1} cheaply in reduced precision
	vector<Work> Minv(n);
	for (size_t i = 0; i < n; ++i)
		Minv[i] = Work(double(Low(1) / Low(A(i, i))));

	// CG iteration in WORK precision
	vector<Work> x(n);
	vector<Work> r = b;
	vector<Work> z(n);
	for (size_t i = 0; i < n; ++i) z[i] = Minv[i] * r[i];
	vector<Work> p = z;

	double rz = 0.0;
	for (size_t i = 0; i < n; ++i) rz += double(r[i]) * double(z[i]);

	int iter = 0;
	double res_norm = 0.0, fwd_err = 0.0;

	for (iter = 1; iter <= max_iter; ++iter) {
		vector<Work> q = A * p;
		double pq = 0.0;
		for (size_t i = 0; i < n; ++i) pq += double(p[i]) * double(q[i]);
		if (pq == 0.0) break;
		double alpha = rz / pq;

		for (size_t i = 0; i < n; ++i)
			x[i] = Work(double(x[i]) + alpha * double(p[i]));
		for (size_t i = 0; i < n; ++i)
			r[i] = Work(double(r[i]) - alpha * double(q[i]));
		for (size_t i = 0; i < n; ++i) z[i] = Minv[i] * r[i];

		double rz_new = 0.0;
		for (size_t i = 0; i < n; ++i) rz_new += double(r[i]) * double(z[i]);
		double beta_val = rz_new / rz;
		rz = rz_new;

		for (size_t i = 0; i < n; ++i)
			p[i] = Work(double(z[i]) + beta_val * double(p[i]));

		res_norm = 0.0;
		for (size_t i = 0; i < n; ++i)
			res_norm += double(r[i]) * double(r[i]);
		res_norm = std::sqrt(res_norm);

		fwd_err = 0.0;
		for (size_t i = 0; i < n; ++i) {
			double d = std::abs(1.0 - double(x[i]));
			if (d > fwd_err) fwd_err = d;
		}

		if (res_norm < tol) break;
	}

	return { iter, res_norm, fwd_err };
}

// -------------------------------------------------------------------------
// Reporting
// -------------------------------------------------------------------------

struct CGResult {
	std::string config;
	std::string scalar_type;
	int         iterations;
	double      residual;
	double      forward_error;
	bool        converged;
};

static void print_header() {
	std::cout << std::left  << std::setw(10) << "Config"
	          << std::setw(28) << "Scalar Type"
	          << std::right << std::setw(8)  << "Iters"
	          << std::setw(14) << "||r||_2"
	          << std::setw(14) << "Fwd Error"
	          << '\n';
	std::cout << std::string(74, '-') << '\n';
}

static void print_row(const CGResult& r) {
	std::cout << std::left  << std::setw(10) << r.config
	          << std::setw(28) << r.scalar_type;
	if (r.converged)
		std::cout << std::right << std::setw(8) << r.iterations;
	else
		std::cout << std::right << std::setw(8) << "DNF";
	std::cout << std::setw(14) << std::scientific << std::setprecision(2) << r.residual
	          << std::setw(14) << r.forward_error
	          << '\n';
}

static void print_2p_header() {
	std::cout << std::left  << std::setw(10) << "Config"
	          << std::setw(24) << "Precond (Low)"
	          << std::setw(24) << "Solver (Work)"
	          << std::right << std::setw(8)  << "Iters"
	          << std::setw(14) << "||r||_2"
	          << std::setw(14) << "Fwd Error"
	          << '\n';
	std::cout << std::string(94, '-') << '\n';
}

struct CG2PResult {
	std::string config;
	std::string low_type;
	std::string work_type;
	int         iterations;
	double      residual;
	double      forward_error;
	bool        converged;
};

static void print_2p_row(const CG2PResult& r) {
	std::cout << std::left  << std::setw(10) << r.config
	          << std::setw(24) << r.low_type
	          << std::setw(24) << r.work_type;
	if (r.converged)
		std::cout << std::right << std::setw(8) << r.iterations;
	else
		std::cout << std::right << std::setw(8) << "DNF";
	std::cout << std::setw(14) << std::scientific << std::setprecision(2) << r.residual
	          << std::setw(14) << r.forward_error
	          << '\n';
}

// -------------------------------------------------------------------------
// Single-precision convenience runner
// -------------------------------------------------------------------------
template<typename Scalar>
CGResult run_cg_experiment(const std::string& config,
                           const std::string& name,
                           size_t n, int max_iter, double tol) {
	using namespace sw::numeric::containers;

	matrix<Scalar> A(n, n);
	for (size_t i = 0; i < n; ++i) {
		A(i, i) = Scalar(2);
		if (i > 0)     A(i, i-1) = Scalar(-1);
		if (i < n - 1) A(i, i+1) = Scalar(-1);
	}
	vector<Scalar> x_exact(n, Scalar(1));
	vector<Scalar> b = A * x_exact;

	auto [iters, res, fwd] = run_cg<Scalar>(A, b, x_exact, max_iter, tol);
	bool conv = (res < tol);
	return { config, name, iters, res, fwd, conv };
}


// =========================================================================
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	size_t N = 32;
	if (argc > 1) N = std::atoi(argv[1]);

	constexpr int MAX_ITER = 500;
	constexpr double TOL = 1.0e-10;
	double kappa = 4.0 * N * N / (M_PI * M_PI);

	std::cout << "Mixed-Precision Conjugate Gradient: Number System Comparison\n";
	std::cout << "Problem: tridiag(-1, 2, -1),  N = " << N
	          << ",  kappa ~ " << static_cast<size_t>(kappa) << '\n';
	std::cout << "Preconditioner: Jacobi (M^{-1} = diag(A)^{-1})\n";
	std::cout << "Convergence: ||r||_2 < " << std::scientific
	          << std::setprecision(0) << TOL << ",  max " << MAX_ITER << " iterations\n\n";

	// =================================================================
	// Section 1: IEEE floating-point
	// =================================================================
	std::cout << "=== Section 1: IEEE Floating-Point ===\n\n";
	print_header();

	std::vector<CGResult> results;
	results.push_back(run_cg_experiment<half>(    "IEEE-1", "half (fp16)",      N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<bfloat_t>("IEEE-2", "bfloat16",        N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<float>(   "IEEE-3", "float (fp32)",    N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<double>(  "IEEE-4", "double (fp64)",   N, MAX_ITER, TOL));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 2: Posit configurations
	// =================================================================
	std::cout << "\n=== Section 2: Posit Configurations ===\n\n";
	print_header();
	results.clear();

	results.push_back(run_cg_experiment<posit<8,0>>(  "Posit-1", "posit<8,0>",   N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<posit<16,1>>( "Posit-2", "posit<16,1>",  N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<posit<32,2>>( "Posit-3", "posit<32,2>",  N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<posit<64,3>>( "Posit-4", "posit<64,3>",  N, MAX_ITER, TOL));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 3: Classic float (cfloat) configurations
	// =================================================================
	std::cout << "\n=== Section 3: cfloat Configurations ===\n\n";
	print_header();
	results.clear();

	using cf16  = cfloat<16, 5, uint16_t, true, false, false>;
	using cf32  = cfloat<32, 8, uint32_t, true, false, false>;
	using cf64  = cfloat<64, 11, uint64_t, true, false, false>;
	using cf12  = cfloat<12, 4, uint16_t, true, false, false>;
	using cf24  = cfloat<24, 7, uint32_t, true, false, false>;

	results.push_back(run_cg_experiment<cf12>( "CF-1", "cfloat<12,4>",  N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<cf16>( "CF-2", "cfloat<16,5>",  N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<cf24>( "CF-3", "cfloat<24,7>",  N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<cf32>( "CF-4", "cfloat<32,8>",  N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<cf64>( "CF-5", "cfloat<64,11>", N, MAX_ITER, TOL));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 4: Extended precision
	// =================================================================
	std::cout << "\n=== Section 4: Extended Precision ===\n\n";
	print_header();
	results.clear();

	results.push_back(run_cg_experiment<dd>(          "Ext-1", "dd (2x64)",      N, MAX_ITER, TOL));
	results.push_back(run_cg_experiment<posit<128,4>>("Ext-2", "posit<128,4>",   N, MAX_ITER, TOL));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Section 5: Two-precision CG (low-precision preconditioner)
	// =================================================================
	std::cout << "\n=== Section 5: Two-Precision CG (Low-Precision Preconditioner) ===\n\n";
	print_2p_header();

	std::vector<CG2PResult> results2p;

	{	auto [i, r, f] = run_cg_two_precision<float, half>(N, MAX_ITER, TOL);
		results2p.push_back({"2P-1", "half (fp16)", "float (fp32)", i, r, f, r < TOL}); }
	{	auto [i, r, f] = run_cg_two_precision<float, bfloat_t>(N, MAX_ITER, TOL);
		results2p.push_back({"2P-2", "bfloat16", "float (fp32)", i, r, f, r < TOL}); }
	{	auto [i, r, f] = run_cg_two_precision<double, float>(N, MAX_ITER, TOL);
		results2p.push_back({"2P-3", "float (fp32)", "double (fp64)", i, r, f, r < TOL}); }
	{	auto [i, r, f] = run_cg_two_precision<double, half>(N, MAX_ITER, TOL);
		results2p.push_back({"2P-4", "half (fp16)", "double (fp64)", i, r, f, r < TOL}); }
	{	auto [i, r, f] = run_cg_two_precision<posit<32,2>, posit<16,1>>(N, MAX_ITER, TOL);
		results2p.push_back({"2P-5", "posit<16,1>", "posit<32,2>", i, r, f, r < TOL}); }
	{	auto [i, r, f] = run_cg_two_precision<dd, float>(N, MAX_ITER, TOL);
		results2p.push_back({"2P-6", "float (fp32)", "dd (2x64)", i, r, f, r < TOL}); }

	for (const auto& r : results2p) print_2p_row(r);

	// =================================================================
	// Section 6: Convergence vs problem size
	// =================================================================
	std::cout << "\n=== Section 6: Convergence vs Problem Size ===\n\n";
	std::cout << std::left  << std::setw(8)  << "N"
	          << std::setw(10) << "kappa"
	          << std::setw(10) << "float"
	          << std::setw(10) << "double"
	          << std::setw(12) << "posit<32,2>"
	          << std::setw(10) << "dd"
	          << '\n';
	std::cout << std::string(60, '-') << '\n';

	for (size_t sz : { 8, 16, 32, 64, 128 }) {
		double k = 4.0 * sz * sz / (M_PI * M_PI);
		auto r1 = run_cg_experiment<float>(     "", "", sz, MAX_ITER, TOL);
		auto r2 = run_cg_experiment<double>(    "", "", sz, MAX_ITER, TOL);
		auto r3 = run_cg_experiment<posit<32,2>>("","", sz, MAX_ITER, TOL);
		auto r4 = run_cg_experiment<dd>(         "", "", sz, MAX_ITER, TOL);

		auto fmt = [&](const CGResult& c) -> std::string {
			return c.converged ? std::to_string(c.iterations) : "DNF";
		};

		std::cout << std::left  << std::setw(8)  << sz
		          << std::setw(10) << std::fixed << std::setprecision(0) << k
		          << std::setw(10) << fmt(r1)
		          << std::setw(10) << fmt(r2)
		          << std::setw(12) << fmt(r3)
		          << std::setw(10) << fmt(r4)
		          << '\n';
	}

	// =================================================================
	// Legend
	// =================================================================
	std::cout << R"(

Legend
------
  Config      Shorthand label
  Scalar Type Number system used for all CG operations
  Iters       Iterations to converge (DNF = did not finish in 500 iters)
  ||r||_2     Final residual 2-norm
  Fwd Error   ||x* - x||_inf  (x* = [1,1,...,1])

Key observations:

  1. CG convergence on tridiag(-1,2,-1) is theoretically bounded by
     sqrt(kappa(A)) iterations for exact arithmetic.  For N=32,
     kappa ~ 414, so we expect ~20 iterations in exact arithmetic.
     Finite precision adds overhead depending on the unit roundoff.

  2. Half precision (5 significand bits) and bfloat16 (7 significand bits)
     may fail to converge or need many more iterations because rounding
     errors destroy A-orthogonality of the CG search directions.

  3. Posit<32,2> typically converges in the same iteration count as
     IEEE float — both have ~24 bits of significand near 1.0.

  4. The two-precision configurations show that a low-precision
     preconditioner (half or bfloat16) paired with a higher-precision
     solver can recover full convergence.  The preconditioner only
     needs to be a rough approximation of A^{-1}.

  5. Double-double (dd) converges in fewer iterations than double
     because the 106-bit significand preserves A-orthogonality
     more faithfully.

  6. CG is fragile for ill-conditioned problems.  As kappa grows,
     low-precision types fail earlier.  For non-SPD or non-symmetric
     systems, consider IDR(s) which is more robust (see idrs.cpp).

References:
  Hestenes, M. R. and Stiefel, E. (1952). "Methods of Conjugate Gradients
  for Solving Linear Systems." J. Res. Nat. Bur. Standards, 49(6), 409-436.

  Greenbaum, A. (1997). "Iterative Methods for Solving Linear Systems."
  SIAM Frontiers in Applied Mathematics.
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
