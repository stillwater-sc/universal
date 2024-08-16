// quaddouble.cpp: components of a quad-double: cli to show the sign/scale/limb components of a quad-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#include <universal/number/qd/qd.hpp>
#include <universal/common/number_traits_reports.hpp>

// ShowRepresentations prints the different output formats for the quad-double type
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, sw::universal::qd f) {
	using namespace sw::universal;
	auto defaultPrecision = ostr.precision(); // save stream state

	constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10; 	// floating-point attribute for printing scientific format

	Scalar v(f); // convert to target cfloat
	ostr << "scientific   : " << std::setprecision(max_digits10) << v << '\n';
	ostr << "triple form  : " << to_triple(v) << '\n';
	ostr << "binary form  : " << '\n' << to_binary(v, true) << '\n';
	ostr << "color coded  : " << '\n' << color_print(v, true) << '\n';

	ostr << std::setprecision(defaultPrecision);
}

/*
  quad-double numbers are an unevaluated set of four doubles.
  Each double-precision segment has an epsilon of approximately 2^53.
  Combining four double-precision numbers gives as a precision of roughly 4 times 53 bits, or 212 bits.
  Therefore, the epsilon of a quad-double number is approximately 2^212/2 = 2^211
  2^211 = 3.2910091146424120843099383651147e+63 ~ 3.29100911e63
*/

// receive a float and print the components of a long double representation
int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using Scalar = qd;

	if (argc != 2) {
		std::cerr << "quaddouble: components of a quad-double floating-point\n";
		std::cerr << "Show the sign/scale/limbs components of a quad-double.\n";
		std::cerr << "Usage: quaddouble fp_value_string\n";
		std::cerr << "Example: quaddouble 0.03124999\n";
		ShowRepresentations<Scalar>(std::cerr, 0.03124999);

		std::cout << "Number Traits of quad-double\n";
		numberTraits<Scalar>(std::cout);

		std::cout << "largest normal number\n";
		std::cout << to_binary(std::numeric_limits<Scalar>::max()) << '\n';
		std::cout << "smallest normal number\n";
		std::cout << to_binary(std::numeric_limits<Scalar>::min()) << '\n';
		std::cout << "smallest denormalized number\n";
		std::cout << to_binary(std::numeric_limits<Scalar>::denorm_min()) << '\n';

		Scalar epsilon{ std::numeric_limits< Scalar >::epsilon() };
		std::cout << "epsilon : " << epsilon << '\n';
		std::cout << to_binary(epsilon) << '\n';
		std::cout.flush();
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}

	qd q(argv[1]);
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
