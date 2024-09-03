// doubledouble.cpp: components of a double-double: cli to show the sign/scale/limb components of a double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#include <universal/number/dd/dd.hpp>
#include <universal/common/number_traits_reports.hpp>

// ShowRepresentations prints the different output formats for the long double type
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, const Scalar& f) {
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
	using Scalar = dd;

	if (argc != 2) {
		std::cerr << "doubledouble: components of a double-double floating-point\n";
		std::cerr << "Show the sign/scale/fraction components of an double-double.\n";
		std::cerr << "Usage: doubledouble fp_value_string\n";
		std::cerr << "Example: doubledouble 0.03124999\n";
		ShowRepresentations<Scalar>(std::cerr, 0.03124999);

		std::cout << "Number Traits of a double-double\n";
		numberTraits<Scalar>(std::cout);

		std::cout << "largest normal number\n";
		std::cout << to_binary(std::numeric_limits<Scalar>::max()) << '\n';
		std::cout << "smallest normal number\n";
		std::cout << to_binary(std::numeric_limits<Scalar>::min()) << '\n';
		std::cout << "smallest denormalized number\n";
		std::cout << to_binary(std::numeric_limits<Scalar>::denorm_min()) << '\n';

		constexpr Scalar epsilon{ std::numeric_limits< Scalar >::epsilon() };
		std::cout << "epsilon : " << epsilon << '\n';
		std::cout << to_binary(epsilon) << '\n';

		std::cout.flush();
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}

	dd doubledouble(argv[1]);
	ShowRepresentations<Scalar>(std::cout, doubledouble);

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
