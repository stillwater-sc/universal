#pragma once
// iostream.hpp: stream insertion and extraction for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

namespace sw { namespace universal {

// prints the value, through long double; NaR prints as "nar"
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
std::ostream& operator<<(std::ostream& ostr, const bposit<nbits, rs, es, bt>& v) {
	if (v.isnar()) return ostr << "nar";
	return ostr << static_cast<long double>(v);
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
std::istream& operator>>(std::istream& istr, bposit<nbits, rs, es, bt>& v) {
	std::string txt;
	istr >> txt;
	if (txt == "nar" || txt == "NaR") { v.setnar(); return istr; }
	try { v = std::stold(txt); }
	catch (...) { istr.setstate(std::ios::failbit); }
	return istr;
}

}} // namespace sw::universal
