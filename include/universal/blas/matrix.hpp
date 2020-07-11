#pragma once
// matrix.hpp: super-simple dense matrix implementation
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <vector>
#include <initializer_list>

namespace sw { namespace unum { namespace blas { 

template<typename Scalar>
class matrix {
public:
	typedef Scalar                            value_type;
	typedef const value_type&                 const_reference;
	typedef value_type&                       reference;
	typedef const value_type*                 const_pointer_type;

	matrix() {}
	matrix(size_t _m, size_t _n) : m{ _m }, n{ _n }, data(m*n, Scalar(0.0)) { }
	matrix(std::initializer_list< std::initializer_list<Scalar> > values) {
		size_t nrows = values.size();
		size_t ncols = values.begin()->size();
		data.resize(nrows * ncols);
		size_t r = 0;
		for (auto l : values) {
			if (l.size() == ncols) {
				size_t c = 0;
				for (auto v : l) {
					data[r*ncols + c] = v;
					++c;
				}
				++r;
			}
		}
	}
	matrix(const matrix& A) : m{ A.m }, n{ A.n }, data(A.data) {}

	// operators
	Scalar operator()(size_t i, size_t j) const { return data[i*n + j]; }
	Scalar& operator()(size_t i, size_t j) { return data[i*n + j]; }


	// modifiers
	void setzero() { for (auto& elem : data) elem = Scalar(0); }

	// selectors
	size_t rows() const { return m; }
	size_t cols() const { return n; }

	// Eigen operators I need to reverse engineer
	matrix Zero(size_t m, size_t n) {
		matrix z(m, n);
		return z;
	}
	matrix transpose() const {
		matrix M(*this);
		return M;
	}
	matrix& diagonal() {

	}

private:
	size_t m, n; // m rows and n columns
	std::vector<Scalar> data;
};

// ostream operator: no need to declare as friend as it only uses public interfaces
template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const matrix<Scalar>& A) {
	size_t m = A.rows();
	size_t n = A.cols();
	for (size_t i = 0; i < m; ++i) {
		for (size_t j = 0; j < n; ++j) {
			ostr << A(i, j) << " ";
		}
		ostr << '\n';
	}
	return ostr;
}

template<typename Scalar>
vector<Scalar> operator*(const matrix<Scalar>& A, const vector<Scalar>& x) {
	vector<Scalar> b(x.size());
	for (size_t i = 0; i < A.rows(); ++i) {
		b[i] = Scalar(0);
		for (size_t j = 0; j < A.cols(); ++j) {
			b[i] += A(i, j) * x[j];
		}
	}
	return b;
}

// create a 2D difference equation matrix of a Laplacian stencil
template<typename Scalar>
void laplacian_setup(matrix<Scalar>& A, size_t m, size_t n) {
	A.setzero();
	assert(A.rows() == m * n);
	for (size_t i = 0; i < m; ++i) {
		for (size_t j = 0; j < n; ++j) {
			Scalar four(4.0), minus_one(-1.0);
			size_t row = i * n + j;
			A(row, row) = four;
			if (j < n - 1) A(row, row + 1) = minus_one;
			if (i < m - 1) A(row, row + n) = minus_one;
			if (j > 0) A(row, row - 1) = minus_one;
			if (i > 0) A(row, row - n) = minus_one;
		}
	}
}

}}} // namespace sw::unum::blas