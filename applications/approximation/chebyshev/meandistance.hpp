#pragma once
// MEANDISTANCE(x) - Geometric mean distance among elements of x
// 
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <blas/blas.hpp>

namespace sw::universal{
	template<typename Scalar>
	blas::vector<Scalar> meandistance(blas::vector<Scalar> x)
	{
        size_t n = size(x);
        blas::vector<Scalar>z(n);
	
        for (size_t i = 0; i < n; ++i){
            Scalar y = 1;
            for (size_t j = 0; j < n; ++j) {
                if (i == j) {
                    continue;
                }
                y = y*(abs(x(i) - x(j)));
            }
            z(i) = sqrt(y);
        }
        return z;
	}
}