/**
 * @file matnorm.hpp
 * @author james quinlan
 * @brief Calculates ||A||_p for p = 1, 2, and inf.  
 * @version 0.1
 * @date 2022-12-13
 * 
 * @copyright Copyright (c) 2017-2021 Stillwater Supercomputing, Inc.
 * This file is part of the universal numbers project, released under an MIT Open Source license.
 */

#pragma once
#include <blas/blas.hpp>

template<typename Scalar>
Scalar matnorm(const sw::universal::blas::matrix<Scalar> & A, size_t p = 2){
    size_t m = num_rows(A);
    size_t n = num_cols(A);
    
    if (p == 1){
        // Col. max = 1-norm
        Scalar Cmax = 0;
        for (size_t j=0; j < n; ++j){
            Scalar N = 0;
            for (size_t i = 0; i < n; ++i){
                Scalar element = abs(A(i,j));
                N += element;
            }
            if (N >= Cmax){Cmax = N;}
        }
        return Cmax;
    } else{
        // Row max = Inf-norm
        Scalar Rmax = 0;
        for (size_t i=0; i < m; ++i){
            Scalar N = 0;
            for (size_t j = 0; j < n; ++j){
                Scalar element = abs(A(i,j));
                N += element;
            }
            if (N >= Rmax){Rmax = N;}
        }
        return Rmax;
    }

    
}