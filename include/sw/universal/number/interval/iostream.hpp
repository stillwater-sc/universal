#pragma once
// iostream.hpp: stream insertion and extraction for interval
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the interval headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half; this header includes only core.hpp.
#include <iostream>
#include <istream>
#include <ostream>

#include <universal/number/interval/core.hpp>

namespace sw { namespace universal {

// stream output
template<typename Scalar>
inline std::ostream& operator<<(std::ostream& ostr, const interval<Scalar>& v) {
	return ostr << '[' << v._lo << ", " << v._hi << ']';
}

// stream input
template<typename Scalar>
inline std::istream& operator>>(std::istream& istr, interval<Scalar>& v) {
	Scalar lo, hi;
	char c;
	istr >> c >> lo >> c >> hi >> c;  // expects format [lo, hi]
	v._lo = lo;
	v._hi = hi;
	return istr;
}

}} // namespace sw::universal
