#pragma once
// inverse.hpp: Gauss-Jordan algorithm to generate matrix inverse
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/blas/matrix.hpp>

// compilation flags
// BLAS_TRACE_ROUNDING_EVENTS
// when set traces the quire operations
#ifndef BLAS_TRACE_ROUNDING_EVENTS
#define BLAS_TRACE_ROUNDING_EVENTS 0
#endif

namespace sw { namespace unum { namespace blas {

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gauss-Jordan algorithm to generate a matrix inverse

// non-pivoting Gauss-Jordan inverse
template<typename Scalar>
matrix<Scalar> inv(const matrix<Scalar>& A) {
	const size_t N = num_rows(A);
	matrix<Scalar> B(A);
	matrix<Scalar> Ainv(num_rows(A), num_cols(A));
	Ainv = 1;
	for (size_t j = 0; j < N; ++j) {  // for each column
		for (size_t i = 0; i < N; ++i) { // normalize each row
			if (i == j) {
				auto normalizer = Scalar(1.0) / B[j][j];
				for (size_t k = 0; k < N; ++k) {
					B[i][k] = normalizer * B[i][k];
					Ainv[i][k] = normalizer * Ainv[i][k];
				}
			}
			else {
				auto normalizer = B(i, j) / B(j, j);
				for (size_t k = 0; k < N; ++k) {
					B[i][k] -= normalizer * B[j][k];
					Ainv[i][k] -= normalizer * Ainv[j][k];
				}
			}
		}
	}
	return Ainv;
}

} } }  // namespace sw::unum::blas
