#pragma once
// iostream.hpp: stream output for unum2 and its lattice
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Layer 2b of the unum2 headers (#1334, Phase 2 group 5c, #1467): the <iostream> half.
// Self-contained.
//
// The dependency runs iostream -> manipulators: operator<< renders each bound with
// lattice::get_exact(), which manipulators.hpp defines. manipulators.hpp does not include
// this header back, so there is no cycle for #pragma once to mask (#1427).
#include <cstdint>
#include <iostream>   // std::ostream, std::cout, std::endl
#include <sstream>

#include <universal/number/unum2/core.hpp>
#include <universal/number/unum2/manipulators.hpp>   // lattice::get_exact()

namespace sw { namespace universal {

// write the lattice to std::cout
template<int... exacts>
void lattice<exacts...>::print() const {
    std::cout << "inf <-->";

    int size = _exacts.size();

    for(int i = size - 1; ~i; i--) 
        std::cout << " -" << _exacts[i] << " <-->";
    for(int i = 1; i < size; i++)
        std::cout << " -/" << _exacts[i] << " <-->";

    std::cout << " 0 <-->";

    for(int i = size - 1; i; i--) 
        std::cout << " /" << _exacts[i] << " <-->";
    for(int i = 0; i < size; i++)
        std::cout << " " << _exacts[i] << " <-->";
    
    std::cout << " inf" << std::endl;
}

// stream insertion for unum2: the SORN as a union of intervals over the lattice
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const unum2<T>& u) {
    std::ostringstream oss;

    uint64_t left_bound = 0;
    bool has_left_bound = false;
    bool bound = false;  // Series of continuous 1s in the SORN bitset
    bool written = false;  // Has something been written to sstream?
    for(uint64_t i = 0; i < u.sorn_length; i++) {
        if(u._sorn[i] == 1) {
            // If already bound, continue
            if(bound) continue;

            // Set left bound
            bound = true;
            has_left_bound = true;
            left_bound = i;

            // If something has been written, that means there were other bounds. Add Union sign.
            if(written)
                oss << " U ";
        } else {
            // Single bit bound. Can be exact or inexact.
            if(bound) {
                // End bound
                bound = false;
                written = true;

                if(static_cast<int64_t>(left_bound) == static_cast<int64_t>(i - 1)) {
                    // If inexact
                    if(left_bound & 0x01) {  // Check ubit
                        oss << "(" << u._lattice.get_exact(u._conv_idx(left_bound - 1))
                            << ", " << u._lattice.get_exact(u._conv_idx(left_bound + 1))
                            << ")";
                    } else {
                        if(left_bound == 0) { 
                            if(u._sorn[u._lattice._N - 1] != 1) 
                                oss << "inf";
                            else written = false;
                        } else oss << u._lattice.get_exact(u._conv_idx(left_bound));
                    }
                } else {  // Multiple bit bound
                          // Check if left_bound is inexact. If so, get previous exact.
                    if(left_bound & 0x01) {
                        left_bound--;
                        oss << "(";
                    } else oss << "[";

                    int64_t right_bound;
                    char brace = ']';
                    if((i - 1) & 0x01) {
                        right_bound = i;
                        brace = ')';
                    } else right_bound = i - 1;

                    oss << u._lattice.get_exact(u._conv_idx(left_bound)) << ", "
                        << u._lattice.get_exact(u._conv_idx(right_bound)) << brace;
                }
            }
        }
    }

    // No bounds.
    if(!has_left_bound)
        oss << "[EMPTY]";
    else if(bound == true && left_bound == 0)
        oss << "[EVERYTHING]";

    // Bit equal 0 code over again.
    else if(bound) {
        // Final bit should be 1 if there is a bound.
        uint64_t i = u.sorn_length - 1;

        if(left_bound == i) {
            // Final bit index in SORN is always inexact
            oss << "(" << u._lattice.get_exact(u._conv_idx(i - 1));

            // If only the first SORN bit is set, that infers infinity is included.
            if(u._sorn[0] == 1)
                oss << ", inf]";
            else oss << ", " << u._lattice.get_exact(u._conv_idx(i + 1)) << ")";
        } else {  // Multiple bit bound
            if(left_bound & 0x01) {
                left_bound--;
                oss << "(";
            } else oss << "[";

            int64_t right_bound;
            char brace = ')';
            if(u._sorn[0] == 1) { 
                right_bound = 0;
                brace = ']';
            } else right_bound = i + 1;

            oss << u._lattice.get_exact(u._conv_idx(left_bound)) << ", "
                << u._lattice.get_exact(u._conv_idx(right_bound)) << brace;
        }
    }

    return (os << oss.str());
}

}} // namespace sw::universal
