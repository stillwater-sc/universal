#pragma once
// cg_fdp_fdp.hpp: Conjugate Gradient method with fused-dot product matrix-vector operator and fused-dot product compensation operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit_fwd.hpp>
#include <numeric/containers/matrix.hpp>

namespace sw { namespace blas {

// cg: Solution of x in Ax=b using preconditioned Conjugate Gradient algorithm 
// with different precision for matrix-vector multiply and residual calculation
// Algorithm scheme: fused-dot-product-based matrix-vector, fused-dot-product-based compensation operators
// Input: 
//   preconditioner M
//   system matrix A
//   right hand side b
//   accuracy tolerance for target solution
// Output:
//   number of iterations to reach required accuracy
//   result vector x, by reference
//   vector of residuals, by reference
template<typename Matrix, typename Vector, size_t MAX_ITERATIONS = 10>
size_t cg_fdp_fdp(const Matrix& M, const Matrix& A, const Vector& b, Vector& x, Vector& residuals, typename Matrix::value_type tolerance = typename Matrix::value_type(0.00001)) {
	using Scalar = typename Matrix::value_type;
	Scalar residual = Scalar(std::numeric_limits<Scalar>::max());
	//size_t m = num_rows(A);
	//size_t n = num_cols(A);
	Vector rho(size(b));
	Vector zeta(size(b));
	Vector p(size(b));
	Vector q(size(b));
	Scalar sigma_1{ 0 }, sigma_2{ 0 }, alpha{ 0 }, beta{ 0 };
	rho = b;  // term is b - A * x, but if we use x(0) = 0 vector, rho = b is equivalent
	size_t itr = 0;
	bool firstIteration = true;
	while (residual > tolerance && itr < MAX_ITERATIONS) {
		zeta = M * rho;
		sigma_2 = sigma_1;
		sigma_1 = sw::universal::fdp(zeta, rho); // dot product, fused dot product if Scalar is a posit type
		if (firstIteration) {
			firstIteration = false;
			p = zeta;
		}
		else {
			beta = sigma_1 / sigma_2;
			p = zeta + beta * p;
		}
		q = A * p;
		alpha = sigma_1 / sw::universal::fdp(p, q);
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
	return itr;
}

}} // namespace sw::blas
