#pragma once
// jacobi.hpp: Jacobi iterative method to solve Ax = b
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/blas/matrix.hpp>

namespace sw::universal::blas {

// Jacobi: Solution of x in Ax=b using Jacobi Method
template<typename Matrix, typename Vector, size_t MAX_ITERATIONS = 100>
size_t Jacobi(const Matrix& A, const Vector& b, Vector& x, typename Matrix::value_type tolerance = typename Matrix::value_type(0.00001)) {
	using Scalar = typename Matrix::value_type;
	Scalar residual = Scalar(std::numeric_limits<Scalar>::max());
	size_t m = num_rows(A);
	size_t n = num_cols(A);
	size_t itr = 0;
	while (residual > tolerance && itr < MAX_ITERATIONS) {
		Vector x_old = x;
		for (size_t i = 0; i < m; ++i) {
			Scalar sigma = 0;
			for (size_t j = 0; j < n; ++j) {
				if (i != j) sigma += A(i, j) * x(j);
			}
			x(i) = (b(i) - sigma) / A(i, i);
		}
		residual = normL1(x_old - x);
		std::cout << '[' << itr << "] " << std::setw(10) << x << "         residual " << residual << std::endl;
		++itr;
	}

	return itr;
}

} // namespace sw::universal::blas
