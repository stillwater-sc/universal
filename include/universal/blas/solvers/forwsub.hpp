#pragma once
// forwsub.hpp: Forward substitution to solve Ax = b 
//              Input: Matrix A, Vector b, bool lower
//              Inplace forward sub. Uses only lower tri.
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// James Quinlan 2022-11-05
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/blas/matrix.hpp>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

template<typename Matrix, typename Vector>
Vector forwsub(const Matrix& A, const Vector& b, bool lower = false) {
	using Scalar = typename Matrix::value_type;
	size_t n = size(b);
    Vector x(n);
    Vector d(n,1);
    
    if (lower){d = diag(A);}  
    
    x(0) = b(0)/d(0);
	for (size_t i = 1; i < n; ++i){
        Scalar y = 0.0;
        for (size_t j = 0; j < i; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/d(i);
    }
	return x;
}


template<unsigned nbits, unsigned es>
vector<posit<nbits,es>> forwsub(const matrix<posit<nbits,es>> & A, const vector<posit<nbits,es>>& b, bool lower = false) {
	// using Scalar = typename Matrix::value_type;
	size_t n = size(b);
    using Vector = vector<posit<nbits,es>>;
    constexpr unsigned capacity = 20;

    Vector x(n);
    Vector d(n,1);
    
    if (lower){d = diag(A);}  
    
    x(0) = b(0)/d(0);
	for (size_t i = 1; i < n; ++i){
        quire<nbits,es,capacity> q{0};
        for (size_t j = 0; j < i; ++j){
            q += quire_mul(A(i,j), x(j));
        }
        posit<nbits,es> y;
        convert(q.to_value(), y); 
        x(i) = (b(i) - y)/d(i);
    }
	return x;
}




}}}
