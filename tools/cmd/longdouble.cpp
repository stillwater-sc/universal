// longdouble.cpp: components of a long double: cli to show the sign/scale/fraction components of a long double native IEEE float
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/native/ieee754.hpp>
#include <universal/common/number_traits_reports.hpp>

// ShowRepresentations prints the different output formats for the long double type
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, long double f) {
	using namespace sw::universal;
	auto oldprec = ostr.precision(); // save stream state

	constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10; 	// floating-point attribute for printing scientific format

	Scalar v(f); // convert to target cfloat
	ostr << "scientific   : " << std::setprecision(max_digits10) << v << '\n';
	ostr << "triple form  : " << to_triple(v) << '\n';
	ostr << "binary form  : " << to_binary(v, true) << '\n';
	ostr << "color coded  : " << color_print(v) << '\n';

	ostr << std::setprecision(oldprec);
}

// receive a float and print the components of a long double representation
int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using Scalar = long double;

	if (argc != 2) {
		std::cerr << "longdouble: components of an IEEE long-double (compiler dependent, 80-bit extended precision on x86 and ARM, 128-bit on RISC-V\n";
		std::cerr << "Show the sign/scale/fraction components of an IEEE long double.\n";
		std::cerr << "Usage: longdouble long_double_value\n";
		std::cerr << "Example: longdouble 0.03124999\n";
		ShowRepresentations<Scalar>(std::cerr, 0.03124999);

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
	ShowRepresentations<Scalar>(std::cout, q);

	std::cout.flush();
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
