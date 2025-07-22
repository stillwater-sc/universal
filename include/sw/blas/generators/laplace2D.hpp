#pragma once
// laplace2D.hpp: generate 2D Laplace operator difference matrix on a square domain
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric/containers.hpp>

namespace sw { namespace universal { namespace blas { 

// generate a 2D square domain Laplacian difference equation matrix
template<typename Scalar>
void laplace2D(matrix<Scalar>& A, size_t m, size_t n) {
	A.resize(m*n, m*n);
	A.setzero();
	assert(A.rows() == m * n);
	for (size_t i = 0; i < m; ++i) {
		for (size_t j = 0; j < n; ++j) {
			Scalar four(4.0), minus_one(-1.0);
			size_t row = i * n + j;
			A(row, row) = four;
			if (j < n - 1) A(row, row + 1) = minus_one;
			if (i < m - 1) A(row, row + n) = minus_one;
			if (j > 0) A(row, row - 1) = minus_one;
			if (i > 0) A(row, row - n) = minus_one;
		}
	}
}

}}} // namespace sw::universal::blas
