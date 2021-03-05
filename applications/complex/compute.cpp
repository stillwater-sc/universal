// compute.cpp: experiments with complex real/imaginary computations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <math.h>
#include <stdint.h>
#include <iostream>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt>
#include <universal/number/areal/areal.hpp>
// Configure the bfloat template environment
// first: enable general or specialized bfloat configurations
#define BFLOAT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/bfloat/bfloat>
// Configure the fixpnt template environment
// first: enable general or specialized posit configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>

namespace special {
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

// conditional compilation
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main() 
try {
	using namespace std;
	using namespace sw::universal;

#if MANUAL_TESTING

	{
		using Scalar = posit<32,2>;

		Scalar p;
	}

	{
		using Scalar = fixpnt<4, 3>;
		Scalar fp{ 1.0f };
		cout << to_binary(fp) << " : " << fp << endl;
	}
	
#else // MANUAL_TESTING


#if STRESS_TESTING

#endif // STRESS_TESTING
#endif // MANUAL_TESTING


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
