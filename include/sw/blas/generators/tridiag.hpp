#pragma once
// tridiag.hpp: generate tridiagonal matrix finite difference forward-time-centered-space(FTCS) in 1D
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric/containers.hpp>

namespace sw { namespace universal { namespace blas { 

// return a new tridiagonal matrix
template<typename Scalar>
matrix<Scalar> tridiag(typename matrix<Scalar>::size_type N, Scalar subdiag = Scalar(-1.0), Scalar diagonal = Scalar(2.0), Scalar superdiag = Scalar(-1.0)) {
	matrix<Scalar> A;
	tridiag(A, N, subdiag, diagonal, superdiag);
	return A;
}

// generate a finite difference equation matrix for 1D problems
template<typename Scalar>
void tridiag(matrix<Scalar>& A, typename matrix<Scalar>::size_type N, Scalar subdiag = Scalar(-1.0), Scalar diagonal = Scalar(2.0), Scalar superdiag = Scalar(-1.0)) {
	using size_type = typename matrix<Scalar>::size_type;
	A.resize(N, N);
	for (size_type j = 0; j < N; ++j) {
		for (size_type i = 0; i < N; ++i) {
			if (j == i - 1) {
				A(i, j) = subdiag;
			}
			else if (j == i + 1) {
				A(i, j) = superdiag;
			}
			else if (j == i) {
				A(i, j) = diagonal;
			}
			else {
				A(i, j) = Scalar(0);
			}
		}
	}
}

}}} // namespace sw::universal::blas
