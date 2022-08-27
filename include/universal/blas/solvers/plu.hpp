#pragma once
// plu.hpp: dense matrix PLU decomposition (PA = LU)
//          via DooLittle Method
// auto [P, L, U] = plu(A); // Returns P(ermutation matrix)
//                             L(ower) Triangular
//                             U(pper) Triangular 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// James Quinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <tuple>
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/blas_l1.hpp>

namespace sw { namespace universal { namespace blas {  

template<typename Scalar>
std::tuple<matrix<Scalar>, matrix<Scalar>, matrix<Scalar>> plu(const matrix<Scalar>& A){ 

    using Matrix = sw::universal::blas::matrix<Scalar>;
    using namespace std;

    Scalar x;
    size_t n = num_rows(A);
    Matrix P(n,n);
    Matrix L(n,n);
    Matrix U(n,n);

    P = 1;
    L = 1;
    U = A;

    // Elimination Process
    for (size_t i = 0; i < n-1; ++i){ // i-th row
        Scalar absmax = abs(U(i,i)); 
        size_t argmax = i;

        // Find largest element in ith column
        for (size_t k = i + 1; k < n; ++k){ // i = subsequent row (ele. in column k)
            if (abs(U(k,i)) > absmax){
                absmax = abs(U(k,i));
                argmax = k;
            }
        }
        // Check for necessary swaps
        if (argmax != i){
            // Swap rows loop
            for (size_t j = 0; j < n;++j){
                x = U(i,j);
                U(i,j) = U(argmax,j);
                U(argmax,j) = x;

                x = P(i,j);
                P(i,j) = P(argmax,j);
                P(argmax,j) = x;
            }
            
        }
        // Continue with row reduction
        for (size_t k = i + 1; k < n; ++k){  // objective row
            L(k,i) = U(k,i) / U(i,i);
            for (size_t j = i; j < n; ++j){
                U(k,j) = U(k,j) - L(k,i)*U(i,j);
            }
        }
    }
    U = triu(U);
    return std::make_tuple(P,L,U); 
} // LU

}}} // namespace sw::universal::blas