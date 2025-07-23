#pragma once
//
// forwsub.hpp: Forward substitution to solve Ax = b
// Input : Matrix A, Vector b, bool lower
// Inplace forward sub.Uses only lower tri.
//
// @author:     James Quinlan
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// @date:       2022-12-17
// @modified:   2023-10-10
//
#include <numeric/containers.hpp>

#include <universal/number/posit/posit.hpp>

namespace sw { namespace blas {

    template<unsigned nbits, unsigned es, unsigned capacity = 10>
    vector<posit<nbits,es>> forwsub(const matrix<posit<nbits,es>> & A, const vector<posit<nbits,es>>& b, bool lower = false) {
        size_t n = size(b);
        using Scalar = posit<nbits, es>;
        using Vector = vector<Scalar>;
        using Quire  = quire<nbits,es,capacity>;
    
        Vector x(n);
        Vector d(n,1);
    
        if (lower){d = diag(A);}  
    
        x(0) = b(0)/d(0);
	    for (size_t i = 1; i < n; ++i){
            Quire q{0};
            for (size_t j = 0; j < i; ++j){
                q += quire_mul(A(i,j), x(j));
            }
            Scalar y;
            convert(q.to_value(), y); 
        
            x(i) = (lower) ? (b(i) - y)/d(i) : (b(i) - y);
        }
	    return x;
    }

}}
