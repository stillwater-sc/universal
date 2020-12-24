#pragma once
// blas_l2.hpp: BLAS Level 2 functions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
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

#ifdef QUIRE_ENABLED_MATVEC
// Matrix-vector product: b = A * x, posit specialized
template<size_t nbits, size_t es>
void matvec(sw::unum::blas::vector< sw::unum::posit<nbits, es> >& b, const sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& A, const sw::unum::blas::vector< sw::unum::posit<nbits, es> >& x) {
	// preconditions
	assert(A.cols() == size(x));
	assert(size(b) == size(x));

#if BLAS_TRACE_ROUNDING_EVENTS
	unsigned errors = 0;
#endif
	size_t nr = size(b);
	size_t nc = size(x);
	for (size_t i = 0; i < nr; ++i) {
		sw::unum::quire<nbits, es> q(0);
		for (size_t j = 0; j < nc; ++j) {
			q += sw::unum::quire_mul(A(i,j), x[j]);
		}
		sw::unum::convert(q.to_value(), b[i]);     // one and only rounding step of the fused-dot product
#if BLAS_TRACE_ROUNDING_EVENTS
		sw::unum::quire<nbits, es> qdiff = q;
		sw::unum::quire<nbits, es> qsum = b[i];
		qdiff -= qsum;
		if (!qdiff.iszero()) {
			++errors;
			std::cout << "q    : " << q << std::endl;
			std::cout << "qsum : " << qsum << std::endl;
			std::cout << "qdiff: " << qdiff << std::endl;
			sw::unum::posit<nbits, es> roundingError;
			convert(qdiff.to_value(), roundingError);
			std::cout << "matvec b[" << i << "] = " << hex_format(b[i]) << " rounding error: " << hex_format(roundingError) << " " << roundingError << std::endl;
		}
#endif
	}
#if BLAS_TRACE_ROUNDING_EVENTS
	if (errors) {
		std::cout << "HPR-BLAS: tracing found " << errors << " rounding errors in matvec operation\n";
	}
#endif
}
#endif // QUIRE_ENABLED_MATVEC

// A times x = b fused matrix-vector product
template<size_t nbits, size_t es>
sw::unum::blas::vector< sw::unum::posit<nbits, es> > fmv(const sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& A, const sw::unum::blas::vector< sw::unum::posit<nbits, es> >& x) {
	// preconditions
	assert(A.cols() == size(x));
	sw::unum::blas::vector< sw::unum::posit<nbits, es> > b(size(x));

#if BLAS_TRACE_ROUNDING_EVENTS
	unsigned errors = 0;
#endif
	size_t nr = size(b);
	size_t nc = size(x);
	for (size_t i = 0; i < nr; ++i) {
		sw::unum::quire<nbits, es> q(0);
		for (size_t j = 0; j < nc; ++j) {
			q += sw::unum::quire_mul(A(i,j), x[j]);
		}
		sw::unum::convert(q.to_value(), b[i]);     // one and only rounding step of the fused-dot product
#if BLAS_TRACE_ROUNDING_EVENTS
		sw::unum::quire<nbits, es> qdiff = q;
		sw::unum::quire<nbits, es> qsum = b[i];
		qdiff -= qsum;
		if (!qdiff.iszero()) {
			++errors;
			std::cout << "q    : " << q << std::endl;
			std::cout << "qsum : " << qsum << std::endl;
			std::cout << "qdiff: " << qdiff << std::endl;
			sw::unum::posit<nbits, es> roundingError;
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
