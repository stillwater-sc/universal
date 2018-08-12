// gismo_test.cpp example testing cricial features for G+Smo integration
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERSION
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>

int main(int argc, char** argv)
try {
    using namespace std;
    using namespace sw::unum;

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
		cout << pretty_print(p) << endl;
    }

{ // Test conversion from long to posit

	posit_32_2 p = 1/posit_32_2(10000000000);
	cout << pretty_print(p) << endl;
}        
    return 0;
}
catch (char const* msg) {
    std::cerr << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
