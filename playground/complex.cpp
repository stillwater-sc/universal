// complex.cpp: complex library replacement for custom types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

template <class _A1>
inline
constexpr typename std::enable_if<sw::universal::is_posit<_A1>, bool>::type
__constexpr_isnan(_A1 __lcpp_x) 
{
    return sw::universal::isnan(__lcpp_x);
}
#include <complex>

constexpr unsigned COLUMN_WIDTH = 12;

int main()
try {
    
    using namespace sw::universal;
    
    report_compiler();
    
    
    posit<32, 2> x(1.0), y(-1.0625), p(0);
	std::complex< posit<32, 2> > c(x, y), i(0,1), d, e(x,y), f(-x, -y);

    // complex functions
    p = real(c); std::cout << " real(" << c << ")      = " << std::setw(COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
    p = imag(c); std::cout << " imag(" << c << ")      = " << std::setw(COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
    d = conj(c); std::cout << " conj(" << c << ")      = " << std::setw(COLUMN_WIDTH) << d << " " << color_print(d.real()) << "," << color_print(d.imag()) << '\n';

	std::cout << std::setw(COLUMN_WIDTH) << e << " " << color_print(e.real()) << "," << color_print(e.imag()) << '\n';
	std::cout << std::setw(COLUMN_WIDTH) << f << " " << color_print(f.real()) << "," << color_print(f.imag()) << '\n';

	d = e + f; std::cout << std::setw(COLUMN_WIDTH) << d << " " << color_print(d.real()) << "," << color_print(d.imag()) << '\n';
	d = e * i; std::cout << std::setw(COLUMN_WIDTH) << d << " " << color_print(d.real()) << "," << color_print(d.imag()) << '\n';
    

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
