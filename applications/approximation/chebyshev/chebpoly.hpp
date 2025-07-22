#pragma once
// CHEBPOLY(n,kind) - returns the coefficients of Chebyshev polynomial
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan

#include <blas/blas.hpp>

namespace chebyshev {
    using namespace sw::universal;

	template<typename Scalar>
	blas::vector<Scalar> chebpoly(size_t n)
	{     
		blas::vector<Scalar>Tn(n+1);
		if (n==0) Tn(0) = 1;
        if (n==1) { Tn(0) = 0; Tn(1) = 1; }
        if (n>1){
            blas::vector<Scalar> T0(n+1);
            blas::vector<Scalar> T1(n+1);
            blas::vector<Scalar> X(n+1);
            T0(0) = 1; T1(1) = 1;
            
            for(size_t i = 2;i < n+1; ++i){
                for(size_t j = n;j > 0; --j){
                    X(j) = T1(j - 1);
                }
                Tn = Scalar(2)*X - T0;
                T0 = T1;
                T1 = Tn;
                X=0;
            }
        }   
		return Tn;
	}
}
