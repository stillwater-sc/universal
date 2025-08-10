#pragma once
// frank.hpp: generate a Frank matrix 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>

namespace sw { namespace blas {

// fill a dense (N, N) matrix with linear index values in row order
template <typename Scalar>
matrix<Scalar> frank(unsigned N) {
	using Matrix = matrix<Scalar>;
	// precondition tests
	if (N <= 0) return matrix<Scalar>{};
	if (N % 2 == 0) {
		std::cerr << "matrix size N is even, must be odd" << std::endl;
		return matrix<Scalar>{};
	}

	Matrix A(N, N);
/*
 * A frank matrix is a Hessenberg matrix with ill conditioned eigenvalues, 
 * whose determinant is 1. The elements may be optionally reflected about 
 * the anti-diagonal.
 * The matrix has all positive eigenvalues and they occur in reciprocal pairs 
 * (so that 1 is an eigenvalue if the order is odd). The eigenvalues may 
 * be obtained in terms of the zeros of the Hermite polynomials. 
 * The smallest half of the eigenvalues are ill conditioned, the more so 
 * for larger order.
 *
 * Frank matrix of size N for 0-based indices
 *   A(i, j) = 0        j <= i - 2
 *             N - i    j  = i - 1
 *             N - j    j >= i
 * 
 *  [ N   N-1 N-2 .... 2  1 ]
 *  [ N-1 N-1 N-2 .... 2  1 ]
 *  [ 0   N-2 N-3 .... 2  1 ]
 *  [ 0   0   N-3 .... 2  1 ]
 *                ....
 *  [ 0   0   0   .... 1  1 ]
*/
	for (unsigned i = 0; i < N; ++i) {
		for (unsigned j = 0; j < N; ++j) {
			if (j + 2 <= i) {
				A(i, j) = Scalar(0);
			}
			else if (j + 1 == i) {
				A(i, j) = Scalar(N - i);
			}
			else if (j >= i) {
				A(i, j) = Scalar(N - j);
			}
			else {
				std::cerr << "unassigned condition " << i << " , " << j << std::endl;
			}
		}
	}
	return A;
}

}} // namespace sw::blas
