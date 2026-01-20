#pragma once
// blas_l1.hpp: BLAS Level 1 functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <numeric/containers/vector.hpp>

namespace sw { namespace blas { 
	using namespace sw::numeric::containers;

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

// a times x plus y
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
size_t amax(size_t n, const Vector& x, size_t incx = 1) {
	size_t ix{ 0 }, index{ 0 };
	auto running_max = std::abs(x[ix]);
	for (ix = 1; ix < n; ix += incx) {
		auto absolute = std::abs(x[ix]);
		if (absolute > running_max) {
			index = ix;
			running_max = absolute;
		}
	}
	return index;
}

// find the index of the element with minimum absolute value
template<typename Vector>
size_t amin(size_t n, const Vector& x, size_t incx = 1) {
	size_t ix{ 0 }, index{ 0 };
	auto running_min = std::abs(x[ix]);
	for (ix = 1; ix < n; ix += incx) {
		auto absolute = std::abs(x[ix]);
		if (absolute < running_min) {
			index = ix;
			running_min = absolute;
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

// norms

// L1-norm of a vector
template<typename Scalar>
Scalar normL1(const vector<Scalar>& v) {
	//using namespace sw::universal; // to specialize abs()
	using std::abs;
	Scalar L1Norm{ 0 };
	for (const Scalar& e : v) {
		L1Norm += abs(e);
	}
	return L1Norm;
}

// L2-norm of a vector
template<typename Scalar>
Scalar normL2(const vector<Scalar>& v) {
	//using namespace sw::universal; // to specialize sqrt()
	using std::sqrt;
	Scalar L2Norm{ 0 };
	for (const Scalar& e : v) {
		L2Norm += e * e;
	}
	return sqrt(L2Norm);
}

// L3-norm of a vector
template<typename Scalar>
Scalar normL3(const vector<Scalar>& v) {
	//using namespace sw::universal; // to specialize abs()
	using std::abs, std::pow;
	Scalar L3Norm{ 0 };
	for (const Scalar& e : v) {
		Scalar abse = abs(e);
		L3Norm += abse * abse * abse;
	}
	return pow(L3Norm, Scalar(1) / Scalar(3));
}

// L4-norm of a vector
template<typename Scalar>
Scalar normL4(const vector<Scalar>& v) {
	//using namespace sw::universal; // to specialize abs()
	using std::pow;
	Scalar L4Norm{ 0 };
	for (const Scalar& e : v) {
		Scalar esqr = e * e;
		L4Norm += esqr * esqr;
	}
	return pow(L4Norm, Scalar(1) / Scalar(4));
}

// Linf-norm of a vector
template<typename Scalar>
Scalar normLinf(const vector<Scalar>& v) {
	//using namespace sw::universal; // to specialize abs()
	using std::abs;
	Scalar LinfNorm{ 0 };
	for (const Scalar& e : v) {
		LinfNorm = (abs(e) > LinfNorm) ? abs(e) : LinfNorm;
	}
	return LinfNorm;
}

template<typename Scalar>
Scalar norm(const vector<Scalar>& v, int p) {
	//using namespace sw::universal; // to specialize pow() and abs()
	using std::pow, std::abs;
	Scalar norm{ 0 };
	switch (p) {
	case 0:
		break; //should be the geometric mean
	case 1:
		norm = normL1(v);
		break;
	case 2:
		norm = normL2(v);
		break;
	case 3:
		norm = normL3(v);
		break;
	case 4:
		norm = normL4(v);
		break;
	case std::numeric_limits<int>::max():
		norm = normLinf(v);
		break;
	default:
		{
			Scalar sp = Scalar( p );
			for (const Scalar& e : v) {
				norm += pow(abs(e), sp);
			}
			norm = pow(norm, Scalar( 1 ) / sp);
		}
		break;
	}
	return norm;
}

}} // namespace sw::blas

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
