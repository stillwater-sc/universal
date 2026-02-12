#pragma once
// luq.hpp: in-place dense matrix LU decomposition
//
// * Assume A = PA, else send to plu
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// James Quinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// #include <universal/utility/directives.hpp>
#include <universal/number/posit1/posit_fwd.hpp>
#include <universal/blas/matrix.hpp>

namespace sw { namespace universal { namespace blas {  

template<typename Scalar>
void luq(matrix<Scalar>& A){ 
    unsigned n = num_rows(A);

    // Gaussian Elimination Process
    for (size_t i = 0; i < n - 1; ++i){ // i-th row
        for (size_t k = i + 1; k < n; ++k){  // objective row
            A(k,i) =  A(k,i) / A(i,i);
            for (size_t j = i+1; j < n; ++j){
                A(k,j) -= A(k,i)*A(i,j);
            }
        }
    }
} // LU


}}} // namespace sw::universal::blas

/*

template<unsigned nbits, unsigned es>
vector<posit<nbits,es>> backsub(const matrix<posit<nbits,es>> & A, const vector<posit<nbits,es>>& b) {
	// using Scalar = typename Matrix::value_type;
	size_t n = size(b);
    using Vector = vector<posit<nbits,es>>;
    constexpr unsigned capacity = 10;

    Vector x(n);
	for (int i = n-1; i >=0; --i){
        quire<nbits,es,capacity> q{0};
        for (int j = i; j < n; ++j){
            q += quire_mul(A(i,j), x(j));
        }
        posit<nbits,es> y;
        convert(q.to_value(), y); 
        x(i) = (b(i) - y)/A(i,i);
    }
	return x;
}
*/