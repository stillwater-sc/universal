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
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/blas_l1.hpp>

namespace sw { namespace universal { namespace blas {  

template<typename Scalar>
void luq(matrix<Scalar>& A){ 

    unsigned n = num_rows(A);
 
    // Gaussian Elimination Process
    for (size_t i = 0; i < n-1; ++i){ // i-th row
        for (size_t k = i + 1; k < n; ++k){  // objective row
            A(k,i) =  A(k,i) / A(i,i);
            for (size_t j = i+1; j < n; ++j){
                A(k,j) -= A(k,i)*A(i,j);
            }
        }
    }
} // LU

}}} // namespace sw::universal::blas