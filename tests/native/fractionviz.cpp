//  fractionviz.cpp : fraction bits visualization of native IEEE-754 types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <cmath>
#include <cfenv>
#pragma STDC_FENV_ACCESS on
#include <limits>
//#include <universal/native/integers.hpp>
#include <universal/native/ieee754.hpp>

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

// TODO: make it work for long double
// that special bit[63] is a kicker
//                         16  15   14   13   12   11   10    9    8    7    6    5    4    3    2    1
// 0b0.111'1111'1111'1111.x100'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
// 0b0.000'0000'0000'0000.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000

// Convert a uint64_t mask into an IEEE-754 field-aligned bit pattern
std::string MaskToIeee754(size_t mask, unsigned nbits, unsigned es) {
	std::stringstream s;
	size_t bit = (1ull << (nbits - 1));
	s << (bit & mask ? "0b1." : "0b0.");
	bit >>= 1;
	for (unsigned i = 0; i < es; ++i) {
		s << (bit & mask ? '1' : '0');
		bit >>= 1;
	}
	s << '.';
	for (size_t i = 0; i < nbits - es - 1; ++i) {
		s << (bit & mask ? '1' : '0');
		bit >>= 1;
	}
	return s.str();
}

template<typename NativeReal,
		 typename = typename std::enable_if< std::is_floating_point<NativeReal>::value, NativeReal>::type
>
void CheckQuietNaN() {
	if (std::numeric_limits<NativeReal>::has_quiet_NaN) {
		NativeReal f = std::numeric_limits<NativeReal>::quiet_NaN();
		std::cout << typeid(NativeReal).name() << " has a quiet NaN encoding : \n";
		std::cout << sw::universal::to_binary(f) << std::endl;
		std::cout << MaskToIeee754(
			sw::universal::ieee754_parameter<NativeReal>::qnanmask,
			sw::universal::ieee754_parameter<NativeReal>::nbits,
			sw::universal::ieee754_parameter<NativeReal>::ebits) << std::endl;
	}
	else {
		std::cout << typeid(NativeReal).name() << " does not have a quiet NaN encoding\n";
	}
}

template<typename NativeReal,
	typename = typename std::enable_if< std::is_floating_point<NativeReal>::value, NativeReal>::type
>
void CheckSignallingNaN() {
	if (std::numeric_limits<NativeReal>::has_signaling_NaN) {
		NativeReal f = std::numeric_limits<NativeReal>::signaling_NaN();
		std::cout << typeid(NativeReal).name() << " has a signalling NaN encoding : \n";
		std::cout << sw::universal::to_binary(f) << std::endl;
		std::cout << MaskToIeee754(
			sw::universal::ieee754_parameter<NativeReal>::snanmask,
			sw::universal::ieee754_parameter<NativeReal>::nbits,
			sw::universal::ieee754_parameter<NativeReal>::ebits) << std::endl;
	}
	else {
		std::cout << typeid(NativeReal).name() << " does not have a signalling NaN encoding\n";
	}
}

void show_fe_exceptions()
{
	int n = std::fetestexcept(FE_ALL_EXCEPT);
	if (n & FE_INVALID) std::cout << "FE_INVALID is raised\n";
	else if (n == 0)    std::cout << "no exceptions are raised\n";
	std::feclearexcept(FE_ALL_EXCEPT);
}

int main()
try {
	using namespace sw::universal;

	// compare bits of different real number representations
	
	float f         = 1.0e10;
	double d        = 1.0e10;
	long double ld  = 1.0e10;

	std::cout << "single precision float     : " << color_print(f) << '\n';
	std::cout << "double precision float     : " << color_print(d) << '\n';
	std::cout << "long double precision float: " << color_print(ld) << '\n';

	// special values
	f = nan("NaN");
	d = nanf("NaN");
	ld = nanl("NaN");

	CheckQuietNaN<float>();
	CheckQuietNaN<double>();
	CheckQuietNaN<long double>();

	CheckSignallingNaN<float>();
	CheckSignallingNaN<double>();
	CheckSignallingNaN<long double>();

	double snan = std::numeric_limits<double>::signaling_NaN();
	std::cout << "After sNaN was obtained ";
	show_fe_exceptions();
	double qnan = snan * 2.0;
	std::cout << "After sNaN was multiplied by 2 ";
	show_fe_exceptions();
	double qnan2 = qnan * 2.0;
	std::cout << "After the quieted NaN was multiplied by 2 ";
	show_fe_exceptions();
	std::cout << "The result is " << qnan2 << '\n';

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
