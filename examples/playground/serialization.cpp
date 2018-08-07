// gismo_test.cpp example testing cricial features for G+Smo integration
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"

#include <string>
#include <iostream>
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>

int main(int argc, char** argv)
    try {
        using namespace std;
        using namespace sw::unum;

		// Test reading posit from std::istringstream
        posit<32,2> p;

        std::string str = "3.1415926535897932384626433832795028841971693993751058209749445923078164062";
        std::istringstream lnstream;
        lnstream.unsetf(std::ios_base::skipws);
        lnstream.clear();
        lnstream.str(str);                       
        lnstream >> std::ws >> p;
		cout << "IEEE float/double format, parsed into a posit<32,2>: " << p << endl;

		lnstream.clear();
		str = "32.2x40000000p";
		lnstream.str(str);
		lnstream >> std::ws >> p;
		cout << "posit format: " << setw(25) << str << "- parsed into a posit<32,2>: " << p << endl;

		lnstream.clear();
		str = "32.2x80000000p";
		lnstream.str(str);
		lnstream >> std::ws >> p;
		cout << "posit format: " << setw(25) << str << "- parsed into a posit<32,2>: " << p << endl;

		lnstream.clear();
		str = "64.3x8000000000000000p";
		lnstream.str(str);
		lnstream >> std::ws >> p;  // TODO: this is truncating the most significant bits, instead of the least significant bits
		cout << "posit format: " << setw(25) << str << "- parsed into a posit<32,2>: " << p << " <---- TODO fix" << endl;
		cout << "pretty posit: " << pretty_print(p) << endl;

		bitblock<1> one; one.set(0, true); str = to_hex(one); cout << "one  : " << str << endl;
		bitblock<2> two; two.set(1, true); str = to_hex(two); cout << "two  : " << str << endl;
		bitblock<3> three; three.set(2, true); str = to_hex(three); cout << "three: " << str << endl;
		bitblock<4> four; four.set(3, true); str = to_hex(four); cout << "four : " << str << endl;

		p.setToZero();
		cout << "posit value     0: " << p << endl;
		p.setToNaR();
		cout << "posit value   NaR: " << p << endl;

        return EXIT_SUCCESS;
    }
    catch (char const* msg) {
        std::cerr << msg << std::endl;
        return EXIT_FAILURE;
    }
	catch (const std::runtime_error& err) {
		std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
		return EXIT_FAILURE;
	}
    catch (...) {
        std::cerr << "Caught unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
