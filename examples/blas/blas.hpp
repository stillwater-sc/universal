// blas.hpp :  include file containing templated C++ interfaces to BLAS routines
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace blas {

		// sum of magnitudes of the vector elements
		template<typename vector_T>
		vector_T asum(int64_t n, const vector_T& x, int64_t incx) {

		}

		// a time x plus y
		template<typename scale_T, typename vector_T>
		void axpy(int64_t n, scale_T a, const vector_T& x, int64_t incx, vector_T& y, int64_t incy) {

		}

		// vector copy
		template<typename vector_T>
		void copy(int64_t n, const vector_T& x, int64_t incx, vector_T& y, int64_t incy) {

		}

		// dot product: the operator vector::x[index] is limited to uint32_t, so the arguments are limited to uint32_t as well
		// since we do not support arbitrary posit configuration conversions, the element type of the vectors x and y are declared to be the same.
		// TODO: investigate if the vector<> index is always a 32bit entity?
		template<typename Ty>
		Ty dot(uint32_t n, const std::vector<Ty>& x, uint32_t incx, const std::vector<Ty>& y, uint32_t incy) {
			Ty product = 0;
			uint32_t ix, iy;
			for (ix = 0, iy = 0; ix < n && iy < n; ix = ix + incx, iy = iy + incy) {
				product += x[ix] * y[iy];
			}
			return product;
		}

		template<typename Qy, typename Ty>
		void fused_dot(Qy& sum_of_products, uint32_t n, const std::vector<Ty>& x, uint32_t incx, const std::vector<Ty>& y, uint32_t incy) {
			uint32_t ix, iy;
			for (ix = 0, iy = 0; ix < n && iy < n; ix = ix + incx, iy = iy + incy) {
				sum_of_products += sw::unum::quire_mul(x[ix], y[iy]);
			}
		}

		// rotation of points in the plane
		template<typename vector_T>
		void rot(int64_t n, const vector_T& x, int64_t incx, vector_T& y, int64_t incy, vector_T c, vector_T s) {
			// x_i = c*x_i + s*y_i
			// y_i = c*y_i - s*x_i
		}

		// compute parameters for a Givens rotation
		template<typename T>
		void rotg(T& a, T& b, T& c, T&s) {
			// Given Cartesian coordinates (a,b) of a point, return parameters c,s,r, and z associated with the Givens rotation.
		}

		// scale a vector
		template<typename scale_T, typename vector_T>
		void axpy(int64_t n, scale_T a, vector_T& x, int64_t incx) {

		}

		// swap two vectors
		template<typename vector_T>
		void swap(int64_t n, vector_T& x, int64_t incx, vector_T& y, int64_t incy) {

		}

		// find the index of the element with maximum absolute value
		template<typename vector_T>
		int64_t amax(int64_t n, const vector_T& x, int64_t incx) {

		}

		// find the index of the element with minimum absolute value
		template<typename vector_T>
		int64_t amin(int64_t n, const vector_T& x, int64_t incx) {

		}

		// absolute value of a complex number
		template<typename T>
		T cabs(T z) {
		}

	} // namespace blas

} // namespace sw
