#pragma once
// forwsub.hpp: Forward substitution to solve Ax = b given A = lower triangular 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// @jquinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
// #include <universal/number/posit/posit_fwd.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

template<typename Matrix, typename Vector>
Vector forwsub(const Matrix& A, const Vector& b) {
	using Scalar = typename Matrix::value_type;
	size_t n = size(b);
    Vector x(n);
    
    x(0) = b(0)/A(0,0);
	for (int i = 1; i < n; ++i){
        Scalar y = 0.0;
        for (int j = 0; j < i; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/A(i,i);
    }

	return x;
}

}}} // namespace sw::universal::blas
