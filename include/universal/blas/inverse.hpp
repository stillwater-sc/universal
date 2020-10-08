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


// These routines are written with separate source S and
// destination D matrices so the source matrix can be retained
// if desired.  However, the compact schemes were designed to
// perform in-place computations to save memory.  In
// other words, S and D can be the SAME matrix.  

// Crout implements an in-place LU decomposition, that is, S and D can be the same
// Crout uses unit diagonals for the upper triangle


/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gauss-Jordan algorithm to generate a matrix inverse

// Crout method using MTL data structures
template<typename Matrix>
Matrix inv(const Matrix& S, Matrix& D) {
	assert(num_rows(S) == num_rows(D));
	assert(num_cols(S) == num_cols(D));
	using value_type = typename Matrix::value_type;
	size_t N = num_rows(S);
	for (size_t k = 0; k < N; ++k) {
		for (size_t i = k; i < N; ++i) {
			value_type sum = 0.;
			for (size_t p = 0; p < k; ++p) sum += D[i][p] * D[p][k];
			D[i][k] = S[i][k] - sum; // not dividing by diagonals
		}
		for (size_t j = k + 1; j < N; ++j) {
			value_type sum = 0.;
			for (size_t p = 0; p < k; ++p) sum += D[k][p] * D[p][j];
			D[k][j] = (S[k][j] - sum) / D[k][k];
		}
	}
}

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
				auto c = Scalar(1.0) / B[j][j];
				for (size_t k = 0; k < N; ++k) {
					B[i][k] = c * B[i][k];
					Ainv[i][k] = c * Ainv[i][k];
				}
			}
			else {
				auto c = B(i, j) / B(j, j);
				//std::cout << "pivot (" << i << ", " << j << "): " << B(i,j) << " / " << B(j,j) << " = " << c << std::endl;
				for (size_t k = 0; k < N; ++k) {
					B[i][k] = B[i][k] - c * B[j][k];
					Ainv[i][k] = Ainv[i][k] - c * Ainv[j][k];
				}
			}
//			std::cout << "col(" << j << ")\n" << B;
//			std::cout << "--\n" << Ainv << std::endl;
		}
	}
	return Ainv;
}

} } }  // namespace sw::unum::blas
