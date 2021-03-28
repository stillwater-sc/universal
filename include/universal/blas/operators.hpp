#pragma once
// operators.hpp: matrix operators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>

namespace sw { namespace universal { namespace blas { 

// generate identity matrix
template<typename Scalar>
matrix<Scalar> eye(size_t N) {
	matrix<Scalar> I(N, N);
	I = Scalar(1.0f);
	return I;
}

// generate transposed matrix
template<typename Scalar>
matrix<Scalar> transpose(const matrix<Scalar>& A) {
	matrix<Scalar> B(A);
	return B.transpose();
}

// return the diagonal of the matrix
template<typename Scalar>
vector<Scalar> diag(const matrix<Scalar>& A) {
	using size_type = typename matrix<Scalar>::size_type;

	size_type m = num_rows(A);
	size_type n = num_cols(A);
	size_type lowerbound = (m < n) ? m : n;
	vector<Scalar> v(lowerbound);
	for (size_type i = 0; i < lowerbound; ++i) {
		v[i] = A(i,i);
	}
	return v;
}

// return a full rank matrix given a vector representing its diagonal
template<typename Scalar>
matrix<Scalar> diag(const vector<Scalar>& d) {
	using size_type = typename matrix<Scalar>::size_type;
	size_type m = size(d);
	matrix<Scalar> A(m,m);
	for (size_type i = 0; i < m; ++i) {
		A[i][i] = d[i];
	}
	return A;
}
//compute minor in-place
template<typename Scalar>
matrix<Scalar> minor(const matrix<Scalar>& A, size_t x=1){
    using size_type = typename matrix<Scalar>::size_type;
    matrix<Scalar> tmp(A);
    for (size_t i = 0; i < x; i++){ 
        tmp[i,i] = size_type(1.0);
    }
    for (size_t i = x; i < num_rows(A); i++){
        for (size_t j = x; j < num_cols(A); j++){
            tmp[i][j] = A[i][j];
        }
    }
    return tmp;
}
//get xth column
template<typename Scalar>
void get_col(matrix<Scalar>& A, vector<Scalar>& v, size_t x){
    using size_type = typename matrix<Scalar>::size_type;
    for (size_t i = 0; i < num_rows(A); i++)
    v[i] = A[i][x];
}

// return lower triangular matrix of A
template<typename Scalar>
matrix<Scalar> tril(const matrix<Scalar>& A, size_t k = 0) {
	using size_type = typename matrix<Scalar>::size_type;
	size_type m = num_rows(A);
	size_type n = num_cols(A);
	matrix<Scalar> L(m, n);
	// use row-order traversal
	for (size_type i = 0; i < m; ++i) {
		for (size_type j = 0; j <= static_cast<size_type>((static_cast<int>(i)-static_cast<int>(k))); ++j) {
			L[i][j] = A[i][j];
		}
	}
	return L;
}

// return upper triangular matrix of A
template<typename Scalar>
matrix<Scalar> triu(const matrix<Scalar>& A, size_t k = 0) {
	using size_type = typename matrix<Scalar>::size_type;
	size_type m = num_rows(A);
	size_type n = num_cols(A);
	matrix<Scalar> U(m, n);
	// use row-order traversal
	for (size_type i = 0; i < m; ++i) {
		for (size_type j = i + k; j < n; ++j) {
			U[i][j] = A[i][j];
		}
	}
	return U;
}

}}} // namespace sw::universal::blas
