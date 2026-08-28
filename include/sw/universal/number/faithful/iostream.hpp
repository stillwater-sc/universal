#pragma once
// iostream.hpp: stream insertion and extraction for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the faithful headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half -- except for faithful, whose manipulators.hpp is commented out
// of the umbrella and does not compile (18 errors on main), so components() lives HERE
// to stay reachable rather than being stranded in a dead header.
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>   // components() builds its string here
#include <string>

#include <universal/number/faithful/core.hpp>

namespace sw { namespace universal {

template<typename FPType>
inline std::ostream& operator<<(std::ostream& ostr, const faithful<FPType>& v) {
	ostr << "( " << v.value << ", " << v.error << ')';
	return ostr;
}

template<typename FPType>
inline std::istream& operator>>(std::istream& istr, faithful<FPType>& v) {
	std::string token;
	istr >> token >> v.value >> token >> v.error >> token;
	return istr;
}

template<typename FloatingPointType>
inline std::string components(const faithful<FloatingPointType>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero";
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite";
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

}} // namespace sw::universal
