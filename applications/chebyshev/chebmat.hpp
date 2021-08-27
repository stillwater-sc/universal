// CHEBMAT(n) - returns n x n Chebyshev change of basis matrix
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan

#pragma once

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

namespace chebyshev {
	using namespace sw::universal;
	template<typename Scalar>
	blas::matrix<Scalar> chebmat(size_t n)
	{
		if (n < 1) return matrix<Scalar>{};
        
        for(size_t i = 1;i < n+1; ++i){
                for(size_t j = 2;j < i; ++j){
                    T(i,j) = 
                }
        }   
		return T;
	}
}
