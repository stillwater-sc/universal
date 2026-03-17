#pragma once
// fdp_blas.hpp: reproducible matrix-vector and matrix-matrix multiply routines using fused dot product
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
// Consumer must include the appropriate quire/fdp headers for their Scalar type before this header
#include <numeric/containers.hpp>
#include <blas/exceptions.hpp>

namespace sw { namespace blas {
	using namespace sw::numeric::containers;

///////////////////////////////////////////////////////////////////////////////////
// fused matrix-vector product
//
// A times x = b fused matrix-vector product
template<typename Scalar>
vector<Scalar> fmv(const matrix<Scalar>& A, const vector<Scalar>& x) {
	// preconditions
	assert(A.cols() == size(x));
	vector<Scalar> b(size(x));

#if BLAS_TRACE_ROUNDING_EVENTS
	unsigned errors = 0;
#endif
	size_t nr = size(b);
	size_t nc = size(x);
	for (size_t i = 0; i < nr; ++i) {
		sw::universal::quire<Scalar> q;
		for (size_t j = 0; j < nc; ++j) {
			q += sw::universal::quire_mul(A(i, j), x[j]);
		}
		b[i] = sw::universal::quire_resolve(q);     // one and only rounding step of the fused-dot product
#if BLAS_TRACE_ROUNDING_EVENTS
		sw::universal::quire<Scalar> qdiff = q;
		sw::universal::quire<Scalar> qsum;
		qsum = b[i];
		qdiff -= qsum;
		if (!qdiff.iszero()) {
			++errors;
			std::cout << "q    : " << q << std::endl;
			std::cout << "qsum : " << qsum << std::endl;
			std::cout << "qdiff: " << qdiff << std::endl;
			Scalar roundingError = sw::universal::quire_resolve(qdiff);
			std::cout << "matvec b[" << i << "] = " << hex_format(b[i]) << " rounding error: " << hex_format(roundingError) << " " << roundingError << std::endl;
		}
#endif
	}
#if BLAS_TRACE_ROUNDING_EVENTS
	if (errors) {
		std::cout << "Universal-BLAS: tracing found " << errors << " rounding errors in matvec operation\n";
	}
#endif
	return b;
}

///////////////////////////////////////////////////////////////////////////////////
// fused matrix-matrix product
//
// A times B = C fused matrix-matrix product
template<typename Scalar>
matrix<Scalar> fmm(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	// preconditions
	assert(A.cols() == B.rows());

	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	if (A.cols() != B.rows()) throw sw::blas::matmul_incompatible_matrices(sw::blas::incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());  // LCOV_EXCL_LINE
	size_t rows = A.rows();
	size_t cols = B.cols();
	size_t dots = A.cols();
	matrix<Scalar> C(rows, cols);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			sw::universal::quire<Scalar, capacity> q;
			for (size_t k = 0; k < dots; ++k) {
				q += sw::universal::quire_mul(A(i, k), B(k, j));
			}
			C(i, j) = sw::universal::quire_resolve(q); // one and only rounding step of the fused-dot product
		}
	}
	return C;
}

}} // namespace sw::blas
