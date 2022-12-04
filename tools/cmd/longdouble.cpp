// longdouble.cpp: components of a long double: cli to show the sign/scale/fraction components of a long double native IEEE float
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/native/ieee754.hpp>
#include <universal/common/number_traits_reports.hpp>
#include <universal/internal/value/value>

// receive a float and print the components of a long double representation
int main(int argc, char** argv)
try {
	using namespace sw::universal::internal;

	// long double attributes
	constexpr int max_digits10 = std::numeric_limits<long double>::max_digits10;
	constexpr int fbits = std::numeric_limits<long double>::digits - 1;

	if (argc != 2) {
		std::cerr << "longdouble: components of an IEEE long-double (compiler dependent, 80-bit extended precision on x86 and ARM, 128-bit on RISC-V\n";
		std::cerr << "Show the sign/scale/fraction components of an IEEE long double.\n";
		std::cerr << "Usage: longdouble long_double_value\n";
		std::cerr << "Example: longdouble 0.03124999\n";
		std::cerr << "long double: 0.0312499899999999983247 (+,-6,000000000000000000000000000000000011111111111110000000000000000)\n\n";
		
		using Scalar = long double;

		std::cout << "Number Traits of IEEE-754 long double\n";
		numberTraits<Scalar>(std::cout);

		std::cout << "smallest normal number\n";
		std::cout << to_binary(std::numeric_limits<long double>::min()) << '\n';
		std::cout << "smallest denormalized number\n";
		std::cout << to_binary(std::numeric_limits<long double>::denorm_min()) << '\n';

		std::cout << '\n';
		std::cout << "Universal parameterization of IEEE-754 fields\n";
		std::cout << ieee754_parameter<Scalar>() << '\n';
		std::cout.flush();
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	long double q = strtold(argv[1], NULL);
	value<fbits> v(q);

	std::cout << "long double: " << std::setprecision(max_digits10) << q << '\n';
	std::cout << "triple form  : " << to_triple(q) << '\n';
	std::cout << "binary form  : " << to_binary(q, true) << '\n';
	std::cout << "color coded  : " << color_print(q) << '\n';

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
