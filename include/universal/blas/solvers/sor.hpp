#pragma once
// sor.hpp: Successive Over Relaxation method
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/blas/matrix.hpp>

namespace sw { namespace universal { namespace blas {

// sor: Solution of x in Ax=b using Successive Over-Relaxation
template<typename Matrix, typename Vector, size_t MAX_ITERATIONS = 100>
size_t sor(const Matrix& A, const Vector& b, Vector& x, typename Matrix::value_type w, typename Matrix::value_type tolerance = typename Matrix::value_type(0.00001)) {
	using Scalar = typename Matrix::value_type;
	Scalar residual = Scalar(std::numeric_limits<Scalar>::max());
	size_t m = num_rows(A);
	size_t n = num_cols(A);
	size_t itr = 0;
	while (residual > tolerance && itr < MAX_ITERATIONS) {
		Vector x_old = x;
		// Gauss-Seidel step
		for (size_t i = 1; i <= m; ++i) {
			Scalar sigma = 0;
			for (size_t j = 1; j <= i - 1; ++j) {
				sigma += A(i - 1, j - 1) * x(j - 1);
			}
			for (size_t j = i + 1; j <= n; ++j) {
				sigma += A(i - 1, j - 1) * x_old(j - 1);
			}
			x(i - 1) = (1 - w) * x_old(i - 1) + w * (b(i - 1) - sigma) / A(i - 1, i - 1);
		}
		residual = norm(x_old - x, 1);
		// std::cout << '[' << itr << "] " << x << " residual " << residual << std::endl;
		++itr;
	}
	std::cout << "over-relaxation factor w is " << w << '\n';
	std::cout << "final residual is " << residual << '\n';
	return itr;
}

}}} // namespace sw::universal::blas
