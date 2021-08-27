// LINSCALE(x) - Scales and shifts vector x to interval [c,d]
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan

#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

namespace sw::universal{
	template<typename Scalar>
	blas::vector<Scalar> linscale(blas::vector<Scalar> x, Scalar c = -1, Scalar d = 1)
	{
        size_t n = size(x);
        Scalar a = x(0);
        Scalar b = x(n-1);
        blas::vector<Scalar>y(n);
        Scalar m = (d - c)/(b - a);
        for (size_t i = 0; i < n; ++i) {
                y(i) = m*(x(i) - a) + c;
        }
        return y;
	}
}