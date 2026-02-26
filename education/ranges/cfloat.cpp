// cfloat.cpp : enumeration of dynamic ranges of different cfloat configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>

// print fixed-point ranges
int main()
try {
	using namespace sw::universal;

	constexpr bool noSubnormals = false;
	constexpr bool hasSubnormals = true;
	constexpr bool noSupernormals = false;
	constexpr bool hasMaxExpValues = true;
	// value ranges of interesting fixed-point configurations

	// standard format floats as specific cfloat configurations
	std::cout << "Standard format floats\n";
	std::cout << "quarter precision: " << cfloat_range<cfloat<  8, 2, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << "half    precision: " << cfloat_range<cfloat< 16, 5, uint16_t, hasSubnormals> >() << '\n';
	std::cout << "bfloat16         : " << cfloat_range<cfloat< 16, 8, uint16_t> >() << '\n';
	std::cout << "TensorFloat      : " << cfloat_range<cfloat< 19, 8, uint32_t> >() << '\n';
	std::cout << "3/4th   precision: " << cfloat_range<cfloat< 24, 8, uint32_t> >() << '\n';
	std::cout << "single  precision: " << cfloat_range<cfloat< 32, 8, uint32_t> >() << '\n';
	std::cout << "double  precision: " << cfloat_range<cfloat< 64,11, uint64_t> >() << '\n';
	std::cout.flush();
	std::cout << "quad    precision: " << cfloat_range<cfloat<128,15, uint32_t> >() << '\n';
	std::cout << "octa    precision: " << cfloat_range<cfloat<256,20, uint32_t> >() << '\n';
	std::cout << '\n';

	// small cfloats to understand their range
	std::cout << "Small cfloats without subnormals and max-exponent values\n";
	std::cout << cfloat_range<cfloat< 8, 2, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 8, 3, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 8, 4, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 9, 2, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 9, 3, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 9, 4, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat<10, 2, uint16_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat<10, 3, uint16_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat<10, 4, uint16_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat<12, 2, uint16_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat<12, 3, uint16_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat<12, 4, uint16_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << '\n';
	std::cout << "Small cfloats with subnormals and max-exponent values\n";
	std::cout << cfloat_range<cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat< 9, 2, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat< 9, 3, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat< 9, 4, uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat<10, 2, uint16_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat<10, 3, uint16_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat<10, 4, uint16_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat<12, 2, uint16_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat<12, 3, uint16_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << cfloat_range<cfloat<12, 4, uint16_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	std::cout << '\n';

	// medium cfloats to understand their range
	std::cout << "Medium simplified cfloats without subnormals and max-exponent values\n";
	std::cout << cfloat_range<cfloat< 16, 8, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 24, 8, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 32, 8, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 40, 8, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 48, 8, uint8_t, noSubnormals, noSupernormals> >() << '\n';
	std::cout << '\n';

	// large cfloats to understand their range
	std::cout << "Large precise cfloats without subnormals and max-exponent values\n";
	std::cout << cfloat_range<cfloat< 64,11, uint8_t, hasSubnormals, hasSubnormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 80,11, uint8_t, hasSubnormals, hasSubnormals> >() << '\n';
	std::cout << cfloat_range<cfloat< 96,11, uint8_t, hasSubnormals, hasSubnormals> >() << '\n';
	std::cout << cfloat_range<cfloat<112,11, uint8_t, hasSubnormals, hasSubnormals> >() << '\n';
	std::cout << cfloat_range<cfloat<128,11, uint8_t, hasSubnormals, hasSubnormals> >() << '\n';

	std::cout.flush();
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
