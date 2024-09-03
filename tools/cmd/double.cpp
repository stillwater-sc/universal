// double.cpp: components of a double: cli to show the sign/scale/fraction components of a double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/common/number_traits_reports.hpp>

// ShowRepresentations prints the different output formats for the double type
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, double f) {
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

// receive a float and print the components of a double representation
int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using Scalar = double;

	if (argc != 2) {
		std::cerr << "double : components of an IEEE double-precision float\n";
		std::cerr << "Show the sign/scale/fraction components of an IEEE double.\n";
		std::cerr << "Usage: double double_value\n";
		std::cerr << "Example: double 0.03124999\n";
		ShowRepresentations<Scalar>(std::cerr, 0.03124999);

		std::cerr << "Number Traits of IEEE-754 double\n";
		numberTraits<Scalar>(std::cout);

		std::cerr << "smallest normal number\n";
		std::cerr << to_binary(std::numeric_limits<Scalar>::min()) << '\n';
		std::cerr << "smallest denormalized number\n";
		std::cerr << to_binary(std::numeric_limits<Scalar>::denorm_min()) << '\n';

		std::cout << '\n';
		std::cerr << "Universal parameterization of IEEE-754 fields\n";
		std::cerr << ieee754_parameter<Scalar>() << '\n';
		std::cerr.flush();

		return EXIT_SUCCESS;   // signal successful completion for ctest
	}

	double d = atof(argv[1]);
	ShowRepresentations<Scalar>(std::cout, d);

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
