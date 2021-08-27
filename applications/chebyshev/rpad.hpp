// RPAD(x,k) - Pad the right of a vector x with k zeros
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan
// RPAD will be used when calling chebypoly

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

namespace sw::universal{

	template<typename Scalar>
	blas::vector<Scalar> rpad(blas::vector<Scalar> x, size_t k)
	{
        size_t n = size(x);
        blas::vector<Scalar>y(n+k);
		for(size_t i = 0; i < n; ++i){
			y(i) = x(i); 
		}
		return y;
	}
}