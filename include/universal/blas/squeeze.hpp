// Squeeze.hpp: Squeeze elements of a matrix
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Authors: James Quinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#pragma once
#include <universal/blas/blas.hpp>


 

// Matrix Squeeze
template<typename Scalar>
void squeeze(sw::universal::blas::matrix<Scalar>& A, size_t algo) {

    //size_t m = num_rows(A);
    // double dmaxpos = double(maxpos);
    // double dmaxneg = double(maxneg);

    if (algo == 1){ // Algo 2.1
        std::cout << A(0,0) << std::endl;
        A(1,1) = 4;
    }

}


