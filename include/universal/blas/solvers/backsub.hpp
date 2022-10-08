#pragma once
// backsub.hpp: Backsubstitution to solve Ax = b given A = upper triangular 
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
Vector backsub(const Matrix& A, const Vector& b) {
	using Scalar = typename Matrix::value_type;
	size_t n = size(b);
    Vector x(n);
    
	for (int i = n-1; i >=0 ;--i){
        Scalar y = 0.0;
        for (int j = i; j < n; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/A(i,i);
        // std::cout << i << "   " <<   x(i) << std::endl; 
    }

	return x;
}

}}} // namespace sw::universal::blas
