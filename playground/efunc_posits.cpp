// efunc_posits.cpp: playgound to experiment with the elementary functions on posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <complex>

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	bool bSuccess = true;

	posit<32, 2> x(1.0), y(-1.0625), p(0);
	std::complex< posit<32, 2> > c(x,y),d;

	constexpr unsigned POSIT_COLUMN_WIDTH = 12;
	// sign and absolute value
	bool s;
	s = sign(x); std::cout << "Sign of " << x << " is " << (s ? "true" : "false") << '\n';
	s = sign(y); std::cout << "Sign of " << y << " is " << (s ? "true" : "false") << '\n';
	p =       x; std::cout << "       " << x << "                = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  ulp(x); std::cout << "   ulp(" << x << ")               = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  abs(y); std::cout << "   abs(" << y << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = fabs(y); std::cout << "  fabs(" << y << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';


	// truncation functions
	x = 1.50001;
	p =  ceil(x); std::cout << " ceil(" << x << ")          = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = floor(x); std::cout << "floor(" << x << ")          = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = round(x); std::cout << "round(" << x << ")          = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = trunc(x); std::cout << "trunc(" << x << ")          = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	// complex functions
	p =  real(c); std::cout << " real(" << c << ")      = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  imag(c); std::cout << " imag(" << c << ")      = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	d =  conj(c); std::cout << " conj(" << c << ")      = " << std::setw(POSIT_COLUMN_WIDTH) << d << " " << color_print(d.real()) << "," << color_print(d.imag()) << '\n';
	
	// sqrt and inverse sqrt
	x = d_pi_4;
	p =  sqrt(x); std::cout << " sqrt(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = rsqrt(x); std::cout << "rsqrt(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	// trigonometric functions
	p =   sin(x); std::cout << "  sin(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  asin(x); std::cout << " asin(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =   cos(x); std::cout << "  cos(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  acos(x); std::cout << " acos(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =   tan(x); std::cout << "  tan(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  atan(x); std::cout << " atan(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	// hyperbolic functions
	p =  sinh(x); std::cout << " sinh(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = asinh(x); std::cout << "asinh(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  cosh(x); std::cout << " cosh(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = acosh(x); std::cout << "acosh(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  tanh(x); std::cout << " tanh(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = atanh(x); std::cout << "atanh(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	// error and gamma functions
	p =  erf(x); std::cout << "  erf(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = erfc(x); std::cout << " erfc(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	// exponential and logarithmic functions

	p =   exp(x); std::cout << "  exp(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  exp2(x); std::cout << " exp2(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = exp10(x); std::cout << "exp10(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =   log(x); std::cout << "  log(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p =  log2(x); std::cout << " log2(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = log10(x); std::cout << "log10(" << x << ")         = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	p = pow(x, y); std::cout << "  pow(" << x << "," << y << ") = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	p = min(x, y); std::cout << "  min(" << x << "," << y << ") = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';
	p = max(x, y); std::cout << "  max(" << x << "," << y << ") = " << std::setw(POSIT_COLUMN_WIDTH) << p << " " << color_print(p) << '\n';

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
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
