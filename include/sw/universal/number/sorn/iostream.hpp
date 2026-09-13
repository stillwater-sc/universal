#pragma once
// iostream.hpp: stream insertion for sorn
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the sorn headers (#1334, Phase 2 group 5c, #1467): the <iostream> half.
// Self-contained.
//
// There are TWO insertion operators, and which one a call picks is part of sorn's
// behaviour: a NON-const sorn prints its current interval, a const sorn prints the
// range of its lattice. The non-const one used to be a hidden friend defined in the
// class; it is a free function template here, which keeps the choice: both are
// templates now, and for a non-const lvalue the sorn& overload binds with fewer added
// cv-qualifications, so it still wins. A const sorn can only bind the const overload.
//
// The dependency runs iostream -> manipulators: the non-const operator prints through
// sornInterval::getInt(), which manipulators.hpp defines.
#include <ostream>

#include <universal/number/sorn/core.hpp>
#include <universal/number/sorn/manipulators.hpp>   // sornInterval::getInt()

namespace sw { namespace universal {

// non-const sorn: its current interval
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
inline std::ostream& operator<<(std::ostream& ostr, sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& s) {
	return ostr << s.sornIntVal.getInt();
}

// const sorn: the range of its lattice
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
inline std::ostream& operator<<(std::ostream& ostr, const sorn< _start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& lhs) { 
	return ostr << "[ " << lhs.minVal() << ", " << lhs.maxVal() << "]";
}

}} // namespace sw::universal
