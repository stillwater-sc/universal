// polynomial_roots.cpp: Wilkinson's polynomial roots in double vs efloat (issue #1100)
//
// Wilkinson's polynomial W(x) = (x-1)(x-2)...(x-20) has the obvious roots
// 1,2,...,20. Expanded into power form its coefficients are enormous (up to
// ~1.4e19) and several exceed 2^53, so storing them as double perturbs them --
// and the roots of W are famously hypersensitive to coefficient perturbation.
//
// This program expands W exactly in efloat, then asks the same question with the
// SAME Newton kernel in each type: starting from each true root location i, is i
// still a root of the stored polynomial? With exact efloat coefficients it is
// (Newton stays at i); with rounded double coefficients it is not (Newton drifts
// away), so double cannot recover the integer roots and efloat can.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstdlib>
#include <exception>
#include <vector>
#include <iostream>
#include <iomanip>
#include <universal/number/efloat/efloat.hpp>

namespace {
constexpr int DEGREE = 20;

// Expand W(x) = prod_{i=1..DEGREE} (x - i) into coefficients c[k] of x^k.
template<typename Real>
std::vector<Real> expand_wilkinson() {
	std::vector<Real> c(DEGREE + 1, Real(0.0));
	c[0] = Real(1.0);
	for (int i = 1; i <= DEGREE; ++i) {  // multiply the current polynomial by (x - i)
		for (int k = DEGREE; k >= 1; --k)
			c[k] = c[k - 1] - Real(static_cast<double>(i)) * c[k];
		c[0] = Real(-static_cast<double>(i)) * c[0];
	}
	return c;
}

template<typename Real>
Real poly(const std::vector<Real>& c, const Real& x) {  // Horner
	Real p(0.0);
	for (int k = DEGREE; k >= 0; --k)
		p = p * x + c[k];
	return p;
}

template<typename Real>
Real dpoly(const std::vector<Real>& c, const Real& x) {  // Horner on the derivative
	Real p(0.0);
	for (int k = DEGREE; k >= 1; --k)
		p = p * x + c[k] * Real(static_cast<double>(k));
	return p;
}

// Newton's method from x0; refines a root of the stored polynomial.
template<typename Real>
Real newton(const std::vector<Real>& c, double x0, int iters) {
	Real x(x0);
	for (int i = 0; i < iters; ++i) {
		Real d = dpoly(c, x);
		if (double(d) == 0.0)
			break;
		x = x - poly(c, x) / d;
	}
	return x;
}
}  // namespace

int main() try {
	using namespace sw::universal;
	using efloat512 = efloat<16>;  // 16 limbs * 32 = 512 bits

	// Exact coefficients in efloat; the same values rounded to double.
	std::vector<efloat512> ce = expand_wilkinson<efloat512>();
	std::vector<double>    cd(ce.size());
	for (std::size_t k = 0; k < ce.size(); ++k)
		cd[k] = double(ce[k]);

	std::cout << "Wilkinson's polynomial W(x) = (x-1)(x-2)...(x-" << DEGREE << "),  roots 1.." << DEGREE << "\n";
	std::cout << "Expanded coefficients reach ~1.4e19; several exceed 2^53, so double cannot store them exactly.\n";

	// Largest absolute error introduced by rounding a coefficient to double.
	double worst  = 0.0;
	int    worstk = 0;
	for (int k = 0; k <= DEGREE; ++k) {
		efloat512 e = ce[k] - efloat512(cd[k]);
		e.setsign(false);
		if (double(e) > worst) {
			worst  = double(e);
			worstk = k;
		}
	}
	std::cout << std::scientific << std::setprecision(3);
	std::cout << "  largest coefficient rounding error: the x^" << worstk << " coefficient is off by " << worst
	          << " when stored as double\n\n";

	// Same Newton kernel from each true root location i, in each type.
	std::cout << "  " << std::left << std::setw(4) << "i" << std::setw(20) << "double root" << std::setw(13) << "|err|"
	          << std::setw(20) << "efloat root" << "|err|\n";
	std::cout << "  " << std::string(62, '-') << "\n";
	double max_d = 0.0, max_e = 0.0;
	for (int i = 1; i <= DEGREE; ++i) {
		double    rd = double(newton<double>(cd, i, 100));
		efloat512 re = newton<efloat512>(ce, i, 100);
		double    ed = std::fabs(rd - i);
		double    ee = std::fabs(double(re) - i);
		max_d        = std::max(max_d, ed);
		max_e        = std::max(max_e, ee);
		std::cout << "  " << std::left << std::fixed << std::setprecision(9) << std::setw(4) << i << std::setw(20) << rd
		          << std::scientific << std::setprecision(2) << std::setw(13) << ed << std::fixed
		          << std::setprecision(9) << std::setw(20) << double(re) << std::scientific << std::setprecision(2)
		          << ee << "\n";
	}

	std::cout << "\n  worst error over all 20 roots:  double = " << std::scientific << std::setprecision(3) << max_d
	          << "   efloat = " << max_e << "\n";
	std::cout << "  Rounding the coefficients to double moved the roots off the integers by up to ~"
	          << std::setprecision(0) << max_d << "; efloat's exact coefficients keep every root at its integer.\n";

	return EXIT_SUCCESS;
} catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
