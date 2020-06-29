#pragma once
// lu.hpp: super-simple dense matrix implementation
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/blas/matrix.hpp>

namespace sw { namespace unum { namespace blas {

	struct no_pivoting {};
	struct partial_pivoting {};
	struct full_pivoting {};

template<typename Matrix, typename PivotingStrategy>
class LU {
public:
	typedef typename Matrix::value_type value_type;

	void compute(const Matrix& A) {

	}
	vector<value_type> solve(const vector<value_type>& b) {
		vector<value_type> x(n);
		// ...
		return x;
	}

private:
	size_t m, n; // rows, cols
};

// partial pivoting Gaussian Elimination


} } }  // namespace sw::unum::blas