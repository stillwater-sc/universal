// constexpr_test.cpp: see whether posits can be constexpr 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// forth: enable/disable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1

// requires C++20 <bit>

// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit_impl.hpp>
#include <universal/number/posit/numeric_limits.hpp>

#define CONSTEXPRESSION 

int main(int argc, char** argv)
{
    using namespace sw::universal;

//     constexpr sw::universal::posit<32, 2> p1{4.2};
//     constexpr sw::universal::posit<32, 2> p2= 4.3;
    CONSTEXPRESSION posit<32, 2> a(SpecificValue::maxpos);
    
    return 0;
}
