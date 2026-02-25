// subnormals.cpp: showcase for cfloat subnormals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION 0
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>

int main()
try {
	using namespace sw::universal;

	// generate individual testcases to hand trace/debug
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;

	// case when the cfloat doesn't have subnormals
	subnormals<cfloat<8, 2, uint8_t, !hasSubnormals, hasSupernormals, isSaturating>>();  // 1 block

	// generate subnormals for different interesting cfloat configurations
	subnormals<cfloat<  8,  2, uint8_t , hasSubnormals, hasSupernormals, isSaturating>>(); // 1 block
	subnormals<cfloat< 16,  5, uint8_t , hasSubnormals, hasSupernormals, isSaturating>>(); // 2 blocks
	subnormals<cfloat< 32,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating>>(); // 4 blocks
	subnormals<cfloat< 40,  9, uint8_t , hasSubnormals, hasSupernormals, isSaturating>>(); // 5 blocks
	subnormals<cfloat< 48, 10, uint16_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 3 blocks
	subnormals<cfloat< 48, 11, uint16_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 3 blocks
	subnormals<cfloat< 56, 11, uint8_t , hasSubnormals, hasSupernormals, isSaturating>>(); // 7 blocks
	subnormals<cfloat< 56, 11, uint32_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 2 blocks
	subnormals<cfloat< 64, 11, uint16_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 4 blocks
	subnormals<cfloat< 80, 15, uint16_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 5 blocks
	subnormals<cfloat< 96, 15, uint32_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 3 blocks
	subnormals<cfloat<112, 15, uint32_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 3 blocks
	subnormals<cfloat<128, 15, uint32_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 4 blocks
	subnormals<cfloat<256, 19, uint32_t, hasSubnormals, hasSupernormals, isSaturating>>(); // 8 blocks

	// TODO: generate a subnormal bit pattern through seed and multiplication
	{
		constexpr size_t nbits = 28;
		constexpr size_t es = 8;
		using bt = uint32_t;
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{ 0 }, b;
		++a;
		for (int i = 0; i < 19; ++i) {
			float f = float(a);
			b = f;
			std::cout << to_binary(f) << " : " << color_print(f) << " : " << f << '\n';
			std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
			std::cout << to_binary(b) << " : " << color_print(b) << " : " << b << '\n';
			// when we have mul
			// a *= 2.0f;
			uint64_t fraction = a.fraction_ull();
			fraction <<= 1;
			a.setfraction(fraction);
		}

		a = 1.0e25f;
		std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
	}


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
