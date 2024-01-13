// RSHIFT(n,offset) - right shift vector by offset
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// James Quinlan, 04/29/2021

#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
namespace sw::universal{
    using namespace sw::universal::blas;

	template<typename Scalar>
	vector<Scalar>rshift(blas::vector<Scalar> x)
	{
		blas::vector<Scalar>y(size(x));
        for(size_t i = n;i > 0; --i){
            y(i) = x(i - 1);
        }
		return y;
	}
}