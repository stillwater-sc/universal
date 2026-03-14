#pragma once
//
// forwsub.hpp: Forward substitution to solve Ax = b using fused dot product
// Input : Matrix A, Vector b, bool lower
// Inplace forward sub.Uses only lower tri.
//
// @author:     James Quinlan
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// @date:       2022-12-17
// @modified:   2023-10-10
//
#include <numeric/containers.hpp>
// Consumer must include the appropriate fdp.hpp header for their Scalar type before this header

namespace sw { namespace blas {
	using namespace sw::numeric::containers;
	using namespace sw::universal;

    template<typename Scalar, unsigned capacity = 10>
    vector<Scalar> forwsub(const matrix<Scalar>& A, const vector<Scalar>& b, bool lower = false) {
        using Vector = vector<Scalar>;
        using Quire  = quire<Scalar, capacity>;
        size_t n = size(b);

        Vector x(n);
        Vector d(n,1);

        if (lower){d = diag(A);}

        x(0) = b(0)/d(0);
	    for (size_t i = 1; i < n; ++i){
            Quire q{0};
            for (size_t j = 0; j < i; ++j){
                q += quire_mul(A(i,j), x(j));
            }
            Scalar y = quire_resolve(q);  // one and only rounding step of the fused-dot product
            x(i)     = (lower) ? (b(i) - y)/d(i) : (b(i) - y);
        }
	    return x;
    }

}} // namespace sw::blas
