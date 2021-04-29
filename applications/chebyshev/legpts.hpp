// LEGPTS(n) - returns the n Legendre point.
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// James Quinlan, 04/29/2021

#include <universal/number/posit/posit>
#include <universal/blas/blas>

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
namespace sw::universal{
    using namespace sw::universal::blas;

	template<typename Scalar>
	vector<Scalar>legpts(int n)
	{
		blas::vector<Scalar>x(n);
        
		return x;
	}
}