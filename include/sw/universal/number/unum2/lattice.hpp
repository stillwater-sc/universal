// Lattice implementation for Unum 2.0
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once

#include <universal/number/unum2/common.hpp>
#include <universal/number/unum2/op_matrix.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <cmath>
#include <initializer_list>
#include <stdexcept>


static inline bool _is_power_of_two(uint64_t value) {
    int count = value & 0x01;
    while(value = value >> 1) 
        count += value & 0x01;
    
    return count == 1;
}


namespace sw { namespace universal {

template <int... exacts>
class lattice final {
private:
    std::vector<int> _exacts = { exacts... };
    uint64_t _N = sizeof... (exacts) << 3;  // Number of points including non-exacts
    uint64_t _N_half = _N >> 1;
    uint64_t _N_quarter = _N_half >> 1;
    uint64_t _MASK = _N - 1;

public:
    lattice() {
        if(!_is_power_of_two(_N))
            throw std::invalid_argument("Number of elements in the lattice has to be power of 2!");
    
        // First element in the lattice must be 1.
        if(_exacts[0] != 1)
            throw std::invalid_argument("First element in lattice must be 1");
        
        int last = 1;
        for(std::vector<int>::iterator it = _exacts.begin() + 1; it < _exacts.end(); ++it) {
            if(*it < 1 || *it <= last) 
                throw std::invalid_argument("Lattice always be in ascending order");
            last = *it;
        }
    }

    lattice(const lattice&) = delete;
    lattice& operator=(const lattice&) = delete;

    std::string get_exact(uint64_t i) const {
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

    void print() const {
        std::cout << "inf <-->";

        size_t size = _exacts.size();
    
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

    double exactvalue(uint64_t i) const {
        if(i >= _N) 
            throw std::out_of_range("Lattice index out of range");
    
        // Return nothing if not exact.
        if(i & 0x01) 
            return 0;
        
        // Check for infinity, zero, -1 or 1.
        if(i == _N >> 1) 
            return INFINITY;
        else if(i == 0)
            return 0;
        else if(i == _N_half)
            return 1;
        else if(i == 3 * _N_quarter)
            return -1;

        double res = 1;

        // Negative
        if(i > _N_half) {
            res = -1;
            i = _horizontal_invert(i, _MASK);
        }

        // Vertical invert
        if(i >= _N_quarter) 
            res *= static_cast<double>(_exacts[(i - _N_quarter) >> 1]);
        else res *= 1 / static_cast<double>(_exacts[_exacts.size() - (i >> 1)]);

        return res;
    }

    static lattice& instance() {
        static lattice<exacts...> _lat;
        return _lat;
    }

    template<typename S>
    static op_matrix<S, lattice>& op_matrix_instance() {
        static op_matrix<S, lattice> _op_mat = op_matrix<S, lattice>(sizeof... (exacts) << 3);
        return _op_mat;
    }

    template<typename S, typename T> friend class sw::universal::unum2;
};


// SOME DEFAULT LATTICES

// 5-bit linear lattice, 32 points.
using linear_5bit = lattice<1, 2, 3, 4>;

// 8-bit linear lattice, 256 points
using linear_8bit = lattice<1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32>;

}}
