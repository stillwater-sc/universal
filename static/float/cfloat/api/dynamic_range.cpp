// dynamic_range.cpp: demonstration of dynamic ranges for classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#include <universal/number/cfloat/cfloat.hpp>

void GenerateSinglePrecisionSubnormals() 
{
	using namespace sw::universal;
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
    constexpr bool hasMaxExpValues = true;
	constexpr bool isSaturating = true;
	cfloat<nbits, es, bt, hasSubnormals, !hasMaxExpValues, !isSaturating> a{};
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

	// for any cfloat with es == 1, you must have subnormals and max-exponent values
	// If you don't have subnormals, your first value would have an 
	// exponent value of 1, which is a max-exponent value when es == 1.
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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
