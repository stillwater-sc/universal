#pragma once
// sequences.hpp: definition of different sequences
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <tuple>

namespace sw { namespace sequences {

// generate the last two terms of a Fibonacci sequence of t terms
template<typename Ty>
std::pair<Ty, Ty> GoldenRatio(unsigned t) {
    Ty n = 0, nplus1 = 1;
    for (unsigned c = 2; c < t; ++c) {
        Ty next = n + nplus1;
        n = nplus1;
        nplus1 = next;
    }
    return std::pair<Ty,Ty>(n, nplus1);
}

// generate the Fibonacci sequence of number of terms
template<typename Ty>
std::vector<Ty> Fibonacci(unsigned terms) {
    Ty n = 0, nplus1 = 1;
    std::vector<Ty> v;
    if (terms == 0) return v;
    v.push_back(n);
    if (terms == 1) return v;
    v.push_back(nplus1);
    if (terms == 2) return v;
    for (unsigned c = 3; c <= terms; ++c) {
	    Ty next = n + nplus1;
	    v.push_back(next);
	    n = nplus1;
        nplus1 = next;
    }
    return v;
}

}}  // namespace sw::sequences
