#pragma once
// ftcs_fd1D.hpp: generate finite difference forward-time-centered-space(FTCS) in 1D
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/blas/blas.hpp>

namespace sw { namespace unum { namespace blas { 

// generate a finite difference equation matrix for 1D problems
template<typename Matrix>
void ftcs_fd1D(Matrix& A, size_t m, size_t n) {
	A.resize(m, n);
	using Scalar = typename Matrix::value_type;
	for (size_t j = 0; j < n; ++j) {
		for (size_t i = 0; i < m; ++i) {
			if (j == i - 1 || j == i + 1) {
				A(i, j) = Scalar(-1);
			}
			else if (j == i) {
				A(i, j) = Scalar(2);
			}
			else {
				A(i, j) = Scalar(0);
			}
		}
	}
}

}}} // namespace sw::unum::blas
