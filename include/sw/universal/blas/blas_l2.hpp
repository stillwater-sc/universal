#pragma once
// blas_l2.hpp: BLAS Level 2 functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>

// compilation flags
// BLAS_TRACE_ROUNDING_EVENTS
// when set traces the quire operations
#ifndef BLAS_TRACE_ROUNDING_EVENTS
#define BLAS_TRACE_ROUNDING_EVENTS 0
#endif

namespace sw { namespace universal { namespace blas {

	// Matrix-vector product: b = A * x, no quire for posit values
	template<typename Matrix, typename Vector>
	void matvec(Vector& b, const Matrix& A, const Vector& x) {
		using Scalar = typename Vector::value_type;
		for (size_t i = 0; i < A.rows(); ++i) {
			b[i] = Scalar(0);
			for (size_t j = 0; j < A.cols(); ++j) {
				b[i] += A(i, j) * x[j];
			}
		}
	}

}}}  // namespace sw::universal::blas
