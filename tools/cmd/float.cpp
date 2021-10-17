// float.cpp: components of a float: cli to show the sign/scale/fraction components of a float 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/native/ieee754.hpp>
#include <universal/common/numeric_limits_utility.hpp>
#include <universal/internal/value/value>

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal::internal;

	// float attributes
	constexpr int max_digits10 = std::numeric_limits<double>::max_digits10;
	constexpr int fbits = std::numeric_limits<float>::digits - 1;

	if (argc != 2) {
		std::cerr << "float : components of an IEEE single-precision float\n";
		std::cerr << "Show the sign/scale/fraction components of an IEEE float.\n";
		std::cerr << "Usage: float float_value\n";
		std::cerr << "Example: float 0.03124999\n";
		std::cerr << "float: 0.031249990686774254 (+,-6,11111111111111111111011)\n\n";

		using Scalar = float;

		std::cout << "Number Traits of IEEE-754 float\n";
		numberTraits<Scalar>(std::cout);

		std::cout << "smallest normal number\n";
		std::cout << to_binary(std::numeric_limits<float>::min()) << '\n';
		std::cout << "smallest denormalized number\n";
		std::cout << to_binary(std::numeric_limits<float>::denorm_min()) << '\n';

		std::cout << '\n';
		std::cout << "Universal parameterization of IEEE-754 fields\n";
		std::cout << ieee754_parameter<Scalar>() << '\n';
		std::cout.flush();
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	float f = float(atof(argv[1]));
	value<fbits> v(f);

	std::cout << "float value  : " << std::setprecision(max_digits10) << f << '\n';
	std::cout << "triple form  : " << to_triple(f) << '\n';
	std::cout << "binary form  : " << to_binary(f, true) << '\n';
	std::cout << "color coded  : " << color_print(f) << '\n';

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
