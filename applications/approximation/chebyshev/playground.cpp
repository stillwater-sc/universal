// Perform Tests on Chebfun files
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, 
// which is released under an MIT Open Source license.
//
// Author: James Quinlan

#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>


float scale = 3.14f;
float normalizer = 3.6545f;

 


 


int main()
try {
	using namespace std;
	using namespace sw::universal;
	using namespace sw::universal::blas;

    // Function
    auto f = []<typename T>(T x) {return x*x;};
    // Do lambdas take vectors?  MATLAB like?
    

    // Display output
    auto y = f(1.415);
    std::cout << y << std::endl;


	return EXIT_SUCCESS;
}

// Runtime error statements
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}