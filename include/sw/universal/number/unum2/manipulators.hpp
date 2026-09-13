#pragma once
// manipulators.hpp: text for the unum2 lattice
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Layer 2a of the unum2 headers (#1334, Phase 2 group 5c, #1467): the <iomanip> half --
// what turns a lattice point into a std::string. iostream.hpp is the <iostream> half.
// Self-contained.
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

#include <universal/number/unum2/core.hpp>

namespace sw { namespace universal {

// the exact value at lattice index i as text ("" for an inexact index)
template<int... exacts>
std::string lattice<exacts...>::get_exact(uint64_t i) const {
    if(i >= _N) 
        throw std::out_of_range("Lattice index out of range");

    // Return nothing if not exact.
    if(i & 0x01) 
        return "";

    // Check for infinity, zero, -1 or 1.
    if(i == _N_half) 
        return "inf";
    else if(i == 0)
        return "0";
    else if(i == _N_quarter)
        return "1";
    else if(i == 3 * _N_quarter)
        return "-1";
    
    std::ostringstream oss;

    // Negative
    if(i > _N_half) {
        oss << '-';
        i = _horizontal_invert(i, _MASK);
    }

    // Vertical invert
    if(i >= _N_quarter) 
        oss << _exacts[(i - _N_quarter) >> 1];
    else oss << '/' << _exacts[_exacts.size() - (i >> 1)];

    return oss.str();
}

}} // namespace sw::universal
