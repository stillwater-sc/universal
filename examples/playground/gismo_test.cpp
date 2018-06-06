// gismo_test.cpp example testing cricial features for G+Smo integration
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"

// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERSION

#include <string>
#include <iostream>
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

            std::string string = "3.141";
            std::istringstream lnstream;
            lnstream.unsetf(std::ios_base::skipws);
            lnstream.clear();
            lnstream.str(string);
                        
            posit_32_2 p;
            lnstream >> std::ws >> p;
        }

	{ // Test conversion from long to posit

	    posit_32_2 p = 1/posit_32_2(10000000000);
	}        
        return 0;
    }
    catch (char const* msg) {
        std::cerr << msg << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Caught unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
