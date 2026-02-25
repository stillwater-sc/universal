// constexpr_test.cpp: see whether posits can be constexpr 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// forth: enable/disable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1

// requires C++20 <bit>

#include <universal/number/posit1/posit1.hpp>

int main()
{
    using namespace sw::universal;

//     constexpr sw::universal::posit<32, 2> p1{4.2};
//     constexpr sw::universal::posit<32, 2> p2= 4.3;
    posit<32, 2> a(SpecificValue::maxpos);
    
    return 0;
}
