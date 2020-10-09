#pragma once
// index_matrix.hpp: linear index matrix generator
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>
#include <algorithm>

namespace sw { namespace unum { namespace blas {

// fill a dense matrix with linear index values in row order
template <typename Matrix>
void row_order_index(Matrix& A) {
	typedef typename Matrix::value_type    value_type;
	typedef typename Matrix::size_type     size_type;

	// generate linear row index
	size_t index = 0;
	for (size_type r = 0; r < num_rows(A); r++) {
		for (size_type c = 0; c < num_cols(A); c++) {
			A[r][c] = value_type(index++);
		}
	}
}

// fill a dense matrix with linear index values in column order
template <typename Matrix>
void column_order_index(Matrix& A) {
	typedef typename Matrix::value_type    value_type;
	typedef typename Matrix::size_type     size_type;

	// generate linear row index
	size_t index = 0;
	for (size_type c = 0; c < num_cols(A); c++) {
		for (size_type r = 0; r < num_rows(A); r++) {
			A[r][c] = value_type(index++);
		}
	}
}

}}} // namespace sw::unum::blas
