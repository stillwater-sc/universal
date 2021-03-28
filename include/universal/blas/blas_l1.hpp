#pragma once
// blas_l1.hpp: BLAS Level 1 functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/number/posit/posit>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas { 

// 1-norm of a vector: sum of magnitudes of the vector elements, default increment stride is 1
template<typename Vector>
typename Vector::value_type asum(size_t n, const Vector& x, size_t incx = 1) {
	typename Vector::value_type sum = 0;
	size_t ix;
	for (ix = 0; ix < n; ix += incx) {
		sum += (x[ix] < 0 ? -x[ix] : x[ix]);
	}
	return sum;
}

// sum of the vector elements, default increment stride is 1
template<typename Vector>
typename Vector::value_type sum(const Vector& x) {
	typename Vector::value_type sum = 0;
	size_t ix;
	for (ix = 0; ix < size(x); ++ix) {
		sum += x[ix];
	}
	return sum;
}

// a time x plus y
template<typename Scalar, typename Vector>
void axpy(size_t n, Scalar a, const Vector& x, size_t incx, Vector& y, size_t incy) {
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < size(x) && iy < size(y); ++cnt, ix += incx, iy += incy) {
		y[iy] += a * x[ix];
	}
}

// vector copy
template<typename Vector>
void copy(size_t n, const Vector& x, size_t incx, Vector& y, size_t incy) {
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < size(x) && iy < size(y); ++cnt, ix += incx, iy += incy) {
		y[iy] = x[ix];
	}
}

// adapter for STL vectors
template<typename Scalar> auto size(const std::vector<Scalar>& v) { return v.size(); }

// dot product: the operator vector::x[index] is limited to uint32_t, so the arguments are limited to uint32_t as well
// The library does support arbitrary posit configuration conversions, but to simplify the 
// behavior of the dot product, the element type of the vectors x and y are declared to be the same.
// TODO: investigate if the vector<> index is always a 32bit entity?
template<typename Vector>
typename Vector::value_type dot(size_t n, const Vector& x, size_t incx, const Vector& y, size_t incy) {
	using value_type = typename Vector::value_type;
	value_type sum_of_products = value_type(0);
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < size(x) && iy < size(y); ++cnt, ix += incx, iy += incy) {
		sum_of_products += x[ix] * y[iy];
	}
	return sum_of_products;
}
// specialized dot product assuming constant stride
template<typename Vector>
typename Vector::value_type dot(const Vector& x, const Vector& y) {
	using value_type = typename Vector::value_type;
	value_type sum_of_products = value_type(0);
	size_t nx = size(x);
	if (nx <= size(y)) {
		for (size_t i = 0; i < nx; ++i) {
			sum_of_products += x[i] * y[i];
		}
	}
	return sum_of_products;
}

// rotation of points in the plane
template<typename Rotation, typename Vector>
void rot(size_t n, Vector& x, size_t incx, Vector& y, size_t incy, Rotation c, Rotation s) {
	// x_i = c*x_i + s*y_i
	// y_i = c*y_i - s*x_i
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < size(x) && iy < size(y); ++cnt, ix += incx, iy += incy) {
		Rotation x_i = c * x[ix] + s * y[iy];
		Rotation y_i = c * y[iy] - s * x[ix];
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
template<typename Scalar, typename Vector>
void scale(size_t n, Scalar alpha, Vector& x, size_t incx) {
	size_t cnt, ix;
	for (cnt = 0, ix = 0; cnt < n && ix < size(x); ix += incx) {
		x[ix] *= alpha;
	}
}

// swap two vectors
template<typename Vector>
void swap(size_t n, Vector& x, size_t incx, Vector& y, size_t incy) {
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < size(x) && iy < size(y); ++cnt, ix += incx, iy += incy) {
		typename Vector::value_type tmp = x[ix];
		x[ix] = y[iy];
		y[iy] = tmp;
	}
}

// find the index of the element with maximum absolute value
template<typename Vector>
size_t amax(size_t n, const Vector& x, size_t incx) {
	typename Vector::value_type running_max = -INFINITY;
	size_t ix, index;
	for (ix = 0; ix < size(x); ix += incx) {
		if (x[ix] > running_max) {
			index = ix;
			running_max = x[ix];
		}
	}
	return index;
}

// find the index of the element with minimum absolute value
template<typename Vector>
size_t amin(size_t n, const Vector& x, size_t incx) {
	typename Vector::value_type running_min = INFINITY;
	size_t ix, index;
	for (ix = 0; ix < size(x); ix += incx) {
		if (x[ix] < running_min) {
			index = ix;
			running_min = x[ix];
		}
	}
	return index;
}

// absolute value of a complex number
template<typename T>
T cabs(T z) {
}

// print a vector
template<typename Vector>
void strided_print(std::ostream& ostr, size_t n, Vector& x, size_t incx = 1) {
	size_t cnt, ix;
	for (cnt = 0, ix = 0; cnt < n && ix < size(x); ++cnt, ix += incx) {
		cnt == 0 ? ostr << "[" << x[ix] : ostr << ", " << x[ix];
	}
	ostr << "]";
}


} } } // namespace sw::universal::blas

// free function norms

// norm's of a vector
template<typename Scalar, typename String>
Scalar norm(const std::vector<Scalar>& v, const String s="one_norm"){
    Scalar ans=0;
        if(strcmp(s, "one_norm")){
            for(auto i:v){
                ans+=abs(i);
            }
        } 
        if(strcmp(s, "two_norm")){
            for(auto i:v){
                ans+=i*i;
            }
            ans=Scalar(sqrt(ans));
        }
        if(strcmp(s, "inf_norm")){
            for(auto i:v){
                ans=Scalar(-1e9);
                ans=std::max(ans,abs(i));
            }
        }
        if(strcmp(s, "frobenius_norm")){
            for(auto i:v){
                ans+=abs(i*i);
            }
            ans=Scalar(sqrt(ans));
        }
        return ans;
}
// specializations for STL vectors

template<typename Ty>
Ty minValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
	return *it;
}

template<typename Ty>
Ty maxValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = max_element(samples.begin(), samples.end());
	return *it;
}
