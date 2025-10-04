#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>
#include <bitset>
#include <cmath>

namespace sw { namespace universal {

template <typename S, typename T>
class unum2 {
private:
    T _lattice;

    // SORN
    uint64_t _sorn_length;
    // TODO: Use unviersal internal libraries
    std::bitset<1 << (sizeof(S) * 8)> _sorn;  // empty

public:
    unum2(S index) : _lattice(T()), _sorn_length(_lattice._N) {
        if(_lattice._N > (1 << (sizeof(S) * 8))) 
            throw std::invalid_argument("Lattice point count overflows given storage type!");

        // Do bitwise or with with half the lattice size. SORN index starts from 2's compliment. That is
        // the first bit in SORN represents 'inf' instead of '0'. Next SORN bit represents (inf, -en) and
        // so on so forth.
        _sorn = std::bitset<1 << (sizeof(S) * 8)>(1);
        _sorn <<= ((index ^ _lattice._N_half) & _lattice._MASK);
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
        
        unum2<S, T> au = unum2<S, T>::from(a);
        unum2<S, T> bu = unum2<S, T>::from(b);
        
        if(a < b) {   
            auto res = au._sorn;
            auto criterion = res;

            if(std::isinf(b)) {
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

        // TODO: Implement criterion a > b
        return unum2<S, T>::empty();
    }

    // Addition
    unum2<S, T> operator + (unum2<S, T>& other) {
        unum2<S, T> res = unum2<S, T>::empty();

        for(S i = 0; i < _sorn_length; i++) {
            for(S j = 0; j < _sorn_length; j++) {
                if(_sorn[i] == 1 && other._sorn[j] == 1)
                    res._sorn |= unum2<S, T>::_sumpoint((i ^ _lattice._N_half) & _lattice._MASK, (j ^ _lattice._N_half) & _lattice._MASK)._sorn;
            }
        }

        return res;
    }

    // Multiplication
    unum2<S, T> operator * (unum2<S, T>& other) {
        unum2<S, T> res = unum2<S, T>::empty();

        for(S i = 0; i < _sorn_length; i++) {
            for(S j = 0; j < _sorn_length; j++) {
                if(_sorn[i] == 1 && other._sorn[j] == 1)
                    res._sorn |= unum2<S, T>::_mulpoint((i ^ _lattice._N_half) & _lattice._MASK, (j ^ _lattice._N_half) & _lattice._MASK)._sorn;
            }
        }

        return res;
    }

    // Negation
    unum2<S, T> operator - () {
        unum2<S, T> res = unum2<S, T>::empty();

        for(uint64_t i = 0; i < _sorn_length; i++) {
            if(_sorn[i] == 1) {
                // Horizontal invert
                uint64_t neg_idx = ((~((i ^ _lattice._N_half) & _lattice._MASK) & _lattice._MASK) + 1) & _lattice._MASK;
                res._sorn = unum2<S, T>(neg_idx)._sorn;
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
        unum2<S, T> res = unum2<S, T>::empty();

        for(uint64_t i = 0; i < _sorn_length; i++) {
            if(_sorn[i] == 1) {
                // Vertical invert
                uint64_t ix = (i ^ _lattice._N_half) & _lattice._MASK;
                uint64_t msb_set = ix & msb_mask;
                uint64_t inverted_idx = ~ix & _lattice._MASK;

                if(!msb_set) inverted_idx = (inverted_idx & msb_set) | (inverted_idx & (msb_mask - 1));
                else inverted_idx |= msb_set;
                inverted_idx = (inverted_idx + 1) & _lattice._MASK;

                res._sorn = unum2<S, T>(inverted_idx)._sorn;
            }
        }

        return res;
    }

    unum2<S, T> operator / (unum2<S, T>& other) {
        auto inv = ~other;
        return this->unum2<S, T>::operator * (inv);
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

    static unum2<S, T> _sumpoint(uint64_t i, uint64_t j) {
        T lattice = T();
    
        // i and j both represent infinity
        if(i == lattice._N_half && j == lattice._N_half) 
            return unum2<S, T>::everything();
        
        // i or j represent infinity
        else if(i == lattice._N_half || j == lattice._N_half)
            return unum2<S, T>(lattice._N_half);  // inf
        
        // i represents 0
        else if(i == 0)
            return unum2<S, T>(j);
        // vice versa
        else if(j == 0)
            return unum2<S, T>(i);

        bool ie = !(i & 0x01);
        bool je = !(j & 0x01);

        double i_left;
        double i_right;
        double j_left = lattice.exactvalue((j - 1) & lattice._MASK);
        double j_right = lattice.exactvalue((j + 1) & lattice._MASK);

        if(ie && je) 
            return unum2<S, T>::from(lattice.exactvalue(i & lattice._MASK) + lattice.exactvalue(j & lattice._MASK));
        else if(ie) {  // only i is exact
            i_left = lattice.exactvalue(i & lattice._MASK);
            i_right = i_left;
        }
        // only j is exact
        else if(je)
            return _sumpoint(j, i);
        else {
            // None is exact
            i_left = lattice.exactvalue((i - 1) & lattice._MASK);
            i_right = lattice.exactvalue((i + 1) & lattice._MASK);
        }

        double res_left;
        S res_left_idx = unum2<S, T>::_from_index(i_left + j_left);
        if(res_left_idx & 0x01) {  // inexact index
            double _left = lattice.exactvalue((res_left_idx - 1) & lattice._MASK);
            double _right = lattice.exactvalue((res_left_idx + 1) & lattice._MASK);

            // for (-inf, -en)
            if(res_left_idx - 1 == lattice._N_half) 
                res_left = _right - 1;
            // for (en, inf)
            else if(res_left_idx + 1 == lattice._N_half)
                res_left = _left + 1;
            else res_left = (_right + _left) / 2.0;
        } else {
            double _exact = lattice.exactvalue(res_left_idx & lattice._MASK);
            double _right = lattice.exactvalue((res_left_idx + 2) & lattice._MASK);

            if(res_left_idx == lattice._N_half)  // inf
                res_left = _right - 1;
            else if(res_left_idx + 2 == lattice._N_half)  // inf
                res_left = _exact + 1;
            else res_left = (_exact + _right) / 2.0;
        }

        double res_right;
        S res_right_idx = unum2<S, T>::_from_index(i_right + j_right);
        if(res_right_idx & 0x01) {
            double _left = lattice.exactvalue((res_right_idx - 1) & lattice._MASK);
            double _right = lattice.exactvalue((res_right_idx + 1) & lattice._MASK);

            // for (-inf, -en)
            if(res_right_idx - 1 == lattice._N_half)
                res_right = _right - 1;
            // for (en, inf)
            else if(res_right_idx + 1 == lattice._N_half) 
                res_right = _left + 1;
            else res_right = (_left + _right) / 2.0;
        } else {  // inexact index
            double _left = lattice.exactvalue((res_right_idx - 2) & lattice._MASK);
            double _exact = lattice.exactvalue(res_right_idx & lattice._MASK);

            if(res_right_idx == lattice._N_half)  // inf
                res_right = _left + 1;
            else if(res_right_idx - 2 == lattice._N_half)  // inf
                res_right = _exact - 1;
            else res_right = (_left + _exact) / 2.0;
        }

        return unum2<S, T>::interval(res_left, res_right);
    }

    static unum2<S, T> _mulpoint(uint64_t i, uint64_t j) {
        T lattice = T();
        
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
        // Either i or j represents infinity
        else if(i == lattice._N_half || j == lattice._N_half)
            return unum2<S, T>::everything();
        
        // i represents 0
        else if(i == 0 || j == 0)
            return unum2<S, T>(0);

        bool ie = !(i & 0x01);
        bool je = !(j & 0x01);

        double i_left;
        double i_right;
        double j_left = lattice.exactvalue((j - 1) & lattice._MASK);
        double j_right = lattice.exactvalue((j + 1) & lattice._MASK);

        if(ie && je) 
            return unum2<S, T>::from(lattice.exactvalue(i & lattice._MASK) * lattice.exactvalue(j & lattice._MASK));
        else if(ie) {  // only i is exact
            i_left = lattice.exactvalue(i & lattice._MASK);
            i_right = i_left;
        }
        // only j is exact
        else if(je)
            return _mulpoint(j, i);
        else {
            // None is exact
            i_left = lattice.exactvalue((i - 1) & lattice._MASK);
            i_right = lattice.exactvalue((i + 1) & lattice._MASK);
        }

        double res_left;
        S res_left_idx = unum2<S, T>::_from_index(i_left * j_left);
        if(res_left_idx & 0x01) {  // inexact index
            double _left = lattice.exactvalue((res_left_idx - 1) & lattice._MASK);
            double _right = lattice.exactvalue((res_left_idx + 1) & lattice._MASK);

            // for (-inf, -en)
            if(res_left_idx - 1 == lattice._N_half) 
                res_left = _right - 1;
            // for (en, inf)
            else if(res_left_idx + 1 == lattice._N_half)
                res_left = _left + 1;
            else res_left = (_right + _left) / 2.0;
        } else {
            double _exact = lattice.exactvalue(res_left_idx & lattice._MASK);
            double _right = lattice.exactvalue((res_left_idx + 2) & lattice._MASK);

            if(res_left_idx == lattice._N_half)  // inf
                res_left = _right - 1;
            else if(res_left_idx + 2 == lattice._N_half)  // inf
                res_left = _exact + 1;
            else res_left = (_exact + _right) / 2.0;
        }

        double res_right;
        S res_right_idx = unum2<S, T>::_from_index(i_right * j_right);
        if(res_right_idx & 0x01) {
            double _left = lattice.exactvalue((res_right_idx - 1) & lattice._MASK);
            double _right = lattice.exactvalue((res_right_idx + 1) & lattice._MASK);

            // for (-inf, -en)
            if(res_right_idx - 1 == lattice._N_half)
                res_right = _right - 1;
            // for (en, inf)
            else if(res_right_idx + 1 == lattice._N_half) 
                res_right = _left + 1;
            else res_right = (_left + _right) / 2.0;
        } else {  // inexact index
            double _left = lattice.exactvalue((res_right_idx - 2) & lattice._MASK);
            double _exact = lattice.exactvalue(res_right_idx & lattice._MASK);

            if(res_right_idx == lattice._N_half)  // inf
                res_right = _left + 1;
            else if(res_right_idx - 2 == lattice._N_half)  // inf
                res_right = _exact - 1;
            else res_right = (_left + _exact) / 2.0;
        }

        return unum2<S, T>::interval(res_left, res_right);
    }
};

}}
