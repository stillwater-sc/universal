#pragma once
// matrix.hpp: super-simple dense matrix implementation
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <vector>

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

	Scalar operator()(size_t i, size_t j) const { return data[i*n + j]; }
	Scalar& operator()(size_t i, size_t j) { return data[i*n + j]; }

	void setzero() {
		for (auto& elem : data) {
			elem = Scalar(0);
		}
	}
	size_t rows() const { return m; }
	size_t cols() const { return n; }

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