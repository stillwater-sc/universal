#pragma once
// iostream.hpp: stream insertion and extraction for dfixpnt
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the dfixpnt headers (#1334): the <iostream> half. Self-contained.
//
// Both operators were hidden friends defined in the class. They use only the public
// to_string() and assign(), both of which stay in the core, so they are free function
// templates here and need no friend declaration. This header includes only core.hpp.
#include <istream>
#include <ostream>
#include <string>

#include <universal/number/dfixpnt/core.hpp>

namespace sw { namespace universal {

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
inline std::ostream& operator<<(std::ostream& os, const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	return os << v.to_string();
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
inline std::istream& operator>>(std::istream& is, dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	std::string s;
	is >> s;
	v.assign(s);
	return is;
}

}} // namespace sw::universal
