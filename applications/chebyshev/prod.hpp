// PROD(x) - Product of elements in x
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan

#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
namespace sw::universal{

	template<typename Scalar>
	Scalar prod(blas::vector<Scalar> x)
	{
        Scalar y = 1;
		for(int k = 0; k < size(x); ++k){
			y = y*x(k); 
		}
		return y;
	}
}