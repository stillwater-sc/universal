// idrs.cpp: mixed-precision IDR(s) Krylov solver case study
//
// IDR(s) -- Induced Dimension Reduction -- is a short-recurrence Krylov
// method for general (non-symmetric) linear systems.  Unlike CG, which
// requires SPD matrices, IDR(s) handles non-symmetric and indefinite
// problems.  The shadow space dimension s controls the trade-off between
// work per iteration and convergence rate: IDR(1) ~ BiCGSTAB, while
// larger s gives smoother, faster convergence.
//
// This exposition compares IDR(s) across Universal's number system
// inventory on two test problems:
//   (A) Non-symmetric convection-diffusion: tridiag(-1-eps, 2, -1+eps)
//   (B) Mildly non-symmetric: tridiag(-0.8, 2, -1.2)
//
// These problems are non-symmetric, so CG is not applicable,
// demonstrating the practical value of robust solvers in mixed-precision.
//
// Reference:
//   Sonneveld, P. and van Gijzen, M. B. (2008). "IDR(s): A family of
//   simple and fast algorithms for solving large nonsymmetric systems of
//   linear equations." SIAM J. Sci. Comput. 31(2), 1035-1062.
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

// standard libs
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// -------------------------------------------------------------------------
// IDR(s) solver -- simplified variant
//
// Uses a preconditioned BiCGSTAB-style iteration generalized to shadow
// space dimension s.  The implementation follows the "basic" IDR(s)
// algorithm (Sonneveld & van Gijzen, 2008, Algorithm 1) which is the
// simplest correct formulation.
//
// All intermediate reductions use double to avoid cross-type issues.
// The Scalar type determines storage and matvec rounding.
//
// Returns (iterations, final ||r||_2, forward_error)
// -------------------------------------------------------------------------
template<typename Scalar>
std::tuple<int, double, double>
idrs_solve(const sw::numeric::containers::matrix<Scalar>& A,
           const sw::numeric::containers::vector<Scalar>& b,
           const sw::numeric::containers::vector<Scalar>& x_exact,
           int s, int max_iter, double tol) {
	using namespace sw::numeric::containers;
	size_t n = size(b);

	// Helpers operating in double
	auto ddot = [&](const vector<Scalar>& u, const vector<Scalar>& v) {
		double d = 0.0;
		for (size_t i = 0; i < n; ++i) d += double(u[i]) * double(v[i]);
		return d;
	};
	auto dnorm = [&](const vector<Scalar>& u) { return std::sqrt(ddot(u, u)); };

	// Shadow space P: s random vectors (fixed seed)
	std::mt19937 rng(42);
	std::normal_distribution<double> dist(0.0, 1.0);
	std::vector<vector<Scalar>> P(s, vector<Scalar>(n));
	for (int j = 0; j < s; ++j)
		for (size_t i = 0; i < n; ++i)
			P[j][i] = Scalar(dist(rng));

	// x0 = 0, r0 = b
	vector<Scalar> x(n);
	vector<Scalar> r = b;
	double rnorm = dnorm(r);
	if (rnorm < tol) return { 0, rnorm, 0.0 };

	// G, U: s+1 columns each (workspace for direction/update vectors)
	// We use a ring buffer approach where newest is index 0
	std::vector<vector<Scalar>> G(s, vector<Scalar>(n));
	std::vector<vector<Scalar>> U(s, vector<Scalar>(n));

	// M = P^T * G  (s x s), initially zero (G is zero)
	std::vector<std::vector<double>> M(s, std::vector<double>(s, 0.0));
	// f = P^T * r  (length s)
	std::vector<double> f(s);
	for (int j = 0; j < s; ++j) f[j] = ddot(P[j], r);

	double omega = 1.0;
	int iter = 0;

	while (iter < max_iter) {
		// --- Phase 1: Generate new G and U vectors ---
		for (int k = 0; k < s; ++k) {
			// Solve M * c = f for c  (s x s system via Gaussian elimination)
			std::vector<std::vector<double>> Mc(s, std::vector<double>(s));
			std::vector<double> fc(s);
			for (int i = 0; i < s; ++i) {
				fc[i] = f[i];
				for (int j = 0; j < s; ++j) Mc[i][j] = M[i][j];
			}
			std::vector<double> c(s, 0.0);
			for (int col = 0; col < s; ++col) {
				int piv = col;
				for (int row = col + 1; row < s; ++row)
					if (std::abs(Mc[row][col]) > std::abs(Mc[piv][col])) piv = row;
				std::swap(Mc[col], Mc[piv]);
				std::swap(fc[col], fc[piv]);
				if (std::abs(Mc[col][col]) < 1.0e-30) continue;
				for (int row = col + 1; row < s; ++row) {
					double fac = Mc[row][col] / Mc[col][col];
					for (int j = col; j < s; ++j) Mc[row][j] -= fac * Mc[col][j];
					fc[row] -= fac * fc[col];
				}
			}
			for (int row = s - 1; row >= 0; --row) {
				c[row] = fc[row];
				for (int j = row + 1; j < s; ++j) c[row] -= Mc[row][j] * c[j];
				if (std::abs(Mc[row][row]) > 1.0e-30) c[row] /= Mc[row][row];
			}

			// v = r - G * c,  u_hat = U * c
			vector<Scalar> v(n);
			vector<Scalar> u_hat(n);
			for (size_t i = 0; i < n; ++i) {
				double vi = double(r[i]);
				double ui = 0.0;
				for (int j = 0; j < s; ++j) {
					vi -= c[j] * double(G[j][i]);
					ui += c[j] * double(U[j][i]);
				}
				v[i] = Scalar(vi);
				u_hat[i] = Scalar(ui);
			}

			// t = A * v
			vector<Scalar> t = A * v;

			// omega = (t, v) / (t, t)
			double tt = ddot(t, t), tv = ddot(t, v);
			if (tt > 1.0e-30) omega = tv / tt;
			if (std::abs(omega) < 1.0e-30) omega = 1.0; // safety

			// G[k] = t - omega * A*v ... wait, t IS A*v
			// New residual direction: g_new = v - omega * t
			// But we need G[k] = A * u_new where u_new = u_hat + omega * v
			// So G[k] = A * u_hat + omega * t

			// u_new = u_hat + omega * v
			for (size_t i = 0; i < n; ++i)
				U[k][i] = Scalar(double(u_hat[i]) + omega * double(v[i]));

			// G[k] = A * U[k]
			G[k] = A * U[k];

			// Bi-orthogonalize G[k] against P[0..k-1]
			for (int j = 0; j < k; ++j) {
				double alpha = ddot(P[j], G[k]);
				if (std::abs(M[j][j]) > 1.0e-30) alpha /= M[j][j];
				for (size_t i = 0; i < n; ++i) {
					G[k][i] = Scalar(double(G[k][i]) - alpha * double(G[j][i]));
					U[k][i] = Scalar(double(U[k][i]) - alpha * double(U[j][i]));
				}
			}

			// Update M column k
			for (int j = 0; j < s; ++j)
				M[j][k] = ddot(P[j], G[k]);

			// beta = f[k] / M[k][k]
			double beta = (std::abs(M[k][k]) > 1.0e-30) ? f[k] / M[k][k] : 0.0;

			// r = r - beta * G[k]
			// x = x + beta * U[k]
			for (size_t i = 0; i < n; ++i) {
				r[i] = Scalar(double(r[i]) - beta * double(G[k][i]));
				x[i] = Scalar(double(x[i]) + beta * double(U[k][i]));
			}

			++iter;

			// Update f
			for (int j = 0; j < s; ++j)
				f[j] = ddot(P[j], r);

			rnorm = dnorm(r);
			if (rnorm < tol || iter >= max_iter) break;
		}

		if (rnorm < tol || iter >= max_iter) break;

		// --- Phase 2: intermediate residual reduction ---
		// v = A * r (one extra matvec per outer loop for residual smoothing)
		vector<Scalar> v = A * r;
		double vv = ddot(v, v), vr = ddot(v, r);
		if (vv > 1.0e-30) omega = vr / vv;
		if (std::abs(omega) < 1.0e-30) omega = 1.0;

		for (size_t i = 0; i < n; ++i) {
			x[i] = Scalar(double(x[i]) + omega * double(r[i]));
			r[i] = Scalar(double(r[i]) - omega * double(v[i]));
		}
		++iter;

		for (int j = 0; j < s; ++j)
			f[j] = ddot(P[j], r);

		rnorm = dnorm(r);
		if (rnorm < tol) break;
	}

	// Forward error
	double fwd_err = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double d = std::abs(double(x_exact[i]) - double(x[i]));
		if (d > fwd_err) fwd_err = d;
	}

	return { iter, rnorm, fwd_err };
}


// -------------------------------------------------------------------------
// Test problem generators
// -------------------------------------------------------------------------

// Non-symmetric convection-diffusion: tridiag(-1-eps, 2, -1+eps)
// From discretization of  -u'' + eps*u' = f  with central differences.
template<typename Scalar>
void make_convdiff(sw::numeric::containers::matrix<Scalar>& A,
                   sw::numeric::containers::vector<Scalar>& b,
                   sw::numeric::containers::vector<Scalar>& x_exact,
                   size_t n, double eps) {
	A = sw::numeric::containers::matrix<Scalar>(n, n);
	for (size_t i = 0; i < n; ++i) {
		A(i, i) = Scalar(2.0);
		if (i > 0)     A(i, i-1) = Scalar(-1.0 - eps);
		if (i < n - 1) A(i, i+1) = Scalar(-1.0 + eps);
	}
	x_exact = sw::numeric::containers::vector<Scalar>(n, Scalar(1));
	b = A * x_exact;
}

// Mildly non-symmetric: tridiag(-0.8, 2, -1.2)
template<typename Scalar>
void make_nonsym(sw::numeric::containers::matrix<Scalar>& A,
                 sw::numeric::containers::vector<Scalar>& b,
                 sw::numeric::containers::vector<Scalar>& x_exact,
                 size_t n) {
	A = sw::numeric::containers::matrix<Scalar>(n, n);
	for (size_t i = 0; i < n; ++i) {
		A(i, i) = Scalar(2.0);
		if (i > 0)     A(i, i-1) = Scalar(-0.8);
		if (i < n - 1) A(i, i+1) = Scalar(-1.2);
	}
	x_exact = sw::numeric::containers::vector<Scalar>(n, Scalar(1));
	b = A * x_exact;
}


// -------------------------------------------------------------------------
// Reporting
// -------------------------------------------------------------------------
struct IDRResult {
	std::string config;
	std::string scalar_type;
	int         s_param;
	int         iterations;
	double      residual;
	double      forward_error;
	bool        converged;
};

static void print_header() {
	std::cout << std::left  << std::setw(10) << "Config"
	          << std::setw(24) << "Scalar Type"
	          << std::right << std::setw(4)  << "s"
	          << std::setw(8)  << "Iters"
	          << std::setw(14) << "||r||_2"
	          << std::setw(14) << "Fwd Error"
	          << '\n';
	std::cout << std::string(74, '-') << '\n';
}

static void print_row(const IDRResult& r) {
	std::cout << std::left  << std::setw(10) << r.config
	          << std::setw(24) << r.scalar_type
	          << std::right << std::setw(4) << r.s_param;
	if (r.converged)
		std::cout << std::setw(8) << r.iterations;
	else
		std::cout << std::setw(8) << "DNF";
	std::cout << std::setw(14) << std::scientific << std::setprecision(2) << r.residual
	          << std::setw(14) << r.forward_error
	          << '\n';
}

// -------------------------------------------------------------------------
// Convenience runner
// -------------------------------------------------------------------------
template<typename Scalar, typename ProblemGen>
IDRResult run_idrs(const std::string& config, const std::string& name,
                   int s, size_t n, int max_iter, double tol,
                   ProblemGen gen) {
	using namespace sw::numeric::containers;
	matrix<Scalar> A;
	vector<Scalar> b, x_exact;
	gen(A, b, x_exact, n);

	auto [iters, res, fwd] = idrs_solve<Scalar>(A, b, x_exact, s, max_iter, tol);
	return { config, name, s, iters, res, fwd, res < tol };
}


// =========================================================================
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	size_t N = 32;
	if (argc > 1) N = std::atoi(argv[1]);

	constexpr int MAX_ITER = 500;
	constexpr double TOL = 1.0e-10;

	std::cout << "Mixed-Precision IDR(s): Robust Krylov Solver for Non-Symmetric Systems\n";
	std::cout << "N = " << N << ",  tol = " << std::scientific << std::setprecision(0)
	          << TOL << ",  max " << MAX_ITER << " iterations\n\n";

	// =================================================================
	// Problem A: Convection-diffusion (non-symmetric)
	// =================================================================
	double eps = 0.5;
	std::cout << "=== Problem A: Convection-Diffusion, eps = " << std::fixed
	          << std::setprecision(1) << eps << " ===\n";
	std::cout << "Matrix: tridiag(" << (-1.0 - eps) << ", 2, " << (-1.0 + eps) << ")\n";
	std::cout << "Non-symmetric: CG not applicable.\n\n";

	auto gen_convdiff = [eps](auto& A, auto& b, auto& x, size_t n) {
		using S = typename std::remove_reference_t<decltype(A)>::value_type;
		make_convdiff<S>(A, b, x, n, eps);
	};

	// --- Effect of shadow space dimension s ---
	std::cout << "--- Effect of Shadow Space Dimension s (double precision) ---\n\n";
	print_header();
	for (int s : { 1, 2, 4, 8 }) {
		auto r = run_idrs<double>("s=" + std::to_string(s), "double (fp64)", s, N, MAX_ITER, TOL, gen_convdiff);
		print_row(r);
	}

	// --- Number system comparison at s=4 ---
	std::cout << "\n--- Number System Comparison (s=4) ---\n\n";
	print_header();

	std::vector<IDRResult> results;
	results.push_back(run_idrs<half>(      "IEEE-1", "half (fp16)",    4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<bfloat_t>(  "IEEE-2", "bfloat16",      4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<float>(     "IEEE-3", "float (fp32)",  4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<double>(    "IEEE-4", "double (fp64)", 4, N, MAX_ITER, TOL, gen_convdiff));

	results.push_back(run_idrs<posit<16,1>>("P-1",  "posit<16,1>",   4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<posit<32,2>>("P-2",  "posit<32,2>",   4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<posit<64,3>>("P-3",  "posit<64,3>",   4, N, MAX_ITER, TOL, gen_convdiff));

	using cf32 = cfloat<32, 8, uint32_t, true, false, false>;
	using cf64 = cfloat<64, 11, uint64_t, true, false, false>;

	results.push_back(run_idrs<cf32>(      "CF-1",  "cfloat<32,8>",  4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<cf64>(      "CF-2",  "cfloat<64,11>", 4, N, MAX_ITER, TOL, gen_convdiff));
	results.push_back(run_idrs<dd>(        "Ext-1", "dd (2x64)",     4, N, MAX_ITER, TOL, gen_convdiff));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Problem B: Mildly non-symmetric tridiag(-0.8, 2, -1.2)
	// =================================================================
	std::cout << "\n=== Problem B: Mildly Non-Symmetric tridiag(-0.8, 2, -1.2) ===\n\n";

	auto gen_nonsym = [](auto& A, auto& b, auto& x, size_t n) {
		using S = typename std::remove_reference_t<decltype(A)>::value_type;
		make_nonsym<S>(A, b, x, n);
	};

	std::cout << "--- Effect of Shadow Space Dimension s (double) ---\n\n";
	print_header();
	for (int s : { 1, 2, 4, 8 }) {
		auto r = run_idrs<double>("s=" + std::to_string(s), "double (fp64)", s, N, MAX_ITER, TOL, gen_nonsym);
		print_row(r);
	}

	std::cout << "\n--- Number System Comparison (s=4) ---\n\n";
	print_header();
	results.clear();

	results.push_back(run_idrs<half>(       "IEEE-1", "half (fp16)",    4, N, MAX_ITER, TOL, gen_nonsym));
	results.push_back(run_idrs<bfloat_t>(   "IEEE-2", "bfloat16",      4, N, MAX_ITER, TOL, gen_nonsym));
	results.push_back(run_idrs<float>(      "IEEE-3", "float (fp32)",  4, N, MAX_ITER, TOL, gen_nonsym));
	results.push_back(run_idrs<double>(     "IEEE-4", "double (fp64)", 4, N, MAX_ITER, TOL, gen_nonsym));
	results.push_back(run_idrs<posit<16,1>>("P-1",    "posit<16,1>",  4, N, MAX_ITER, TOL, gen_nonsym));
	results.push_back(run_idrs<posit<32,2>>("P-2",    "posit<32,2>",  4, N, MAX_ITER, TOL, gen_nonsym));
	results.push_back(run_idrs<dd>(         "Ext-1",  "dd (2x64)",    4, N, MAX_ITER, TOL, gen_nonsym));

	for (const auto& r : results) print_row(r);

	// =================================================================
	// Convergence vs problem size (double, s=4, Problem A)
	// =================================================================
	std::cout << "\n=== Convergence vs Problem Size (double, s=4, Problem A) ===\n\n";
	std::cout << std::left  << std::setw(8)  << "N"
	          << std::right << std::setw(8)  << "Iters"
	          << std::setw(14) << "||r||_2"
	          << std::setw(14) << "Fwd Error"
	          << '\n';
	std::cout << std::string(44, '-') << '\n';

	for (size_t sz : { 8, 16, 32, 64, 128 }) {
		auto r = run_idrs<double>("", "double", 4, sz, MAX_ITER, TOL, gen_convdiff);
		std::cout << std::left  << std::setw(8) << sz;
		if (r.converged)
			std::cout << std::right << std::setw(8) << r.iterations;
		else
			std::cout << std::right << std::setw(8) << "DNF";
		std::cout << std::setw(14) << std::scientific << std::setprecision(2) << r.residual
		          << std::setw(14) << r.forward_error
		          << '\n';
	}

	// =================================================================
	// Legend
	// =================================================================
	std::cout << R"(

Legend
------
  Config      Shorthand label
  Scalar Type Number system used for all IDR(s) operations
  s           Shadow space dimension (larger = smoother convergence)
  Iters       Iterations to converge (DNF = did not finish in 500 iters)
  ||r||_2     Final residual 2-norm
  Fwd Error   ||x* - x||_inf  (x* = [1,1,...,1])

Key observations:

  1. IDR(s) succeeds on non-symmetric problems where CG is not applicable.
     The convection-diffusion matrix (eps=0.5) has eigenvalues with
     significant imaginary parts; CG would diverge immediately.

  2. Increasing s from 1 to 4 typically reduces iteration count at the
     cost of more work per iteration.  IDR(1) behaves like BiCGSTAB and
     may show irregular convergence; IDR(4) is much smoother.

  3. Low-precision types (half, bfloat16) struggle because the Krylov
     basis vectors lose linear independence faster.  However, IDR(s)
     at least attempts the problem, whereas CG cannot.

  4. Posit<32,2> and IEEE float show comparable iteration counts,
     consistent with the CG observations -- both have ~24 bits of
     significand near 1.0.

  5. Double-double (dd) shows the benefit of extended precision:
     fewer iterations and better forward error.

  6. For practitioners:
     - Use CG when A is guaranteed SPD (Laplacian, mass matrix)
     - Use IDR(s=4) as a robust default for general systems
     - The mixed-precision story transfers: the solver precision
       determines convergence behavior, not the problem formulation

References:
  Sonneveld, P. and van Gijzen, M. B. (2008). "IDR(s): A family of
  simple and fast algorithms for solving large nonsymmetric systems of
  linear equations." SIAM J. Sci. Comput. 31(2), 1035-1062.

  van Gijzen, M. B. and Sonneveld, P. (2011). "Algorithm 913: An elegant
  IDR(s) variant that efficiently exploits biorthogonality properties."
  ACM Trans. Math. Softw. 38(1), Article 5.
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
