// serialization.cpp example testing cricial features for G+Smo integration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// enable special posit format emission
#define POSIT_ERROR_FREE_IO_FORMAT 1
#include <universal/number/posit1/posit1.hpp>

int main(int argc, char** argv)
try {
    using namespace sw::universal;

	// Test reading posit from std::istringstream
    posit<32,2> p;

    std::string str = "3.1415926535897932384626433832795028841971693993751058209749445923078164062";
    std::istringstream lnstream;
    lnstream.unsetf(std::ios_base::skipws);
    lnstream.clear();
    lnstream.str(str);                       
    lnstream >> std::ws >> p;
	std::cout << "IEEE float/double format, parsed into a posit<32,2>: " << p << '\n';

	lnstream.clear();
	str = "32.2x40000000p";
	lnstream.str(str);
	lnstream >> std::ws >> p;
	std::cout << "posit format: " << std::setw(25) << str << "- parsed into a posit<32,2>: " << p << '\n';

	lnstream.clear();
	str = "32.2x80000000p";
	lnstream.str(str);
	lnstream >> std::ws >> p;
	std::cout << "posit format: " << std::setw(25) << str << "- parsed into a posit<32,2>: " << p << '\n';

	lnstream.clear();
	str = "64.3x8000000000000000p";
	lnstream.str(str);
	lnstream >> std::ws >> p;  // testing that we are NOT truncating the most significant bits
	std::cout << "posit format: " << std::setw(25) << str << "- parsed into a posit<32,2>: " << p << " <---- should have the most significant 32bits of the 64.3 posit\n";
	std::cout << "pretty posit: " << pretty_print(p) << '\n';

	std::cout << "Bitblock patterns\n";
	bitblock<1> one; one.set(0, true); str = to_hex(one); std::cout << "one  : \"" << one << "\"    value : " << str << '\n';
	bitblock<2> two; two.set(1, true); str = to_hex(two); std::cout << "two  : \"" << two << "\"   value : " << str << '\n';
	bitblock<3> three; three.set(2, true); str = to_hex(three); std::cout << "three: \"" << three << "\"  value : " << str << '\n';
	bitblock<4> four; four.set(3, true); str = to_hex(four); std::cout << "four : \"" << four << "\" value : " << str << '\n';

	p.setzero();
	std::cout << "posit value     0: " << p << '\n';
	p.setnar();
	std::cout << "posit value   NaR: " << p << '\n';

    return EXIT_SUCCESS;
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
