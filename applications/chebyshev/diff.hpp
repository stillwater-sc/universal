// DIFF(x,y) - Difference of elements in x and y
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>
#include <universal/blas/blas>

namespace sw::universal{

	template<typename Scalar>
	blas::vector<Scalar> diff(blas::vector<Scalar> x, blas::vector<Scalar> y)
	{
        size_t n = size(x);

        if (size(x)!=size(y)){
            std::cout << "Unequal vector dimensions" << std::endl;
            return blas::vector<Scalar>(0);
        }
        blas::vector<Scalar>z(n);
		for(size_t k = 0; k < n; ++k){
			z(k) = x(k) - y(k); 
		}
		return z;
	}
}