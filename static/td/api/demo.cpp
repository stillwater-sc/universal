// api.cpp: application programming interface tests for double-double (dd) number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/internal/floatcascade/dd_impl.hpp>
#include <universal/number/td/td.hpp>
// Test the interoperability design

#include <iostream>
#include <iomanip>

int main() {
    using namespace sw::universal;
    
    std::cout << std::setprecision(17);
    
    // Test basic floatcascade construction
    floatcascade<2> fc2(1.5);
    floatcascade<3> fc3(2.5);
    
    std::cout << "floatcascade<2>: " << fc2 << std::endl;
    std::cout << "floatcascade<3>: " << fc3 << std::endl;
    
    // Test number system interoperability
    dd d1(1.0);
    td t1(2.0);
    
    std::cout << "\nOriginal values:" << std::endl;
    std::cout << "dd: " << d1 << std::endl;
    std::cout << "td: " << t1 << std::endl;
    
    // Convert dd to td
    //td t_from_d(d1);
    td t_from_d(d1.get_cascade());
    std::cout << "td from dd: " << t_from_d << std::endl;
    
    // Arithmetic between same types
    dd d2(0.5);
    dd d_sum = d1 + d2;
    std::cout << "dd + dd: " << d_sum << std::endl;
    
    td t2(0.25);
    td t_sum = t1 + t2;
    std::cout << "td + td: " << t_sum << std::endl;
    
    // Test floatcascade extraction and assignment
    floatcascade<3> extracted = t_sum.get_cascade();
    td t_reassigned;
    t_reassigned = extracted;
    std::cout << "Extracted and reassigned td: " << t_reassigned << std::endl;
    
    return 0;
}
