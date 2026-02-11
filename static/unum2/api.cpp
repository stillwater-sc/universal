// api.cpp: class interface tests for arbitrary configuration unum types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//

// minimum set of include files to reflect source code dependencies
// Use operation matrix/table
#define UNUM2_USE_OP_MATRIX 1
#include <universal/number/unum2/unum2.hpp>
#include <limits>


int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
    using u2 = unum2<linear_5bit>;

    // Print lattice
    linear_5bit().print();

	std::cout << "unum class interface tests\n";


    // Arithmetic
    {
		int start = nrOfFailedTestCases;
        u2 a = u2::from(3.5);  // (3, 4)
        u2 b = u2::interval(-1.0, 1.0 / 2);  // [-1.0, /2]
        
        // `|` operator is SORN union.
        if(a + b != (u2(0) | u2::interval(2, 47)))  // compare against 0 U [2, inf)
            ++nrOfFailedTestCases;

        if(a - b != (u2(0) | u2::interval(2.5, 47.0)))  // compare against 0 U (2, inf)
            ++nrOfFailedTestCases;

        if(a * b != u2::interval(-3.5, 1.5))  // compare against (-4, 2)
            ++nrOfFailedTestCases;
        
        if(a / b != u2::interval(-47, 47))  // compare against (inf, inf)
            ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
            std::cout << "FAIL : " << a << ", " << b << std::endl;
		}
    }

    // Unary
    {
		int start = nrOfFailedTestCases;
        u2 a = u2::interval(-3.0, 1.0 / 3);  // [-3, /3]

        if(-a != u2::interval(-1.0 / 3, 3.0)) 
            ++nrOfFailedTestCases;

        // invert
        // compare against [inf, -/3] U [3, inf]
        if(~a != (u2::interval(std::numeric_limits<double>::infinity(), -1.0 / 3) | u2::interval(3.0, std::numeric_limits<double>::infinity())))
            ++nrOfFailedTestCases;

        // absolute
        if(a.abs() != u2::interval(0, 3))
            ++nrOfFailedTestCases;

        // Raise to power
        if((a ^ 3) != u2::interval(-47.0, 1.0 / 8))  // compare against (inf, /4)
            ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
            std::cout << "FAIL : " << a << std::endl;
		}
    }


	std::cout << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
/*
catch (const sw::universal::unum2_arithmetic_exception& err) {
	std::cerr << "Uncaught unum2 arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum2_internal_exception& err) {
	std::cerr << "Uncaught unum2 internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
*/
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
