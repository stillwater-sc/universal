#pragma once
// iostream.hpp: stream insertion and extraction for rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the rational headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half.
#include <iostream>
#include <istream>
#include <ostream>

#include <universal/number/rational/core.hpp>
#include <universal/number/rational/manipulators.hpp>

namespace sw { namespace universal {

template<unsigned nnbits, typename nBase, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const rational<nnbits,nBase,nbt>& v) {
	return ostr << double(v);
}

template<unsigned nnbits, typename nBase, typename nbt>
inline std::istream& operator>>(std::istream& istr, const rational<nnbits,nBase,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

}} // namespace sw::universal
