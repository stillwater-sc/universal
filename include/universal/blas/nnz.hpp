// nnz.hpp: Number of Nonzero elements
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// @jquinlan
//
// This file is part of the universal numbers project, released under an MIT Open Source license.

#pragma once
#include <universal/blas/blas.hpp>

template<typename Scalar>
size_t nnz(const sw::universal::blas::matrix<Scalar> & A){
    size_t m = num_rows(A);
    size_t n = num_cols(A);
    size_t NNZ = 0;

    for (size_t i=0; i < m; ++i){
            for (size_t j = 0; j < n; ++j){
                Scalar element = A(i,j);
                if (element != 0){
                    NNZ += 1;
                }
            }
        }
    return NNZ;
}