// lns.cpp: components of a logarithmic number: cli to show the sign/scale/fraction components of a logarithmic number 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/lns/lns.hpp>

template<typename LNS>
void PrintLnsEncodings(float f) {
	using namespace sw::universal;
	LNS v{f};

	constexpr size_t columnWidth = 50;
	std::cout << std::setw(columnWidth) << std::left << type_tag(LNS()) << ": " << std::setprecision(std::numeric_limits<LNS>::max_digits10) << std::right << v << '\n';

	//std::cout << "triple form  : " << to_triple(v) << '\n';   // this is the 'right' way this needs to work
	std::cout << "triple form  : " << to_triple(double(v)) << '\n';   // this is a short cut marshalling through a double
	std::cout << "binary form  : " << to_binary(v, true) << '\n';
	std::cout << "color coded  : " << color_print(v) << '\n';
}

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// lns attributes
	constexpr int max_digits10 = std::numeric_limits<double>::max_digits10;

	if (argc != 2) {
		std::cerr << "lns : components of a logarithmic number\n";
		std::cerr << "Show the sign/scale/fraction components of a logarithmic number.\n";
		std::cerr << "Usage: lns float_value\n";
		std::cerr << "Example: lns 0.03124999\n";
		std::cerr << "TBD" << '\n';

		std::cout << std::numeric_limits<lns<4, 1>>::min_exponent << '\n';

		std::cout << "Number Traits of logarithmic number systems\n";
		numberTraits< lns< 4, 1> >(std::cout);
		std::cout << '\n';
		numberTraits< lns< 8, 4> >(std::cout);
		std::cout << '\n';
		numberTraits< lns<12, 6> >(std::cout);
		std::cout << '\n';
		numberTraits< lns<16, 8> >(std::cout);
		std::cout << '\n';
		numberTraits< lns<20,10> >(std::cout);
		std::cout << '\n';
		numberTraits< lns<24,12> >(std::cout);
		std::cout << '\n';
		numberTraits< lns<32, 23> >(std::cout);
		std::cout << '\n';

		std::cout << "float reference\n";
		numberTraits< float >(std::cout);

		return EXIT_SUCCESS;  // signal successful completion for ctest
	}

	float f = float(atof(argv[1]));
	std::cout << "float value  : " << std::setprecision(max_digits10) << f << '\n';

	PrintLnsEncodings< lns<6, 2, std::uint32_t> >(f);
	PrintLnsEncodings< lns<6, 3, std::uint32_t> >(f);
	PrintLnsEncodings< lns<6, 4, std::uint32_t> >(f);

	std::cout << '\n';

	PrintLnsEncodings< lns<8, 2, std::uint32_t> >(f);
	PrintLnsEncodings< lns<8, 4, std::uint32_t> >(f);
	PrintLnsEncodings< lns<8, 6, std::uint32_t> >(f);

	std::cout << '\n';

	PrintLnsEncodings< lns<10, 3, std::uint32_t> >(f);
	PrintLnsEncodings< lns<10, 5, std::uint32_t> >(f);
	PrintLnsEncodings< lns<10, 8, std::uint32_t> >(f);

	std::cout << '\n';

	PrintLnsEncodings< lns<12, 4, std::uint32_t> >(f);
	PrintLnsEncodings< lns<12, 6, std::uint32_t> >(f);
	PrintLnsEncodings< lns<12, 9, std::uint32_t> >(f);

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
