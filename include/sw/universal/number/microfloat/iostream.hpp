#pragma once
// iostream.hpp: stream insertion and extraction for microfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the microfloat headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half. This header includes only core.hpp -- the operators convert to
// a native float and stream that, so they need nothing from the text layer.
#include <iostream>
#include <istream>
#include <ostream>

#include <universal/number/microfloat/core.hpp>

namespace sw { namespace universal {

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline std::ostream& operator<<(std::ostream& ostr, microfloat<n,e,i,na,s> mf) {
	return ostr << float(mf);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline std::istream& operator>>(std::istream& istr, microfloat<n,e,i,na,s>& p) {
	float f;
	istr >> f;
	p = microfloat<n,e,i,na,s>(f);
	return istr;
}

}} // namespace sw::universal
