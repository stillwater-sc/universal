// compute.cpp: experiments with complex real/imaginary computations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <math.h>
#include <stdint.h>
#include <iostream>

#include <cmath>
#include <complex>

#include <universal/native/ieee754.hpp>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>

// Configure the cfloat template environment
// first: enable general or specialized cfloat configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
//#include <universal/number/cfloat/cfloat.hpp>

// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/math/complex/manipulators.hpp>  // to_binary() for complex types


template<typename Scalar>
void TestComplexConjugate(float r = 0.25f, float i = 0.5f) {
	constexpr bool nibbleMarker = true;
	Scalar s{ 0 };
	std::cout << "TestComplexConjugate for type " << sw::universal::type_tag(s) << '\n';
	std::complex<Scalar> c(r, i);
	std::complex<Scalar> cconj(r, -i);
	std::cout << sw::universal::to_binary(c, nibbleMarker) << " : " << c << '\n';
	std::cout << sw::universal::to_binary(cconj, nibbleMarker) << " : " << cconj << '\n';
	std::complex<Scalar> product = c * cconj;
	std::cout << '(' << r << '+' << i << ")*(" << r << '-' << i << ") = " << sw::universal::to_binary(product, nibbleMarker) << " : " << product << '\n';
	std::cout << "----\n";
}

namespace special {
	
	/////////////////////////////////        NATIVE IEEE-754       ////////////////////////////////////////////////  

	bool isnan(std::complex<float> x) { return (std::isnan(x.real()) || std::isnan(x.imag())); }
	bool isinf(std::complex<float> x) { return (std::isinf(x.real()) || std::isinf(x.imag())); }
	std::complex<float> copysign(std::complex<float> x, std::complex<float> y) { return std::complex<float>(std::copysign(x.real(), y.real()), std::copysign(x.real(), y.real())); }

	bool isnan(std::complex<double> x) { return (std::isnan(x.real()) || std::isnan(x.imag())); }
	bool isinf(std::complex<double> x) { return (std::isinf(x.real()) || std::isinf(x.imag())); }
	std::complex<double> copysign(std::complex<double> x, std::complex<double> y) { return std::complex<double>(std::copysign(x.real(), y.real()), std::copysign(x.real(), y.real())); }

	bool isnan(std::complex<long double> x) { return (std::isnan(x.real()) || std::isnan(x.imag())); }
	bool isinf(std::complex<long double> x) { return (std::isinf(x.real()) || std::isinf(x.imag())); }
	std::complex<long double> copysign(std::complex<long double> x, std::complex<long double> y) { return std::complex<long double>(std::copysign(x.real(), y.real()), std::copysign(x.real(), y.real())); }

	/////////////////////////////////            FIXPNT           ////////////////////////////////////////////////  

	template<typename FixedPoint>
	bool isnan(std::complex<FixedPoint> x) {
		return (isnan(x.real()) || isnan(x.imag()));
	}
	template<typename FixedPoint>
	bool isinf(std::complex<FixedPoint> x) {
		return (isinf(x.real()) || isinf(x.imag()));
	}
	template<typename FixedPoint>
	std::complex<FixedPoint> copysign(std::complex<FixedPoint> x, std::complex<FixedPoint> y) {
		return std::complex<FixedPoint>(copysign(x.real(), y.real()), copysign(x.real(), y.real()));
	}
/*
	using fp43sat = sw::universal::fixpnt<4, 3, sw::universal::Saturating, uint8_t>;
	bool isnan(std::complex<fp43sat> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<fp43sat> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<fp43sat> copysign(std::complex<fp43sat> x, std::complex<fp43sat> y) { return std::complex<fp43sat>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	using fp87sat = sw::universal::fixpnt<8, 7, sw::universal::Saturating, uint8_t>;
	bool isnan(std::complex<fp87sat> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<fp87sat> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<fp87sat> copysign(std::complex<fp87sat> x, std::complex<fp87sat> y) { return std::complex<fp87sat>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	using fp1615sat = sw::universal::fixpnt<16, 15, sw::universal::Saturating, uint16_t>;
	bool isnan(std::complex<fp1615sat> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<fp1615sat > x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<fp1615sat> copysign(std::complex<fp1615sat> x, std::complex<fp1615sat > y) { return std::complex<fp1615sat>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	using fp3231sat = sw::universal::fixpnt<32, 31, sw::universal::Saturating, uint32_t>;
	bool isnan(std::complex<fp3231sat> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<fp3231sat> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<fp3231sat> copysign(std::complex<fp3231sat> x, std::complex<fp3231sat> y) { return std::complex<fp3231sat>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	using fp6463sat = sw::universal::fixpnt<64, 63, sw::universal::Saturating, uint32_t>;
	bool isnan(std::complex<fp6463sat> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<fp6463sat> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<fp6463sat> copysign(std::complex<fp6463sat> x, std::complex<fp6463sat> y) { return std::complex<fp6463sat>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }
*/

	/////////////////////////////////            CFLOAT          ////////////////////////////////////////////////   
	
/*  disabling for clang as it is assuming an IEEE-754 type
/Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/usr/include/c++/v1/complex:636:23: error: no matching function for call to 'copysign'
                __d = copysign(_Tp(0), __d);
                      ^~~~~~~~
/Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/usr/include/math.h:512:15: note: candidate function not viable: no known conversion from 'sw::universal::cfloat<8, 3, unsigned char, false, false, false>' to 'double' for 1st argument
extern double copysign(double, double);
              ^
/Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/usr/include/c++/v1/math.h:1102:40: note: candidate function not viable: no known conversion from 'sw::universal::cfloat<8, 3, unsigned char, false, false, false>' to 'float' for 1st argument
inline _LIBCPP_INLINE_VISIBILITY float copysign(float __lcpp_x,
                                       ^
/Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/usr/include/c++/v1/math.h:1107:1: note: candidate function not viable: no known conversion from 'sw::universal::cfloat<8, 3, unsigned char, false, false, false>' to 'long double' for 1st argument
copysign(long double __lcpp_x, long double __lcpp_y) _NOEXCEPT {

	template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
	bool isnan(std::complex<sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) { return (isnan(x.real()) || isnan(x.imag())); }
	template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
	bool isinf(std::complex<sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x) { return (isinf(x.real()) || isinf(x.imag())); }
	template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
	std::complex<sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > copysign(std::complex<sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > x, 
		std::complex<sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > y) {
		return std::complex<sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> > (copysign(x.real(), y.real()), copysign(x.real(), y.real()));
	}

*/

	/////////////////////////////////            POSIT           ////////////////////////////////////////////////   
	bool isnan(std::complex<sw::universal::posit<2, 0>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<2, 0>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<2, 0>> copysign(std::complex<sw::universal::posit<2, 0>> x, std::complex<sw::universal::posit<2, 0>> y) { return std::complex<sw::universal::posit<2, 0>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<3, 0>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<3, 0>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<3, 0>> copysign(std::complex<sw::universal::posit<3, 0>> x, std::complex<sw::universal::posit<3, 0>> y) { return std::complex<sw::universal::posit<3, 0>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<3, 1>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<3, 1>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<3, 1>> copysign(std::complex<sw::universal::posit<3, 1>> x, std::complex<sw::universal::posit<3, 1>> y) { return std::complex<sw::universal::posit<3, 1>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<4, 0>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<4, 0>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<4, 0>> copysign(std::complex<sw::universal::posit<4, 0>> x, std::complex<sw::universal::posit<4, 0>> y) { return std::complex<sw::universal::posit<4, 0>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<8, 0>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<8, 0>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<8, 0>> copysign(std::complex<sw::universal::posit<8, 0>> x, std::complex<sw::universal::posit<8, 0>> y) { return std::complex<sw::universal::posit<8, 0>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<8, 1>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<8, 1>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<8, 1>> copysign(std::complex<sw::universal::posit<8, 1>> x, std::complex<sw::universal::posit<8, 1>> y) { return std::complex<sw::universal::posit<8, 1>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<16, 1>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<16, 1>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<16, 1>> copysign(std::complex<sw::universal::posit<16, 1>> x, std::complex<sw::universal::posit<16, 1>> y) { return std::complex<sw::universal::posit<16, 1>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<32, 2>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<32, 2>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<32, 2>> copysign(std::complex<sw::universal::posit<32, 2>> x, std::complex<sw::universal::posit<32, 2>> y) { return std::complex<sw::universal::posit<32, 2>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<64, 3>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<64, 3>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<64, 3>> copysign(std::complex<sw::universal::posit<64, 3>> x, std::complex<sw::universal::posit<64, 3>> y) { return std::complex<sw::universal::posit<64, 3>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<128, 4>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<128, 4>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<128, 4>> copysign(std::complex<sw::universal::posit<128, 4>> x, std::complex<sw::universal::posit<128, 4>> y) { return std::complex<sw::universal::posit<128, 4>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }

	bool isnan(std::complex<sw::universal::posit<256, 5>> x) { return (isnan(x.real()) || isnan(x.imag())); }
	bool isinf(std::complex<sw::universal::posit<256, 5>> x) { return (isinf(x.real()) || isinf(x.imag())); }
	std::complex<sw::universal::posit<256, 5>> copysign(std::complex<sw::universal::posit<256, 5>> x, std::complex<sw::universal::posit<256, 5>> y) { return std::complex<sw::universal::posit<256, 5>>(copysign(x.real(), y.real()), copysign(x.real(), y.real())); }
}

int main() 
try {
	using namespace std; // needed for the imaginary literals
	using namespace sw::universal;

	{
		// check if imaginary literals compile
		std::complex<double> c = 0.25 + 0.5i;
		std::cout << "complex variable: " << c << '\n';
	}

	std::cout << "----\ntesting complex conjugate operations for different number types\n";
	TestComplexConjugate<float>();
	TestComplexConjugate<fixpnt<8, 4> >();
	//TestComplexConjugate<cfloat<8, 3> >();  // <-- at this small a float you need subnormals when es < 3 to represent 0.25
	TestComplexConjugate<posit<8, 2> >();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
