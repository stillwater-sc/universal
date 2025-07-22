#pragma once
// minij.hpp: uniform random matrix generator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric/containers.hpp>

namespace sw { namespace blas {
    using namespace sw::numeric::containers;

    // minij returns the n-by-n symmetric positive definite matrix with entries A(i,j) = min(i+1,j+1) as C++ is zero indexed
    template<typename Scalar>
    matrix<Scalar>& minij(matrix<Scalar>& A) {
        for (unsigned i = 0; i < num_rows(A); ++i) {
            for (unsigned j = 0; j < num_cols(A); ++j) {
                A(i, j) = Scalar(std::min(i+1, j+1));
            }
        }
        return A;
    }

    // MATLAB-style minij returns the n-by-n symmetric positive definite matrix with entries A(i,j) = min(i,j) with i,j range 1..N
    template<typename Scalar>
    matrix<Scalar> minij(unsigned N) {
        matrix<Scalar> A(N, N);
        return minij(A);
    }

}} // namespace sw::blas