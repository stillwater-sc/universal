// dynamic_range.cpp: demonstration of dynamic ranges for classic cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION 0
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/cfloat/mathlib.hpp>
//#include <universal/verification/test_suite_conversion.hpp>
//#include <universal/verification/cfloat_test_suite.hpp>

/*
b0.00000000.00000000000000000000001 : 1.401298464324817e-45
b0.00000000.00000000000000000000010 : 2.802596928649634e-45
b0.00000000.00000000000000000000100 : 5.605193857299268e-45
b0.00000000.00000000000000000001000 : 1.121038771459854e-44
b0.00000000.00000000000000000010000 : 2.242077542919707e-44
b0.00000000.00000000000000000100000 : 4.484155085839415e-44
b0.00000000.00000000000000001000000 : 8.968310171678829e-44
b0.00000000.00000000000000010000000 : 1.793662034335766e-43
b0.00000000.00000000000000100000000 : 3.587324068671532e-43
b0.00000000.00000000000001000000000 : 7.174648137343063e-43
b0.00000000.00000000000010000000000 : 1.434929627468613e-42
b0.00000000.00000000000100000000000 : 2.869859254937225e-42
b0.00000000.00000000001000000000000 : 5.739718509874451e-42
b0.00000000.00000000010000000000000 : 1.14794370197489e-41
b0.00000000.00000000100000000000000 : 2.29588740394978e-41
b0.00000000.00000001000000000000000 : 4.591774807899561e-41
b0.00000000.00000010000000000000000 : 9.183549615799121e-41
b0.00000000.00000100000000000000000 : 1.836709923159824e-40
b0.00000000.00001000000000000000000 : 3.673419846319648e-40
b0.00000000.00010000000000000000000 : 7.346839692639297e-40
b0.00000000.00100000000000000000000 : 1.469367938527859e-39
b0.00000000.01000000000000000000000 : 2.938735877055719e-39
b0.00000000.10000000000000000000000 : 5.877471754111438e-39
b0.00000001.00000000000000000000000 : 1.175494350822288e-38
b0.00000010.00000000000000000000000 : 2.350988701644575e-38
*/
// float subnormals with the last entry being the smallest normal value
constexpr float ieee754_float_subnormals[24] = {
 1.401298464324817e-45f,
 2.802596928649634e-45f,
 5.605193857299268e-45f,
 1.121038771459854e-44f,
 2.242077542919707e-44f,
 4.484155085839415e-44f,
 8.968310171678829e-44f,
 1.793662034335766e-43f,
 3.587324068671532e-43f,
 7.174648137343063e-43f,
 1.434929627468613e-42f,
 2.869859254937225e-42f,
 5.739718509874451e-42f,
 1.14794370197489e-41f,
 2.29588740394978e-41f,
 4.591774807899561e-41f,
 9.183549615799121e-41f,
 1.836709923159824e-40f,
 3.673419846319648e-40f,
 7.346839692639297e-40f,
 1.469367938527859e-39f,
 2.938735877055719e-39f,
 5.877471754111438e-39f,
 1.175494350822288e-38f     // smallest normal value
};

void GenerateSinglePrecisionSubnormals() 
{
	using namespace sw::universal;
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
    constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;
	cfloat<nbits, es, bt, hasSubnormals, !hasSupernormals, !isSaturating> a;
	++a;
	float f = float(a);
	std::cout << std::setprecision(16);
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(f) << " : " << f << '\n';
	for (int i = 0; i < 24; ++i) {
		f *= 2;
		std::cout << to_binary(f) << " : " << f << '\n';
	}
	for (int i = 0; i < 24; ++i) {
		f = ieee754_float_subnormals[i];
		std::cout << to_binary(f) << " : " << f << '\n';
	}
	std::cout << std::setprecision(5);
}

int main()
try {
	using namespace sw::universal;

	// for any cfloat with es == 1, you must have subnormals and supernormals
	// If you don't have subnormals, your first value would have an 
	// exponent value of 1, which is a supernormal when es == 1.
	std::cout << dynamic_range(cfloat<4, 1, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<5, 1, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<6, 1, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<7, 1, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<8, 1, uint8_t, true, true, false>()) << '\n';

	std::cout << dynamic_range(cfloat<8, 2, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<8, 2, uint8_t, false, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<8, 2, uint8_t, false, false, false>()) << '\n';

	std::cout << dynamic_range(cfloat<16, 5, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<16, 5, uint8_t, false, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<16, 5, uint8_t, false, false, false>()) << '\n';

	std::cout << dynamic_range(cfloat<32, 8, uint8_t, true, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<32, 8, uint8_t, false, true, false>()) << '\n';
	std::cout << dynamic_range(cfloat<32, 8, uint8_t, false, false, false>()) << '\n';

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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
