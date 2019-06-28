// fdp.hpp :  include file containing templated C++ interfaces to fused dot product
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <vector>

namespace sw {
	namespace unum {

// dot product: the operator vector::x[index] is limited to uint32_t, so the arguments are limited to uint32_t as well
// since we do not support arbitrary posit configuration conversions, the element type of the vectors x and y are declared to be the same.
// TODO: investigate if the vector<> index is always a 32bit entity?
template<typename Ty>
Ty dot(size_t n, const std::vector<Ty>& x, size_t incx, const std::vector<Ty>& y, size_t incy) {
	Ty sum_of_products = 0;
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
		Ty product = x[ix] * y[iy];
		sum_of_products += product;
	}
	return sum_of_products;
}

/// //////////////////////////////////////////////////////////////////
/// fused dot product operators
/// fdp_qc         fused dot product with quire continuation
/// fdp_stride     fused dot product with non-negative stride
/// fdp            fused dot product of two vectors

// Fused dot product with quire continuation
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, size_t n, const Vector& x, size_t incx, const Vector& y, size_t incy) {
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix = ix + incx, iy = iy + incy) {
		sum_of_products += sw::unum::quire_mul(x[ix], y[iy]);
	}
}

// Resolved fused dot product, with the option to control capacity bits in the quire
template<typename Vector, size_t capacity = 10>
typename Vector::value_type fdp_stride(size_t n, const Vector& x, size_t incx, const Vector& y, size_t incy) {
	constexpr size_t nbits = Vector::value_type::nbits;
	constexpr size_t es = Vector::value_type::es;
	quire<nbits, es, capacity> q = 0;
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix = ix + incx, iy = iy + incy) {
		q += sw::unum::quire_mul(x[ix], y[iy]);
		if (sw::unum::_trace_quire_add) std::cout << q << '\n';
	}
	typename Vector::value_type sum;
	convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
	return sum;
}

// Specialized resolved fused dot product that assumes unit stride and a standard vector,
// with the option to control capacity bits in the quire
template<typename Vector, size_t capacity = 10>
typename Vector::value_type fdp(const Vector& x, const Vector& y) {
	constexpr size_t nbits = Vector::value_type::nbits;
	constexpr size_t es = Vector::value_type::es;
	quire<nbits, es, capacity> q = 0;
	size_t ix, iy, n = std::size(x);
	for (ix = 0, iy = 0; ix < n && iy < n; ++ix, ++iy) {
		q += sw::unum::quire_mul(x[ix], y[iy]);
	}
	typename Vector::value_type sum;
	convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
	return sum;
}

#ifdef BLAS_L2
// LEVEL 2 BLAS operators
template<typename Ty>
void matvec(const std::vector<Ty>& A, const std::vector<Ty>& x, std::vector<Ty>& b) {
	// preconditions
	size_t d = x.size();
	assert(A.size() == d*d);
	assert(b.size() == d);
	for (size_t i = 0; i < d; ++i) {
		b[i] = 0;
		for (size_t j = 0; j < d; ++j) {
			//std::cout << "b[" << i << "] = " << b[i] << std::endl;
			//std::cout << "A[" << i << "][" << j << "] = " << A[i*d + j] << std::endl;
			//std::cout << "x[" << j << "] = " << x[j] << std::endl;
			b[i] = b[i] + A[i*d + j] * x[j];
		}
		//std::cout << "b[" << i << "] = " << b[i] << std::endl;
	}
}

// leverage template parameter inference to specialize matvec to use the quire when the inputs are posit vectors
template<size_t nbits, size_t es, size_t capacity = 10>
void matvec(const std::vector< posit<nbits, es> >& A, const std::vector< posit<nbits, es> >& x, std::vector< posit<nbits, es> >& b) {
	// preconditions
	size_t d = x.size();
	assert(A.size() == d*d);
	assert(b.size() == d);
	for (size_t i = 0; i < d; ++i) {
		b[i] = 0;
		quire<nbits, es, capacity> q;   // initialized to 0 by constructor
		for (size_t j = 0; j < d; ++j) {
			q += quire_mul(A[i*d + j], x[j]);
			if (sw::unum::_trace_quire_add) std::cout << q << '\n';
		}  
		convert(q.to_value(), b[i]);  // one and only rounding step of the fused-dot product
		//std::cout << "b[" << i << "] = " << b[i] << std::endl;
	}
}
#endif  // BLAS_L2

#ifdef BLAS_L3
// LEVEL 3 BLAS operators

// matrix-matrix multiplication
template<typename Ty>
void matmul(const std::vector<Ty>& A, const std::vector<Ty>& B, std::vector<Ty>& C) {
	// preconditions
	size_t d = size_t(std::sqrt(A.size()));
	assert(A.size() == d*d);
	assert(B.size() == d*d);
	assert(C.size() == d*d);
	for (size_t i = 0; i < d; ++i) {
		for (size_t j = 0; j < d; ++j) {
			C[i*d + j] = Ty(0);
			for (size_t k = 0; k < d; ++k) {
				C[i*d + j] = C[i*d + j] + A[i*d + k] * B[k*d + j];
			}
		}
	}
}

// leverage template parameter inference to specialize matmul to use the quire when the inputs are posit vectors
template<size_t nbits, size_t es, size_t capacity = 10>
void matmul(const std::vector<posit<nbits,es> >& A, const std::vector< posit<nbits, es> >& B, std::vector< posit<nbits, es> >& C) {
	// preconditions
	size_t d = size_t(std::sqrt(A.size()));
	assert(A.size() == d*d);
	assert(B.size() == d*d);
	assert(C.size() == d*d);
	for (size_t i = 0; i < d; ++i) {
		for (size_t j = 0; j < d; ++j) {
			C[i*d + j] = 0;
			quire<nbits, es, capacity> q;   // initialized to 0 by constructor
			for (size_t k = 0; k < d; ++k) {
				// C[i*d + j] = C[i*d + j] + A[i*d + k] * B[k*d + j];
				q += quire_mul(A[i*d + k], B[k*d + j]);
				if (sw::unum::_trace_quire_add) std::cout << q << '\n';
			}
			convert(q.to_value(), C[i*d + j]);  // one and only rounding step of the fused-dot product
		}
	}
}

#endif  // BLAS_L3

} // namespace unum
} // namespace sw

