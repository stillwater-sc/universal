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
    
	for (size_t i = n-1; i >=0 ;--i){
        Scalar y = 0.0;
        for (size_t j = i; j < n; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/A(i,i);
    }

	return x;
}


template<unsigned nbits, unsigned es>
vector<posit<nbits,es>> backsub(const matrix<posit<nbits,es>> & A, const vector<posit<nbits,es>>& b) {
	// using Scalar = typename Matrix::value_type;
	size_t n = size(b);
    using Vector = vector<posit<nbits,es>>;
    constexpr unsigned capacity = 10;

    Vector x(n);
	for (size_t i = n-1; i >=0; --i){
        quire<nbits,es,capacity> q{0};
        for (size_t j = i; j < n; ++j){
            q += quire_mul(A(i,j), x(j));
        }
        posit<nbits,es> y;
        convert(q.to_value(), y); 
        x(i) = (b(i) - y)/A(i,i);
    }
	return x;
}



}}} // namespace sw::universal::blas
