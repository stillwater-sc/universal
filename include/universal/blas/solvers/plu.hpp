#pragma once
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

#include<universal/blas/matrix.hpp>

namespace sw { namespace universal { namespace blas {  

    /// <summary>
    ///  dense matrix LU with partial pivoting (PA = LU) decomposition via DooLittle Method (in place)
    /// </summary>
    /// <typeparam name="Scalar"></typeparam>
    /// <param name="A">dense matrix to factor</param>
    /// <param name="P">associated permutation matrix</param>
    template<typename Scalar>
    void plu(matrix<Scalar>& A, vector<size_t>& P){ 
        Scalar x;
        size_t n = num_rows(A);
        for (size_t i = 0; i < n; ++i){ // i-th row
            P(i) = i;

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
                P(i) = argmax;
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
    }

}}} // namespace sw::universal::blas
