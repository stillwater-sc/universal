#pragma once
// inverse.hpp: Gauss-Jordan algorithm to generate matrix inverse
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <utility>  // std::swap
#include <numeric/containers.hpp>

// compilation flags
// BLAS_TRACE_ROUNDING_EVENTS
// when set traces the quire operations
#ifndef BLAS_TRACE_ROUNDING_EVENTS
#define BLAS_TRACE_ROUNDING_EVENTS 0
#endif

namespace sw { namespace blas {

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gauss-Jordan algorithm to generate a matrix inverse

	// partial pivoting only swaps rows
	// full pivoting swaps rows and columns and requires bookkeeping to order the solution correctly

	// optimal pivot: simply pick the largest absolute value element
	// this would make the pivoting dependent on scaling
	// implicit pivoting: pre-scale all equations so that their largest coefficient is unity

// full pivoting Gauss-Jordan inverse without implicit pivoting, returns a null matrix when A is singular
template<typename Scalar>
matrix<Scalar> inv(const matrix<Scalar>& A) {
	using std::fabs;
	using size_type = typename matrix<Scalar>::size_type;
	size_type N = num_rows(A);
	if (N != num_cols(A)) {  // LCOV_EXCL_START
		std::cerr << "inv matrix argument is not square: (" << num_rows(A) << " x " << num_cols(A) << ")\n";
		return matrix<Scalar>{};
	}  // LCOV_EXCL_STOP
	matrix<Scalar> B(A);
	vector<size_t> indxc(N), indxr(N), indxp(N);
	size_type irow = 0, icol = 0;
	for (size_type i = 0; i < N; ++i) {
		// find largest absolute value to select as pivot
		// scan across all NxN indices but skip the row/column that we have already processed (indxp == 1)
		Scalar pivot = 0; 
		for (size_type j = 0; j < N; ++j) {
			if (indxp[j] != 1) {  // skip the row/column if already processed
				for (size_type k = 0; k < N; ++k) {
//					std::cout << "iteration (" << j << "," << k << ")\n";
					if (indxp[k] == 0) {
						Scalar e = fabs(B(j,k));
						if (e > pivot) {  // > emphasizes upper left, >= emphasizes lower right
							pivot = e;
							irow = j;
							icol = k;
						}
					}
					else if (indxp[k] > 1) {  // LCOV_EXCL_START
						std::cerr << "inv matrix argument is singular at machine precision\n";
						return matrix<Scalar>{};
					}  // LCOV_EXCL_STOP
//					std::cout << "[" << irow << ", " << icol << "] = " << pivot << std::endl;
				}
			}
		}
		if (indxp[icol] == 1) {  // LCOV_EXCL_START
			std::cerr << "inv matrix argument is singular at machine precision\n";
			return matrix<Scalar>{};
		}  // LCOV_EXCL_STOP
		++(indxp[icol]);

		// we now have the pivot element
//		std::cout << " pivot value : " << pivot << " at (" << irow << "," << icol << ")\n";
//		std::cout << " pivot index : " << indxp << std::endl;

		// put the pivot on the diagonal 
		if (irow != icol) {
			for (size_type l = 0; l < N; ++l) std::swap(B(irow, l), B(icol, l));
		}
//		std::cout << "matrix B\n" << B << std::endl;
		indxr[i] = irow;
		indxc[i] = icol;
		if (B(icol, icol) == 0.0) {  // LCOV_EXCL_START
			std::cerr << "inv matrix argument is singular\n";
			return matrix<Scalar>{};
		}  // LCOV_EXCL_STOP
		auto normalizer = Scalar(1.0) / B(icol, icol);
		B(icol, icol) = Scalar(1.0);
		for (size_type l = 0; l < N; ++l) B(icol, l) *= normalizer;
//		std::cout << "matrix B\n" << B << std::endl;
		for (size_type ll = 0; ll < N; ++ll) { // reduce the rows
			if (ll != icol) {  // skip the row with the pivot
				auto dum = B(ll, icol);
				B(ll, icol) = Scalar(0);
				for (size_type l = 0; l < N; ++l) B(ll, l) -= B(icol, l) * dum;
			}
		}
//		std::cout << "matrix B\n" << B << std::endl;
	}
	// unscramble the solution by interchanging pairs of columns in the reverse order that the permutation was constructed
	for (size_type l = N; l > 0; --l) {
		if (indxr[l-1] != indxc[l-1]) {
			for (size_type k = 0; k < N; ++k) std::swap(B(k, indxr[l-1]), B(k, indxc[l-1]));
		}
	}
	return B;
}

// non-pivoting Gauss-Jordan inverse
template<typename Matrix>
Matrix invfast(const Matrix& A) {
	using size_type = typename Matrix::size_type;
	using Scalar = typename Matrix::value_type;
	size_type N = num_rows(A);
	Matrix B(A);
	Matrix Ainv(num_rows(A), num_cols(A));
	Ainv = 1;
	for (size_type j = 0; j < N; ++j) {  // for each column
		for (size_type i = 0; i < N; ++i) { // normalize each row
			if (i == j) {
				auto normalizer = Scalar(1.0) / B[j][j];
				for (size_type k = 0; k < N; ++k) {
					B[i][k] = normalizer * B[i][k];
					Ainv[i][k] = normalizer * Ainv[i][k];
				}
			}
			else {
				auto normalizer = B(i, j) / B(j, j);
				for (size_type k = 0; k < N; ++k) {
					B[i][k] -= normalizer * B[j][k];
					Ainv[i][k] -= normalizer * Ainv[j][k];
				}
			}
		}
	}
	return Ainv;
}

} }  // namespace sw::blas
