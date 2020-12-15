#pragma once
// sequences.hpp: definition of different sequences
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <tuple>

namespace sw { namespace sequences {

// generate the Fibonacci sequence of terms terms
template<typename Ty>
std::pair<Ty, Ty> Fibonacci(unsigned terms) {
    Ty first = 0, second = 1, next;
    for (unsigned c = 0; c < terms; ++c) {
	    if (c <= 1) {
		    next = Ty(c);
	    }
	    else {
		    next = first + second;
		    first = second;
		    second = next;
	    }
    }
    return std::pair<Ty, Ty>(first, second);
}

}}  // namespace sw::sequences
