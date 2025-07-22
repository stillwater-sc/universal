#pragma once
// CHEBMAT(n) - returns n x n Chebyshev change of basis matrix
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan
// Modified: 2021-10-15

#pragma once

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <numeric/containers/matrix.hpp>

namespace chebyshev {

	template<typename Scalar>
	sw::universal::blas::matrix<Scalar> chebmat(size_t n)
	{
		if (n < 1) return blas::matrix<Scalar>{};
		Scalar one(1.0f);
		sw::universal::blas::matrix<Scalar> T(n+1, n+1);
		T = one;
        for(size_t i = 1;i < n+1; ++i){
                for(size_t j = 2;j < i; ++j){
                    T(i,j) = 1;
                }
        }   
		return T;
	}
}
// End Notes
/*
In practice, FFT is used to determine the coefficients a_0, a_1, ...
*/