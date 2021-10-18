// CHEBPOLY(n,kind) - returns the coefficients of Chebyshev poly of the kind.
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
    using namespace sw::universal;
	template<typename Scalar>
	blas::vector<Scalar> chebpoly(size_t n)
	{
		if(n<0){
			std::cerr << "Parameter must be a nonnegative integer. Provided n == " << n << '\n';
			return blas::vector<Scalar>(1);
		}
        
		blas::vector<Scalar>Tn(n+1);
		if (n==0) Tn(0) = 1;
        if (n==1) Tn(0) = 0;Tn(1) = 1;
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
