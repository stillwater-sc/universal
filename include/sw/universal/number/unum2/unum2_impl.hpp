// Universal Number 2.0 implementation including SORN (Sets of Real Numbers).
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once


#ifndef UNUM2_USE_OP_MATRIX
#define UNUM2_USE_OP_MATRIX     1  // Use op matrix
#endif  // UNUM2_USE_OP_MATRIX


#include <universal/number/unum2/common.hpp>
#include <universal/number/unum2/unum2_fwd.hpp>

#include <cstdint>
#include <bitset>
#include <iosfwd>     // std::ostream, named by the operator<< friend declaration
#include <algorithm>
#include <cmath>

namespace sw { namespace universal {

template <size_t N>
static inline int _find_first(const std::bitset<N>& b)
{
    for (size_t i = 0; i < N; ++i) {
        if (b.test(i))
            return static_cast<int>(i);
    }

    return -1;
}

template <int... exacts>
class unum2<lattice<exacts...>> final {
private:
    using T = lattice<exacts...>;
    const T& _lattice;

    // SORN
    constexpr static uint64_t sorn_length = sizeof... (exacts) << 3;
    std::bitset<sorn_length> _sorn;  // empty

public:
    unum2(uint64_t index) : _lattice(T::instance()) {
        //uint64_tDo bitwise or with with half the lattice size. SORN index starts from 2's compliment. That is
        // the first bit in SORN represents 'inf' instead of '0'. Next SORN bit represents (inf, -en) and
        // so on so forth.
        _sorn = std::bitset<sorn_length>().set((index ^ _lattice._N_half) & _lattice._MASK);
    }

    unum2() : _lattice(T::instance()) {
        _sorn.reset();
    }

    unum2(const unum2<T>& num) : 
        _lattice(num._lattice),
        _sorn(num._sorn)
    {}

    unum2(unum2<T>&& num) noexcept :
        _lattice(num._lattice),
        _sorn(std::move(num._sorn))
    {}

    unum2<T>& operator = (const unum2<T>& num) {
        if (this != &num)
            _sorn = num._sorn;
        return *this;
    }

    unum2<T>& operator = (unum2<T>&& num) noexcept {
        if (this != &num)
            _sorn = std::move(num._sorn);
        return *this;
    }

    // stream insertion: declared here -- it reads the SORN and the lattice -- and defined
    // in iostream.hpp, which is what keeps <sstream> out of the core (#1334)
    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const unum2<U>& u);

    static unum2<T> empty() {
        unum2<T> res(0);
        res._sorn.reset();

        return res;
    }

    static unum2<T> everything() {
        unum2<T> res(0);
        res._sorn.set();

        return res;
    }

    template<typename TT>
    static unum2<T> from(TT value) {
        return unum2<T>(_from_index(value));
    }

    template<typename TT>
    static unum2<T> interval(TT a, TT b) {
        if(a == b) 
            return unum2<T>::from(a);
        
        uint64_t ai = unum2<T>::_from_index(a);
        uint64_t bi = unum2<T>::_from_index(b);
        
        if(a < b) return unum2<T>::_bound(ai, bi);
        return unum2<T>::_bound_inverse(ai, bi);
    }

    // Addition
    unum2<T> operator + (const unum2<T>& other) const {
        auto res = unum2<T>::empty();
        for(uint64_t i = 0; i < sorn_length; i++) {
            for(uint64_t j = 0; j < sorn_length; j++) {
                if(_sorn[i] == 1 && other._sorn[j] == 1)
                    res._sorn |= unum2<T>::_sumpoint(_conv_idx(i), _conv_idx(j), _lattice)._sorn;
            }
        }

        return res;
    }

    // SORN Concatenation or Union
    unum2<T> operator | (const unum2<T>& other) const {
        auto res = unum2<T>::empty();
        res._sorn = this->_sorn | other._sorn;
        return res;
    }

    // Multiplication
    unum2<T> operator * (const unum2<T>& other) const {
        auto res = unum2<T>::empty();
        for(uint64_t i = 0; i < sorn_length; i++) {
            for(uint64_t j = 0; j < sorn_length; j++) {
                if(_sorn[i] == 1 && other._sorn[j] == 1)
                    res._sorn |= unum2<T>::_mulpoint(_conv_idx(i), _conv_idx(j), _lattice)._sorn;
            }
        }

        return res;
    }

    // Negation
    unum2<T> operator - () const {
        auto res = unum2<T>::empty();
        for(uint64_t i = 0; i < sorn_length; i++) {
            if(_sorn[i] == 1) {
                // Horizontal invert
                uint64_t neg_idx = _horizontal_invert(_conv_idx(i), _lattice._MASK);
                res._sorn.set(_conv_idx(neg_idx));
            }
        }

        return res;
    }

    // Subtraction
    unum2<T> operator - (const unum2<T>& other) const {
        auto neg = -other;
        return this->unum2<T>::operator + (neg);
    }

    // Invert
    unum2<T> operator ~ () const {
        uint64_t msb_mask = _lattice._N >> 1;
        auto res = unum2<T>::empty();
        for(uint64_t i = 0; i < sorn_length; i++) {
            if(_sorn[i] == 1) {
                // Vertical invert
                uint64_t ix = _conv_idx(i);
                uint64_t msb_set = ix & msb_mask;
                uint64_t inverted_idx = ~ix & _lattice._MASK;

                if(!msb_set) inverted_idx &= (msb_mask - 1);
                else inverted_idx |= msb_set;
                inverted_idx = (inverted_idx + 1) & _lattice._MASK;

                res._sorn.set(_conv_idx(inverted_idx));
            }
        }

        return res;
    }

    unum2<T> operator / (const unum2<T>& other) const {
        auto inv = ~other;
        return this->unum2<T>::operator * (inv);
    }

    bool operator == (const unum2<T>& other) const {
        return this->_sorn == other._sorn;
    }

    // Raise to a power
    unum2<T> operator ^ (double n) const {
        auto res = unum2<T>::empty();
        for(uint64_t i = 0; i < sorn_length; i++) {
            if(_sorn[i] == 1)
                res._sorn |= unum2<T>::_powpoint(_conv_idx(i), n, _lattice)._sorn;
        }

        return res;
    }

    // Absolute
    unum2<T> abs() const {
        auto res = unum2<T>::empty();
        for(uint64_t i = 0; i < sorn_length; i++) {
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

    uint64_t _conv_idx(uint64_t idx) const {
        // In Unum2 SORN, 0 starts from _N_half, instead of 0 for compatibility
        // reasons. This function converts adjusted index (e.g. 15 -> 0) to
        // absolute index (e.g. 0 -> 0) and vice versa.
        return (idx ^ _lattice._N_half) & _lattice._MASK;
    }

private:
    template<typename TT>
    static uint64_t _from_index(TT value) {
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
        const size_t exact_size = lattice._exacts.size();

        // Try exact values.
        int64_t index = -1;
        for(size_t i = 1; i < exact_size; i++) {
            const int e = lattice._exacts[i];
            
            if(absolute_value == static_cast<TT>(e)) {
                // Vertical flip
                index = static_cast<int64_t>((i << 1) + lattice._N_quarter);
                break;
            }
            else if(absolute_value == (1 / static_cast<TT>(e))) {
                index = static_cast<int64_t>(lattice._N_quarter - (i << 1));
                break;
            }

            // No exacts :(
            // Compare bounds.
            if(absolute_value > static_cast<TT>(lattice._exacts[i - 1]) && 
            absolute_value < static_cast<TT>(e))
            {
                // Vertical flip.
                index = static_cast<int64_t>((i << 1) + lattice._N_quarter - 1);
                break;
            } else {
                TT reciprocal_right = 1 / static_cast<TT>(lattice._exacts[exact_size - i - 1]);
                TT reciprocal_left = 1 / static_cast<TT>(lattice._exacts[exact_size - i]);

                if(absolute_value > reciprocal_left && absolute_value < reciprocal_right) {
                    index = static_cast<int64_t>((i << 1) + 1);
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

    static unum2<T> _bound(uint64_t a, uint64_t b) {
        // Given unum has to be points.
        // and a < b
        // Note: Unum 'a' is operated on, thus changes.
        
        if(a == b) 
            return a;

        auto au = unum2<T>(a);
        auto bu = unum2<T>(b);
        auto res = au._sorn;
        auto criterion = res;

        // If b is infinity
        if(b == au._lattice._N_half) {
            // *FIXME* mixed-domain shift_count
            int shift_count = au._lattice._N - _find_first(au._sorn);
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

    static unum2<T> _bound_inverse(uint64_t a, uint64_t b) {
        // When bound a > bound b
        auto res = unum2<T>::_bound(b, a);
        res._sorn = res._sorn ^ std::bitset<sorn_length>().set();
        // Lattice should move coutner-clockwise in this case, but including the bounded
        // unums.
        res._sorn.set(a ^ res._lattice._N_half).set(b ^ res._lattice._N_half);
        return res;
    }

    static unum2<T> _sumpoint(uint64_t i, uint64_t j, const T& lattice) {
        op_matrix<T>* op_mat;

        if(UNUM2_USE_OP_MATRIX) {
            op_mat = &T::op_matrix_instance();

            if(op_mat->has(i, j, OP_MATRIX_TYPE_ADD))
                return op_mat->get(i, j, OP_MATRIX_TYPE_ADD);
        }

        unum2<T> res;

        // i and j both represent infinity
        if(i == lattice._N_half && j == lattice._N_half) {
            res = unum2<T>::everything();
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_ADD, res);
            return res;
        }
        // i or j represent infinity
        else if(i == lattice._N_half || j == lattice._N_half) {
            res = unum2<T>(lattice._N_half);  // inf
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_ADD, res);
            return res;
        }
        // i represents 0
        else if(i == 0 || j == 0) {
            res = unum2<T>(j);
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_ADD, res);
            return res;
        }

        // is exact
        bool ie = !(i & 0x01);
        bool je = !(j & 0x01);

        double i_left;
        double i_right;
        double j_left;
        double j_right;

        if(ie && je) {
            res = unum2<T>::from(lattice.exactvalue(i & lattice._MASK) + lattice.exactvalue(j & lattice._MASK));
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_ADD, res);
            return res;
        } else {
            j_left = lattice.exactvalue((j - 1) & lattice._MASK);
            j_right = lattice.exactvalue((j + 1) & lattice._MASK);
        }

        if(ie) {  // only i is exact
            i_left = lattice.exactvalue(i & lattice._MASK);
            i_right = i_left;
        }
        // only j is exact
        else if(je) {
            res = _sumpoint(j, i, lattice);
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_ADD, res);
            return res;
        } else {
            // None is exact
            i_left = lattice.exactvalue((i - 1) & lattice._MASK);
            i_right = lattice.exactvalue((i + 1) & lattice._MASK);
        }

        uint64_t res_left_idx = unum2<T>::_from_index(i_left + j_left);
        uint64_t res_right_idx = unum2<T>::_from_index(i_right + j_right);

        res = unum2<T>::_bound(res_left_idx, res_right_idx);
        if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_ADD, res);
        return res;
    }

    static unum2<T> _mulpoint(uint64_t i, uint64_t j, const T& lattice) {
        op_matrix<T>* op_mat;

        if(UNUM2_USE_OP_MATRIX) {
            op_mat = &T::op_matrix_instance();

            if(op_mat->has(i, j, OP_MATRIX_TYPE_MUL)) {
                return op_mat->get(i, j, OP_MATRIX_TYPE_MUL);
            }
        }

        unum2<T> res;

        // inf * 0 = everything
        if((i == lattice._N_half && j == 0) || (i == 0 && j == lattice._N_half)) {
            res = unum2<T>::everything();
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        }
        // inf * 1 = inf
        else if((i == lattice._N_half && j == lattice._N_quarter) || (i == lattice._N_quarter && j == lattice._N_half)) {
            res = unum2<T>(lattice._N_half);  // inf
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        } 
        // i represents 1
        else if(i == lattice._N_quarter) {
            res = unum2<T>(j);
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        }
        // j represents 1
        else if(j == lattice._N_quarter) {
            res = unum2<T>(i);
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        } 
        // i represents 0
        else if(i == 0 || j == 0) {
            res = unum2<T>(0);
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        } 

        // is exact
        bool ie = !(i & 0x01);
        bool je = !(j & 0x01);

        double i_left;
        double i_right;
        double j_left;
        double j_right;

        if(ie && je) {
            res = unum2<T>::from(lattice.exactvalue(i & lattice._MASK) * lattice.exactvalue(j & lattice._MASK));
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        }
        else {
            j_left = lattice.exactvalue((j - 1) & lattice._MASK);
            j_right = lattice.exactvalue((j + 1) & lattice._MASK);
        }
            
        if(ie) {  // only i is exact
            i_left = lattice.exactvalue(i & lattice._MASK);
            i_right = i_left;
        }
        // only j is exact
        else if(je) {
            res = _mulpoint(j, i, lattice);
            if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
            return res;
        } else {
            // None is exact
            i_left = lattice.exactvalue((i - 1) & lattice._MASK);
            i_right = lattice.exactvalue((i + 1) & lattice._MASK);
        }

        // candidates
        double candidates[] = { i_left * j_left, i_left * j_right, i_right * j_left, i_right * j_right };

        // check for NaNs. That should occur only when we encounter things like inf * 0. Return everything
        // in this case.
        for(int i = 0; i < 4; i++) {
            if(std::isnan(candidates[i])) {
                res = unum2<T>::everything();
                if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
                return res;
            }    
        }

        double res_left = *std::min_element(candidates, candidates + 4);
        double res_right = *std::max_element(candidates, candidates + 4);
        uint64_t res_left_idx = unum2<T>::_from_index(res_left);
        uint64_t res_right_idx = unum2<T>::_from_index(res_right);

        if(!(res_left_idx & 0x01))  // exact
            res_left_idx = (res_left_idx + 1) & lattice._MASK;
        if(!(res_right_idx & 0x01))  // exact
            res_right_idx = (res_right_idx - 1) & lattice._MASK;

        res = unum2<T>::_bound(res_left_idx, res_right_idx);
        if(UNUM2_USE_OP_MATRIX) op_mat->set(i, j, OP_MATRIX_TYPE_MUL, res);
        return res;
    }

    static unum2<T> _powpoint(uint64_t i, double n, const T& lattice) {
        if(!(i & 0x01)) {
            double value = lattice.exactvalue(i);
            value = std::pow(value, n);

            // e.g. sqrt(-2) which will result complex number.
            if(std::isnan(value))
                return unum2<T>::empty();
            
            return unum2<T>::from(value);
        }

        double left_bound = lattice.exactvalue((i - 1) & lattice._MASK);
        double right_bound = lattice.exactvalue((i + 1) & lattice._MASK);

        left_bound = std::pow(left_bound, n);
        right_bound = std::pow(right_bound, n);
        if(std::isnan(left_bound) || std::isnan(right_bound)) 
            return unum2<T>::empty();

        uint64_t left_idx = unum2<T>::_from_index(std::min(left_bound, right_bound));
        uint64_t right_idx = unum2<T>::_from_index(std::max(left_bound, right_bound));

        if(!(left_idx & 0x01))  // exact
            left_idx = (left_idx + 1) & lattice._MASK;
        if(!(right_idx & 0x01))  // exact
            right_idx = (right_idx - 1) & lattice._MASK;

        return unum2<T>::_bound(left_idx, right_idx);
    }
};


template<typename T> unum2<T> pow(unum2<T> u, int n) {
    return u ^ n;
}

template<typename T> unum2<T> abs(unum2<T> u) {
    return u.abs();
}

}}
