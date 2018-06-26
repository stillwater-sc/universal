// blas.hpp :  include file containing templated C++ interfaces to BLAS routines
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <vector>
#include "blas_utils.hpp"

namespace sw {
	namespace blas {

		// LEVEL 1 BLAS operators

		// sum of magnitudes of the vector elements
		template<typename vector_T>
		vector_T asum(size_t n, const vector_T& x, size_t incx) {

		}

		// a time x plus y
		template<typename scale_T, typename vector_T>
		void axpy(size_t n, scale_T a, const vector_T& x, size_t incx, vector_T& y, size_t incy) {
			size_t cnt, ix, iy;
			for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
				y[iy] += a * x[ix];
			}
		}

		// vector copy
		template<typename vector_T>
		void copy(size_t n, const vector_T& x, size_t incx, vector_T& y, size_t incy) {
			size_t cnt, ix, iy;
			for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
				y[iy] = x[ix];
			}
		}

		// dot product: the operator vector::x[index] is limited to uint32_t, so the arguments are limited to uint32_t as well
		// since we do not support arbitrary posit configuration conversions, the element type of the vectors x and y are declared to be the same.
		// TODO: investigate if the vector<> index is always a 32bit entity?
		template<typename Ty>
		Ty dot(size_t n, const std::vector<Ty>& x, size_t incx, const std::vector<Ty>& y, size_t incy) {
			Ty product = 0;
			size_t cnt, ix, iy;
			for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
				product += x[ix] * y[iy];
			}
			return product;
		}
		// fused dot product operators
		// Fused dot product with quire continuation
		template<typename Qy, typename Ty>
		void fused_dot(Qy& sum_of_products, size_t n, const std::vector<Ty>& x, size_t incx, const std::vector<Ty>& y, size_t incy) {
			size_t ix, iy;
			for (ix = 0, iy = 0; ix < n && iy < n; ix = ix + incx, iy = iy + incy) {
				sum_of_products += sw::unum::quire_mul(x[ix], y[iy]);
			}
		}
		// Standalone fused dot product
		template<size_t nbits, size_t es, size_t capacity = 10>
		sw::unum::posit<nbits, es> fused_dot(size_t n, const std::vector< sw::unum::posit<nbits, es> >& x, size_t incx, const std::vector< sw::unum::posit<nbits, es> >& y, size_t incy) {
			sw::unum::quire<nbits, es, capacity> q;   // initialized to 0 by constructor
			size_t ix, iy;
			for (ix = 0, iy = 0; ix < n && iy < n; ix = ix + incx, iy = iy + incy) {
				q += sw::unum::quire_mul(x[ix], y[iy]);
				if (sw::unum::_trace_quire_add) std::cout << q << '\n';
			}
			sw::unum::posit<nbits, es> sum;
			sum.convert(q.to_value());     // one and only rounding step of the fused-dot product
			return sum;
		}

		// rotation of points in the plane
		template<typename rotation_T, typename vector_T>
		void rot(size_t n, vector_T& x, size_t incx, vector_T& y, size_t incy, rotation_T c, rotation_T s) {
			// x_i = c*x_i + s*y_i
			// y_i = c*y_i - s*x_i
			size_t cnt, ix, iy;
			for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
				rotation_T x_i = c*x[ix] + s*y[iy];
				rotation_T y_i = c*y[iy] - s*x[ix];
				y[iy] = y_i;
				x[ix] = x_i;
			}
		}

		// compute parameters for a Givens rotation
		template<typename T>
		void rotg(T& a, T& b, T& c, T&s) {
			// Given Cartesian coordinates (a,b) of a point, return parameters c,s,r, and z associated with the Givens rotation.
		}

		// scale a vector
		template<typename scale_T, typename vector_T>
		void scale(size_t n, scale_T a, vector_T& x, size_t incx) {
			size_t cnt, ix;
			for (cnt = 0, ix = 0; cnt < n && ix < x.size(); ix += incx) {
				x[ix] *= a;
			}
		}

		// swap two vectors
		template<typename vector_T>
		void swap(size_t n, vector_T& x, size_t incx, vector_T& y, size_t incy) {
			size_t cnt, ix, iy;
			for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
				//x.typename tmp = x[ix];
				//x[ix] = y[iy];
				//y[iy] = tmp;
			}
		}

		// find the index of the element with maximum absolute value
		template<typename vector_T>
		size_t amax(size_t n, const vector_T& x, size_t incx) {

		}

		// find the index of the element with minimum absolute value
		template<typename vector_T>
		size_t amin(size_t n, const vector_T& x, size_t incx) {

		}

		// absolute value of a complex number
		template<typename T>
		T cabs(T z) {
		}

		// print a vector
		template<typename vector_T>
		void print(std::ostream& ostr, size_t n, vector_T& x, size_t incx = 1) {
			size_t cnt, ix;
			for (cnt = 0, ix = 0; cnt < n && ix < x.size(); ++cnt, ix += incx) {
				cnt == 0 ? ostr << "[" << x[ix] : ostr << ", " << x[ix];
			}
			ostr << "]";
		}


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

		template<size_t nbits, size_t es>
		void matvec(const std::vector< sw::unum::posit<nbits, es> >& A, const std::vector< sw::unum::posit<nbits, es> >& x, std::vector< sw::unum::posit<nbits, es> >& b) {
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

		template<typename Ty>
		void eye(std::vector<Ty>& I) {
			// preconditions
			int d = int(std::sqrt(I.size()));
			assert(I.size() == d*d);
			for (int i = 0; i < d; ++i) {
				for (int j = 0; j < d; ++j) {
					I[i*d + j] = (i == j ? Ty(1) : Ty(0));
				}
			}
		}

		// LEVEL 3 BLAS operators

		template<typename Ty>
		void matmul(const std::vector<Ty>& A, const std::vector<Ty>& B, std::vector<Ty>& C) {
			// preconditions
			int d = int(std::sqrt(A.size()));
			assert(A.size() == d*d);
			assert(B.size() == d*d);
			assert(C.size() == d*d);
			for (int i = 0; i < d; ++i) {
				for (int j = 0; j < d; ++j) {
					C[i*d + j] = Ty(0);
					for (int k = 0; k < d; ++k) {
						C[i*d + j] = C[i*d + j] + A[i*d + k] * B[k*d + j];
					}
				}
			}
		}

	} // namespace blas

} // namespace sw
