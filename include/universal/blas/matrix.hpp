#pragma once
// matrix.hpp: super-simple dense matrix implementation
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <vector>

namespace sw { namespace unum { namespace blas { 

template<typename Scalar>
class matrix {
public:
	typedef Scalar                            value_type;
	typedef const value_type&                 const_reference;
	typedef value_type&                       reference;
	typedef const value_type*                 const_pointer_type;

	matrix() {}
	matrix(unsigned _n, unsigned _m) : n{ _n }, m{ _m }, data(n*m, Scalar(0.0)) { }

	Scalar operator()(int i, int j) const { return data[i*m + j]; }
	Scalar& operator()(int i, int j) { return data[i*m + j]; }

	unsigned rows() const { return n; }
	unsigned cols() const { return m; }

private:
	unsigned n, m;
	std::vector<Scalar> data;
};

// ostream operator: no need to declare as friend as it only uses public interfaces
template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const matrix<Scalar>& A) {
	unsigned n = A.rows();
	unsigned m = A.cols();
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned j = 0; j < n; ++j) {
			ostr << A(i, j) << " ";
		}
		ostr << '\n';
	}
	return ostr;
}

}}} // namespace sw::unum::blas