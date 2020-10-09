#pragma once
// tridiag.hpp: generate tridiagonal matrix finite difference forward-time-centered-space(FTCS) in 1D
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/blas/blas.hpp>

namespace sw { namespace unum { namespace blas { 

// generate a finite difference equation matrix for 1D problems
template<typename Scalar>
void tridiag(matrix<Scalar>& A, size_t N, Scalar subdiag = Scalar(-1.0), Scalar diagonal = Scalar(2.0), Scalar superdiag = Scalar(-1.0)) {
	A.resize(N, N);
	for (size_t j = 0; j < N; ++j) {
		for (size_t i = 0; i < N; ++i) {
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

}}} // namespace sw::unum::blas
