#pragma once
// trigonometry.hpp: vectorized trigonometry functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <blas/blas.hpp>
#include <blas/generators/gaussian_random.hpp>

namespace sw { namespace blas {  
	using namespace sw::numeric::containers;

    template<typename Scalar>
    std::tuple<matrix<Scalar>,matrix<Scalar>, matrix<Scalar>> randsvd(const matrix<Scalar>& A) {
        size_t k = std::min(num_cols(A), num_rows(A));
        size_t n = num_cols(A), m = num_rows(A);                
        matrix<Scalar> omega(n, k),Y(m, k), B(k, n);
        double mean = 1.0;
        double stddev = 0.5;
        gaussian_random(omega, mean, stddev);
        Y = A * omega;
        matrix<Scalar> Q(n, k), R(n, n);
        std::tie(Q, R) = solvers::qr(Y);
        Q.transpose();
        B = Q * A;
        matrix<Scalar> S(n, k), V(n, n), D(n, n);
        std::tie(S, V, D) = solvers::svd(B,k);
        return std::make_tuple(S, V, D);
    }

}} // namespace sw::blas