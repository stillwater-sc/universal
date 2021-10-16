// double.cpp: components of a double: cli to show the sign/scale/fraction components of a double
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

// receive a float and print the components of a double representation
int main(int argc, char** argv)
try {
	using namespace sw::universal::internal;

	// double attributes
	constexpr int max_digits10 = std::numeric_limits<double>::max_digits10;
	constexpr int fbits = std::numeric_limits<double>::digits - 1;

	if (argc != 2) {
		std::cerr << "double : components of an IEEE double-precision float\n";
		std::cerr << "Show the sign/scale/fraction components of an IEEE double.\n";
		std::cerr << "Usage: double double_value\n";
		std::cerr << "Example: double 0.03124999\n";
		std::cerr << "double: 0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)\n";
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	double d = atof(argv[1]);
	value<fbits> v(d);

	std::cout << "double: " << std::setprecision(max_digits10) << d << " " << to_triple(v) << '\n';

	using Scalar = double;

	std::cout << "Universal parameterization of IEEE-754 fields\n";
	std::cout << ieee754_parameter<Scalar>() << '\n';

	std::cout << "Number Traits of IEEE-754 double\n";
	numberTraits<Scalar>(std::cout);
	
	std::cout << "smallest normal number\n";
	std::cout << to_binary(std::numeric_limits<double>::min()) << '\n';
	std::cout << "smallest denormalized number\n";
	std::cout << to_binary(std::numeric_limits<double>::denorm_min()) << '\n';

	std::cout.flush();
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
