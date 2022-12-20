/** **********************************************************************
 * plu.hpp: dense matrix PLU decomposition (PA = LU)
 *          via DooLittle Method (in place)
 *
 * @author:     James Quinlan
 * @date:       2022-12-18
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license
 * 
 * This file is part of the Universal Number Library project.
 * ***********************************************************************
 */

#pragma once
#include<universal/utility/directives.hpp>
#include<universal/number/posit/posit_fwd.hpp>
#include<universal/blas/matrix.hpp>

namespace sw { namespace universal { namespace blas {  

template<typename Scalar>
void plu(matrix<Scalar>& A, matrix<size_t>& P, size_t n){ 
    Scalar x;
    for (size_t i = 0; i < n-1; ++i){ // i-th row
        P(i,0) = i;
        P(i,1) = i;

        Scalar absmax = abs(A(i,i)); 
        size_t argmax = i;

        // Select k >= i to maximize |U(k,i)| 
        for (size_t k = i + 1; k < n; ++k){
            if (abs(A(k,i)) > absmax){
                absmax = abs(A(k,i));
                argmax = k;
            }
        }

        // Check for necessary swaps
        if (argmax != i){
            P(i,1) = argmax;
            for (size_t j = 0; j < n;++j){  // j = i originally
                x = A(i,j);
                A(i,j) = A(argmax,j);
                A(argmax,j) = x;
            }
        }

        // Continue with row reduction
        for (size_t k = i + 1; k < n; ++k){  // objective row
            A(k,i) = A(k,i) / A(i,i);
            for (size_t j = i+1; j < n; ++j){
                A(k,j) = A(k,j) - A(k,i)*A(i,j);
            }
         } // update L
    }
} // LU
}}} // namespace sw::universal::blas