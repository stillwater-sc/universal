// gismo_test.cpp example testing cricial features for G+Smo integration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>

int main(int argc, char** argv)
try {
    using namespace sw::universal;

    typedef posit<32,2> posit_32_2;

    { // Test conversion from std::size_t to posits
        size_t size = 32;
        posit_32_2 p (size);
    }

    { // Test reading posit from std::istringstream

        std::string str = "3.141";
        std::istringstream lnstream;
        lnstream.unsetf(std::ios_base::skipws);
        lnstream.clear();
        lnstream.str(str);
                        
        posit_32_2 p;
        lnstream >> std::ws >> p;
        std::cout << pretty_print(p) << std::endl;
    }

{ // Test conversion from long to posit

	posit_32_2 p = 1/posit_32_2(10000000000);
	std::cout << pretty_print(p) << std::endl;
}        
    return 0;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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
