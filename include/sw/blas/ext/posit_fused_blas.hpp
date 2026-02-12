#pragma once
// posit_fused_blas.hpp: reproducible matrix-vector and matrix-matrix multiply routines for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/number/posit1/posit_fwd.hpp>
#include <numeric/containers.hpp>
#include <blas/exceptions.hpp>

namespace sw { namespace blas {
	using namespace sw::numeric::containers;

///////////////////////////////////////////////////////////////////////////////////
// fused matrix-vector product
//  
// TODO: how would you generalize this to posits, cfloats, lns, integer, or even native int and float?
//
// A times x = b fused matrix-vector product
template<unsigned nbits, unsigned es>
vector< sw::universal::posit<nbits, es> > fmv(const matrix< sw::universal::posit<nbits, es> >& A, const vector< sw::universal::posit<nbits, es> >& x) {
	// preconditions
	assert(A.cols() == size(x));
	vector< sw::universal::posit<nbits, es> > b(size(x));

#if BLAS_TRACE_ROUNDING_EVENTS
	unsigned errors = 0;
#endif
	size_t nr = size(b);
	size_t nc = size(x);
	for (size_t i = 0; i < nr; ++i) {
		sw::universal::quire<nbits, es> q(0);
		for (size_t j = 0; j < nc; ++j) {
			q += sw::universal::quire_mul(A(i, j), x[j]);
		}
		sw::universal::convert(q.to_value(), b[i]);     // one and only rounding step of the fused-dot product
#if BLAS_TRACE_ROUNDING_EVENTS
		sw::universal::quire<nbits, es> qdiff = q;
		sw::universal::quire<nbits, es> qsum = b[i];
		qdiff -= qsum;
		if (!qdiff.iszero()) {
			++errors;
			std::cout << "q    : " << q << std::endl;
			std::cout << "qsum : " << qsum << std::endl;
			std::cout << "qdiff: " << qdiff << std::endl;
			sw::universal::posit<nbits, es> roundingError;
			convert(qdiff.to_value(), roundingError);
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
// TODO: how to generalize this to posit, cfloat, lns, integer, etc.
//
// A times B = C fused matrix-vector product
template<unsigned nbits, unsigned es>
matrix< sw::universal::posit<nbits, es> > fmm(const matrix< sw::universal::posit<nbits, es> >& A, const matrix< sw::universal::posit<nbits, es> >& B) {
	// preconditions
	assert(A.cols() == B.rows());

	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	if (A.cols() != B.rows()) throw sw::blas::matmul_incompatible_matrices(sw::blas::incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());  // LCOV_EXCL_LINE
	size_t rows = A.rows();
	size_t cols = B.cols();
	size_t dots = A.cols();
	matrix< sw::universal::posit<nbits, es> > C(rows, cols);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			sw::universal::quire<nbits, es, capacity> q;
			for (size_t k = 0; k < dots; ++k) {
				q += sw::universal::quire_mul(A(i, k), B(k, j));
			}
			sw::universal::convert(q.to_value(), C(i, j)); // one and only rounding step of the fused-dot product
		}
	}
	return C;
}

}} // namespace sw::blas
