#pragma once
// vector.hpp: super-simple vector class
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <vector>
#include <initializer_list>

namespace sw { namespace unum { namespace blas {

template<typename Scalar>
class vector {
public:
	typedef Scalar                            value_type;
	typedef const value_type&                 const_reference;
	typedef value_type&                       reference;
	typedef const value_type*                 const_pointer_type;

	vector() : data(0) {}
	vector(size_t N) : data(N) {}
	vector(size_t N, const Scalar& val) : data(N, val) {}
	vector(std::initializer_list<Scalar> iList) : data(iList) {}

	vector& operator=(const Scalar& val) {
		for (auto& v : data) v = val;
		return *this;
	}
	vector& assign(const Scalar& val) {
		for (auto& v : data) v = val;
		return *this;
	}

	size_t size() const { return data.size(); }

	value_type operator[](size_t index) const { return data[index]; }

private:
	std::vector<Scalar> data;
};

template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const vector<Scalar>& v) {
	auto width = ostr.precision() + 2;
	for (size_t j = 0; j < size(v); ++j) ostr << std::setw(width) << v[j] << " ";
	return ostr;
}

template<typename Scalar> auto size(const vector<Scalar>& v) { return v.size(); }

}}}  // namespace sw::unum::blas
