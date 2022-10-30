// matnorm.hpp: matrix norm
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// @jquinlan
//
// This file is part of the universal numbers project, released under an MIT Open Source license.

#pragma once
#include <universal/blas/blas.hpp>

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
        // std::cout << "Cmax = " << Cmax << std::endl;
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
        // std::cout << "Rmax = " << Rmax << std::endl;
        return Rmax;
    }

    
}