// CHEBMAT(n) - returns n x n Chebyshev change of basis matrix
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan
// Modified: 2021-10-17

#pragma once

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

namespace chebyshev {

	template<typename Scalar>
	sw::universal::blas::matrix<Scalar> chebmat(size_t n)
	{
		if (n < 1) return blas::matrix<Scalar>{};
		Scalar one(1.0f);
		sw::universal::blas::matrix<Scalar> T(n, n);
		T = one;
		if (n > 1){
			T(1,1) = 1;
			for(size_t i = 2;i<n;++i){
				T(i,0) = -T(i-2,0);
			}
		}
        for(size_t i = 2;i < n; ++i){
                for(size_t j = 1;j < i+1; ++j){
                    T(i,j) = 2*T(i-1,j-1) - T(i-2,j);
                }
        }   
		return T;
	}
}
// Note:
/*
In practice, FFT is used to determine the coefficients a_0, a_1, ...
*/