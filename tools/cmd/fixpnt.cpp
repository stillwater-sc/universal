// fixpnt.cpp: components of a fixed-point: cli to show the sign/scale/fraction components of a fixed-point value 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <universal/number/fixpnt/fixpnt.hpp>

// ShowRepresentations prints the different output formats for the Scalar type
// TODO: guard with cfloat trait
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, Scalar value) {
	using namespace sw::universal;

	auto defaultPrecision = ostr.precision(); // save stream state

	constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10; 	// floating-point attribute for printing scientific format

	std::cout << '\n' << type_tag(Scalar()) << '\n';
	Scalar v(value); // convert to target fixpnt
	ostr << "fixed  form  : " << std::setprecision(max_digits10) << v << '\n';
	ostr << "triple form  : " << to_triple(v) << '\n';
	ostr << "binary form  : " << to_binary(v, true) << '\n';
	ostr << "color coded  : " << color_print(v) << '\n';

	ostr << std::setprecision(defaultPrecision);
}

void Show(std::ostream& ostr, double d) {
	using namespace sw::universal;

	ShowRepresentations(ostr, fixpnt< 8, 4>(d));
	ShowRepresentations(ostr, fixpnt<12, 4>(d));
	ShowRepresentations(ostr, fixpnt<16, 8>(d));
	ShowRepresentations(ostr, fixpnt<24, 8>(d));
	ShowRepresentations(ostr, fixpnt<32, 16>(d));
	ShowRepresentations(ostr, fixpnt<48, 16>(d));
	ShowRepresentations(ostr, fixpnt<64, 32>(d));
}

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "fixpnt : components of a fixed-point value\n";
		std::cerr << "Show the sign/scale/fraction components of a fixed-point value.\n";
		std::cerr << "Usage: fixpnt float_value\n";
		std::cerr << "Example: fixpnt 1.0625\n";
		Show(std::cerr, 1.0625);
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	double d = atof(argv[1]);

	std::cout << "fixpnt " << argv[1] << '\n';
	Show(std::cout, d);

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
