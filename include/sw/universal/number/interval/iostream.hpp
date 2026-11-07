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

// stream input: [lo, hi], the format operator<< writes. A malformed token, a wrong delimiter,
// or lo > hi sets failbit and leaves v unchanged; it used to overwrite v whatever was read
// (#1971).
template<typename Scalar>
inline std::istream& operator>>(std::istream& istr, interval<Scalar>& v) {
	Scalar lo{}, hi{};
	char open = 0, comma = 0, close = 0;
	istr >> open;
	if (istr && open == '[') istr >> lo;
	if (istr) istr >> comma;
	if (istr && comma == ',') istr >> hi;
	if (istr) istr >> close;
	if (!istr || open != '[' || comma != ',' || close != ']' || hi < lo) {
		istr.setstate(std::ios_base::failbit);
		return istr;
	}
	v._lo = lo;
	v._hi = hi;
	return istr;
}

}} // namespace sw::universal
