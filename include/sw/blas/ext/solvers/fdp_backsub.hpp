#pragma once
//
// backsub.hpp: Backsubstitution to solve Ax = b given A = upper triangular
// using fused dot product
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
    vector<Scalar> backsub(const matrix<Scalar>& A, const vector<Scalar>& b) {
        using Vector = vector<Scalar>;
        using Quire  = quire<Scalar, capacity>;
	    unsigned n = static_cast<unsigned>(size(b));

        Vector x(n);
	    for (unsigned ii = 0; ii < n; ++ii) {
            unsigned i = n - 1 - ii;
            Quire q{0};
            for (unsigned j = i; j < n; ++j) {
                q += quire_mul(A(i,j), x(j));
            }
            Scalar y = quire_resolve(q);
            x(i) = (b(i) - y)/A(i,i);
        }
	    return x;
    }

}} // namespace sw::blas
