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
	for (int i = 1; i < n; ++i){
        Scalar y = 0.0;
        for (int j = 0; j < i; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/d(i);
    }
	return x;
}

}}}
