// Universal Number 2.0 implementation including SORN (Sets of Real Numbers).
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once

#include <sw/universal/number/unum2/common.hpp>

#include <iostream>
#include <sstream>
#include <cstdint>
#include <bitset>
#include <algorithm>
#include <cmath>

namespace sw { namespace universal {

template <typename S, typename T>
class unum2 {
private:
    T _lattice;

    // SORN
    uint64_t _sorn_length;
    std::bitset<1 << (sizeof(S) * 8)> _sorn;  // empty

public:
    unum2(S index) : _lattice(T()), _sorn_length(_lattice._N) {
        if(_lattice._N > (1 << (sizeof(S) * 8))) 
            throw std::invalid_argument("Lattice point count overflows given storage type!");

        // Do bitwise or with with half the lattice size. SORN index starts from 2's compliment. That is
        // the first bit in SORN represents 'inf' instead of '0'. Next SORN bit represents (inf, -en) and
        // so on so forth.
        _sorn = std::bitset<1 << (sizeof(S) * 8)>().set((index ^ _lattice._N_half) & _lattice._MASK);
    }

    friend std::ostream& operator << (std::ostream& os, const unum2<S, T>& u) {
        std::ostringstream oss;

        int64_t left_bound = -1;
        bool bound = false;  // Series of continuous 1s in the SORN bitset
        bool written = false;  // Has something been written to sstream?
        for(S i = 0; i < u._sorn_length; i++) {
            if(u._sorn[i] == 1) {
                // If already bound, continue
                if(bound) continue;
                
                // Set left bound
                bound = true;
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
            
                    if(left_bound == i - 1) {
                        // If inexact
                        if(left_bound & 0x01) {  // Check ubit
                            oss << "(" << u._lattice.get_exact(((left_bound - 1) ^ u._lattice._N_half) & u._lattice._MASK)
                                << ", " << u._lattice.get_exact(((left_bound + 1) ^ u._lattice._N_half) & u._lattice._MASK)
                                << ")";
                        } else {
                            if(left_bound == 0) { 
                                if(u._sorn[u._lattice._N - 1] != 1) 
                                    oss << "inf";
                                else written = false;
                            } else oss << u._lattice.get_exact((left_bound ^ u._lattice._N_half) & u._lattice._MASK);
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
                    
                        oss << u._lattice.get_exact((left_bound ^ u._lattice._N_half) & u._lattice._MASK) << ", "
                            << u._lattice.get_exact((right_bound ^ u._lattice._N_half) & u._lattice._MASK) << brace;
                    }
                }
            }
        }

        // No bounds.
        if(left_bound == -1) 
            oss << "[EMPTY]";
        else if(bound == true && left_bound == 0)
            oss << "[EVERYTHING]";
        
        // Bit equal 0 code over again.
        else if(bound) {
            // Final bit should be 1 if there is a bound.
            int64_t i = u._sorn_length - 1;
    
            if(left_bound == i) {
                // Final bit index in SORN is always inexact
                oss << "(" << u._lattice.get_exact(((i - 1) ^ u._lattice._N_half) & u._lattice._MASK);

                // If only the first SORN bit is set, that infers infinity is included.
                if(u._sorn[0] == 1)
                    oss << ", inf]";
                else oss << ", " << u._lattice.get_exact(((i + 1) ^ u._lattice._N_half) & u._lattice._MASK) << ")";
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
            
                oss << u._lattice.get_exact((left_bound ^ u._lattice._N_half) & u._lattice._MASK) << ", "
                    << u._lattice.get_exact((right_bound ^ u._lattice._N_half) & u._lattice._MASK) << brace;
            }
        }

        return (os << oss.str());
    }

    static unum2<S, T> empty() {
        unum2<S, T> res(0);
        res._sorn.reset();

        return res;
    }

    static unum2<S, T> everything() {
        unum2<S, T> res(0);
        res._sorn.set();

        return res;
    }

    template<typename TT>
    static unum2<S, T> from(TT value) {
        return unum2<S, T>(_from_index(value));
    }

    template<typename TT>
    static unum2<S, T> interval(TT a, TT b) {
        if(a == b) 
            return unum2<S, T>::from(a);
        
        S ai = unum2<S, T>::_from_index(a);
        S bi = unum2<S, T>::_from_index(b);
        
        if(a < b) return unum2<S, T>::_bound(ai, bi);
        return unum2<S, T>::_bound_inverse(ai, bi);
    }

    // Addition
    unum2<S, T> operator + (unum2<S, T>& other) {
        auto res = unum2<S, T>::empty();
        for(S i = 0; i < _sorn_length; i++) {
            for(S j = 0; j < _sorn_length; j++) {
                if(_sorn[i] == 1 && other._sorn[j] == 1)
                    res._sorn |= unum2<S, T>::_sumpoint(_conv_idx(i), _conv_idx(j), _lattice)._sorn;
            }
        }

        return res;
    }

    // Multiplication
    unum2<S, T> operator * (unum2<S, T>& other) {
        auto res = unum2<S, T>::empty();
        for(S i = 0; i < _sorn_length; i++) {
            for(S j = 0; j < _sorn_length; j++) {
                if(_sorn[i] == 1 && other._sorn[j] == 1)
                    res._sorn |= unum2<S, T>::_mulpoint(_conv_idx(i), _conv_idx(j), _lattice)._sorn;
            }
        }

        return res;
    }

    // Negation
    unum2<S, T> operator - () {
        auto res = unum2<S, T>::empty();
        for(uint64_t i = 0; i < _sorn_length; i++) {
            if(_sorn[i] == 1) {
                // Horizontal invert
                uint64_t neg_idx = _horizontal_invert(_conv_idx(i), _lattice._MASK);
                res._sorn.set(_conv_idx(neg_idx));
            }
        }

        return res;
    }

    // Subtraction
    unum2<S, T> operator - (unum2<S, T>& other) {
        auto neg = -other;
        return this->unum2<S, T>::operator + (neg);
    }

    // Invert
    unum2<S, T> operator ~ () {
        uint64_t msb_mask = _lattice._N >> 1;
        auto res = unum2<S, T>::empty();
        for(uint64_t i = 0; i < _sorn_length; i++) {
            if(_sorn[i] == 1) {
                // Vertical invert
                uint64_t ix = _conv_idx(i);
                uint64_t msb_set = ix & msb_mask;
                uint64_t inverted_idx = ~ix & _lattice._MASK;

                if(!msb_set) inverted_idx = (inverted_idx & msb_set) | (inverted_idx & (msb_mask - 1));
                else inverted_idx |= msb_set;
                inverted_idx = (inverted_idx + 1) & _lattice._MASK;

                res._sorn.set(_conv_idx(inverted_idx));
            }
        }

        return res;
    }

    unum2<S, T> operator / (unum2<S, T>& other) {
        auto inv = ~other;
        return this->unum2<S, T>::operator * (inv);
    }

    bool operator == (unum2<S, T>& other) {
        return this->_sorn == other._sorn;
    }

    // Raise to a power
    unum2<S, T> operator ^ (double n) {
        auto res = unum2<S, T>::empty();
        for(S i = 0; i < _sorn_length; i++) {
            if(_sorn[i] == 1)
                res._sorn |= unum2<S, T>::_powpoint(_conv_idx(i), n, _lattice)._sorn;
        }

        return res;
    }

    // Absolute
    unum2<S, T> abs() {
        auto res = unum2<S, T>::empty();
        for(int i = 0; i < _sorn_length; i++) {
            if(_sorn[i] == 1) {
                uint64_t idx = _conv_idx(i);

                // Negative point
                if(idx > _lattice._N_half)
                    // Horizontal invert
                    res._sorn.set(_conv_idx(_horizontal_invert(idx, _lattice._MASK)));
                else res._sorn.set(i);
            }
        }

        return res;
    }

private:
    S _conv_idx(S idx) {
        // In Unum2 SORN, 0 starts from _N_half, instead of 0 for compatibility
        // reasons. This function converts adjusted index (e.g. 15 -> 0) to
        // absolute index (e.g. 0 -> 0) and vice versa.
        return (idx ^ _lattice._N_half) & _lattice._MASK;
    }

    template<typename TT>
    static S _from_index(TT value) {
        T lattice = T();
    
        if(!std::isfinite(value))
            return lattice._N >> 1;
        else if(value == 0)
            return 0;
        else if(value == 1)
            return lattice._N_quarter;
        else if(value == -1)
            return lattice._N_quarter * 3;

        TT absolute_value = std::abs(value);
        size_t exact_size = lattice._exacts.size();

        // Try exact values.
        int64_t index = -1;
        for(int64_t i = 1; i < lattice._exacts.size(); i++) {
            int e = lattice._exacts[i];
            
            if(absolute_value == static_cast<TT>(e)) {
                // Vertical flip
                index = (i << 1) + lattice._N_quarter;
                break;
            }
            else if(absolute_value == (1 / static_cast<TT>(e))) {
                index = lattice._N_quarter - (i << 1);
                break;
            }

            // No exacts :(
            // Compare bounds.
            if(absolute_value > static_cast<TT>(lattice._exacts[i - 1]) && 
            absolute_value < static_cast<TT>(e))
            {
                // Vertical flip.
                index = (i << 1) + lattice._N_quarter - 1;
                break;
            } else {
                TT reciprocal_right = 1 / static_cast<TT>(lattice._exacts[exact_size - i - 1]);
                TT reciprocal_left = 1 / static_cast<TT>(lattice._exacts[exact_size - i]);

                if(absolute_value > reciprocal_left && absolute_value < reciprocal_right) {
                    index = (i << 1) + 1;
                    break;
                }
            }
        }

        if(index == -1) {
            // Check (0, e1) or (en, inf) bounds.
            if(absolute_value > 0 && absolute_value < (1 / static_cast<TT>(lattice._exacts[exact_size - 1])))
                index = 1;
            else if(absolute_value > static_cast<TT>(lattice._exacts[exact_size - 1]))
                index = lattice._N_half - 1;
        }
        
        // Horizontal flip
        if(value < 0)
            index = ((~index & lattice._MASK) + 1) & lattice._MASK;
        
        return index & lattice._MASK;
    }

    static unum2<S, T> _bound(S a, S b) {
        // Given unum has to be points.
        // and a < b
        // Note: Unum 'a' is operated on, thus changes.
        
        if(a == b) 
            return a;

        auto au = unum2<S, T>(a);
        auto bu = unum2<S, T>(b);
        auto res = au._sorn;
        auto criterion = res;

        // If b is infinity
        if(b == au._lattice._N_half) {
            int shift_count = au._lattice._N - au._sorn._Find_first();
            while(shift_count--) {
                criterion <<= 1;
                res |= criterion;
            }

            res |= bu._sorn;
        } else {
            while(criterion != bu._sorn) {
                criterion <<= 1;
                res |= criterion;
            }
        }

        au._sorn = res;
        return au;
    }

    static unum2<S, T> _bound_inverse(S a, S b) {
        // When bound a > bound b
        auto res = unum2<S, T>::_bound(b, a);
        res._sorn = res._sorn ^ std::bitset<1 << (sizeof(S) * 8)>().set();
        // Lattice should move coutner-clockwise in this case, but including the bounded
        // unums.
        res._sorn.set(a ^ res._lattice._N_half).set(b ^ res._lattice._N_half);
        return res;
    }

    static unum2<S, T> _sumpoint(S i, S j, T lattice) {
        // i and j both represent infinity
        if(i == lattice._N_half && j == lattice._N_half) 
            return unum2<S, T>::everything();
        // i or j represent infinity
        else if(i == lattice._N_half || j == lattice._N_half)
            return unum2<S, T>(lattice._N_half);  // inf
        // i represents 0
        else if(i == 0 || j == 0)
            return unum2<S, T>(j);

        // is exact
        bool ie = !(i & 0x01);
        bool je = !(j & 0x01);

        double i_left;
        double i_right;
        double j_left;
        double j_right;

        if(ie && je) 
            return unum2<S, T>::from(lattice.exactvalue(i & lattice._MASK) + lattice.exactvalue(j & lattice._MASK));
        else {
            j_left = lattice.exactvalue((j - 1) & lattice._MASK);
            j_right = lattice.exactvalue((j + 1) & lattice._MASK);
        }

        if(ie) {  // only i is exact
            i_left = lattice.exactvalue(i & lattice._MASK);
            i_right = i_left;
        }
        // only j is exact
        else if(je)
            return _sumpoint(j, i, lattice);
        else {
            // None is exact
            i_left = lattice.exactvalue((i - 1) & lattice._MASK);
            i_right = lattice.exactvalue((i + 1) & lattice._MASK);
        }

        S res_left_idx = unum2<S, T>::_from_index(i_left + j_left);
        S res_right_idx = unum2<S, T>::_from_index(i_right + j_right);

        return unum2<S, T>::_bound(res_left_idx, res_right_idx);
    }

    static unum2<S, T> _mulpoint(S i, S j, T lattice) {
        // inf * 0 = everything
        if((i == lattice._N_half && j == 0) || (i == 0 && j == lattice._N_half))
            return unum2<S, T>::everything();
        // inf * 1 = inf
        else if((i == lattice._N_half && j == lattice._N_quarter) || (i == lattice._N_quarter && j == lattice._N_half))
            return unum2<S, T>(lattice._N_half);  // inf
        // i represents 1
        else if(i == lattice._N_quarter)
            return unum2<S, T>(j);
        // j represents 1
        else if(j == lattice._N_quarter)
            return unum2<S, T>(i);
        // i represents 0
        else if(i == 0 || j == 0)
            return unum2<S, T>(0);

        // is exact
        bool ie = !(i & 0x01);
        bool je = !(j & 0x01);

        double i_left;
        double i_right;
        double j_left;
        double j_right;

        if(ie && je) 
            return unum2<S, T>::from(lattice.exactvalue(i & lattice._MASK) * lattice.exactvalue(j & lattice._MASK));
        else {
            j_left = lattice.exactvalue((j - 1) & lattice._MASK);
            j_right = lattice.exactvalue((j + 1) & lattice._MASK);
        }
            
        if(ie) {  // only i is exact
            i_left = lattice.exactvalue(i & lattice._MASK);
            i_right = i_left;
        }
        // only j is exact
        else if(je)
            return _mulpoint(j, i, lattice);
        else {
            // None is exact
            i_left = lattice.exactvalue((i - 1) & lattice._MASK);
            i_right = lattice.exactvalue((i + 1) & lattice._MASK);
        }

        // candidates
        double candidates[] = { i_left * j_left, i_left * j_right, i_right * j_left, i_right * j_right };

        // check for NaNs. That should occur only when we encounter things like inf * 0. Return everything
        // in this case.
        for(int i = 0; i < 4; i++) {
            if(std::isnan(candidates[i])) 
                return unum2<S, T>::everything();
        }

        double res_left = *std::min_element(candidates, candidates + 4);
        double res_right = *std::max_element(candidates, candidates + 4);
        S res_left_idx = unum2<S, T>::_from_index(res_left);
        S res_right_idx = unum2<S, T>::_from_index(res_right);

        if(!(res_left_idx & 0x01))  // exact
            res_left_idx = (res_left_idx + 1) & lattice._MASK;
        if(!(res_right_idx & 0x01))  // exact
            res_right_idx = (res_right_idx - 1) & lattice._MASK;

        return unum2<S, T>::_bound(res_left_idx, res_right_idx);
    }

    static unum2<S, T> _powpoint(S i, double n, T lattice) {
        if(!(i & 0x01)) {
            double value = lattice.exactvalue(i);
            value = std::pow(value, n);

            // e.g. sqrt(-2) which will result complex number.
            if(std::isnan(value))
                return unum2<S, T>::empty();
            
            return unum2<S, T>::from(value);
        }

        double left_bound = lattice.exactvalue((i - 1) & lattice._MASK);
        double right_bound = lattice.exactvalue((i + 1) & lattice._MASK);

        left_bound = std::pow(left_bound, n);
        right_bound = std::pow(right_bound, n);
        if(std::isnan(left_bound) || std::isnan(right_bound)) 
            return unum2<S, T>::empty();

        S left_idx = unum2<S, T>::_from_index(std::min(left_bound, right_bound));
        S right_idx = unum2<S, T>::_from_index(std::max(left_bound, right_bound));

        if(!(left_idx & 0x01))  // exact
            left_idx = (left_idx + 1) & lattice._MASK;
        if(!(right_idx & 0x01))  // exact
            right_idx = (right_idx - 1) & lattice._MASK;

        return unum2<S, T>::_bound(left_idx, right_idx);
    }
};

}}
