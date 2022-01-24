#pragma once
// index.hpp: generate a linear index matrix 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>

namespace sw { namespace universal { namespace blas {

// fill a dense (M, N) matrix with linear index values in row order
template <typename Scalar>
matrix<Scalar> row_order_index(size_t M, size_t N, Scalar start = 1) {
	using Matrix = matrix<Scalar>;
	typedef typename Matrix::size_type     size_type;

	Matrix A(M, N);
	// generate linear row index
	Scalar index = start;
	for (size_type r = 0; r < M; r++) {
		for (size_type c = 0; c < N; c++) {
			A[r][c] = index;
			index += Scalar(1.0);
		}
	}
	return A;
}

// fill a dense (M, N) matrix with linear index values in column order
template <typename Scalar>
matrix<Scalar> column_order_index(size_t M, size_t N, Scalar start = 1) {
	using Matrix = matrix<Scalar>;
	typedef typename Matrix::size_type     size_type;

	Matrix A(M, N);
	// generate linear row index
	Scalar index = start;
	for (size_type c = 0; c < N; c++) {
		for (size_type r = 0; r < M; r++) {
			A[r][c] = index;
			index += Scalar(1.0);
		}
	}
	return A;
}

}}} // namespace sw::universal::blas
