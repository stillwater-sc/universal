// efunc_valids.cpp: playgound to experiment with the elementary math functions on valids
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <complex>

// enable valid arithmetic exceptions
#define VALID_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/valid/valid.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	bool bSuccess = true;

	std::cout << "valid elementary functions not implemented yet\n";
	/*
	valid<32, 2> x(1.0), y(-1.0), v(0.0);
	std::complex< valid<32, 2> > c(x,y),d;

	// sign and absolute value
	bool s;
	s = sign(x); std::cout << "Sign of " << x << " is " << s << '\n';
	s = sign(y); std::cout << "Sign of " << y << " is " << s << '\n';
	v = abs(y); std::cout << "   abs(" << y << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = fabs(y); std::cout << "  fabs(" << y << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';

	// truncation functions
	x = 1.50001;
	v = ceil(x); std::cout << " ceil(" << x << ")     = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = floor(x); std::cout << "floor(" << x << ")     = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = round(x); std::cout << "round(" << x << ")     = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = trunc(x); std::cout << "trunc(" << x << ")     = " << std::setw(10) << v << " " << color_print(v) << '\n';

	// complex functions
	v = real(c); std::cout << " real(" << c << ") = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = imag(c); std::cout << " imag(" << c << ") = " << std::setw(10) << v << " " << color_print(v) << '\n';
	d = conj(c); std::cout << " conj(" << c << ") = " << std::setw(10) << d << " " << color_print(d.real()) << "," << color_print(d.imag()) << '\n';

	// sqrt and inverse sqrt
	x = m_pi_4;
	v =  sqrt(x); std::cout << " sqrt(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = rsqrt(x); std::cout << "rsqrt(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';

	// trigonometric functions
	v =  sin(x); std::cout << "  sin(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = asin(x); std::cout << " asin(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =  cos(x); std::cout << "  cos(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = acos(x); std::cout << " acos(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =  tan(x); std::cout << "  tan(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = atan(x); std::cout << " atan(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';

	// hyperbolic functions
	v =  sinh(x); std::cout << " sinh(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = asinh(x); std::cout << "asinh(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =  cosh(x); std::cout << " cosh(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = acosh(x); std::cout << "acosh(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =  tanh(x); std::cout << " tanh(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = atanh(x); std::cout << "atanh(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';

	// error and gamma functions
	v =  erf(x); std::cout << "  erf(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = erfc(x); std::cout << " erfc(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';

	// exponential and logarithmic functions

	v =   exp(x); std::cout << "  exp(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =  exp2(x); std::cout << " exp2(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = exp10(x); std::cout << "exp10(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =   log(x); std::cout << "  log(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v =  log2(x); std::cout << " log2(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';
	v = log10(x); std::cout << "log10(" << x << ")    = " << std::setw(10) << v << " " << color_print(v) << '\n';

	v = pow(x, y); std::cout << "  pow(" << x << "," << y << ") = " << std::setw(10) << v << " " << color_print(v) << '\n';
	*/

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::valid_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
// TODO: enable quire exception trap for valids
//catch (const sw::universal::quire_exception& err) {
//	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
//	return EXIT_FAILURE;
//}
catch (const sw::universal::valid_internal_exception& err) {
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
