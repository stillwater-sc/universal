// construct.cpp: functional tests to construct unums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>

// Use operation table
#define UNUM2_USE_OP_MATRIX 1
#include <universal/number/unum2/unum2.hpp>


int main(int argc, char* argv[]) 
try {
	int nrOfFailedTestCases = 0;
    using namespace sw::universal;
    using u2 = unum2<linear_5bit>;

    // Print lattice
    linear_5bit().print();

    u2 a = u2(9); // 9th index in the lattice -> (1, 2)
    if(a != u2::from(1.5)) nrOfFailedTestCases++;

    a = u2::interval(-0.125, 3.0);
    if(a != (u2::interval(-0.125, 0.0) | u2::interval(0.0, 3.0))) 
        nrOfFailedTestCases++;
    
    // Reverse interval
    std::cout << "Inverse interval, (1.0, -2.5): " << u2::interval(1.0, -2.5) << std::endl;

    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
