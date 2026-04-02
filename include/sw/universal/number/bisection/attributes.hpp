#pragma once
// attributes.hpp: value range and property queries for bisection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <sstream>
#include <iomanip>

namespace sw { namespace universal {

template<typename G, typename R, unsigned nbits, typename bt>
inline std::string bisection_range(const bisection<G, R, nbits, bt>&) {
	using Bisection = bisection<G, R, nbits, bt>;
	Bisection mn, mp;
	mn.maxneg();
	mp.maxpos();
	Bisection nn, np;
	nn.minneg();
	np.minpos();
	std::ostringstream ss;
	ss << "bisection<" << nbits << ">\n"
	   << "[ " << double(mn) << " ... " << double(nn) << "  0  "
	   << double(np) << " ... " << double(mp) << " ]";
	return ss.str();
}

}} // namespace sw::universal
