// bfloat.cpp: components of a bfloat: cli to show the sign/scale/fraction components of a Google Brain float
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/common/number_traits_reports.hpp>


// ShowRepresentations prints the different output formats for the float type
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, Scalar f) {
	using namespace sw::universal;
	auto oldprec = ostr.precision(); // save stream state

	constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10; 	// floating-point attribute for printing scientific format

	Scalar v(f); // convert to target bfloat16
	ostr << "scientific   : " << std::setprecision(max_digits10) << v << '\n';
	ostr << "triple form  : " << to_triple(v) << '\n';
	ostr << "binary form  : " << to_binary(v, true) << '\n';
	ostr << "color coded  : " << color_print(v) << '\n';

	ostr << std::setprecision(oldprec);
}

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using Scalar = bfloat16;

	if (argc != 2) {
		std::cerr << "bfloat : components of a Google Brain floating-point: 16 bits with 8 exponent bits\n";
		std::cerr << "Show the sign/scale/fraction components of a Google Brain floating-point.\n";
		std::cerr << "Usage: bfloat value\n";
		std::cerr << "Example: bfloat 0.03124999\n";
		ShowRepresentations<Scalar>(std::cerr, 0.03124999f);

		std::cerr << "\nNumber Traits of bfloat\n";
		std::cerr << "min exponent       " << int(std::numeric_limits<Scalar>::min_exponent) << '\n';
		numberTraits<Scalar>(std::cout);

		std::cerr << "smallest normal number\n";
		std::cerr << to_binary(std::numeric_limits<Scalar>::min()) << " : " << std::numeric_limits<Scalar>::min() << '\n';
		std::cerr << "smallest denormalized number\n";
		std::cerr << to_binary(std::numeric_limits<Scalar>::denorm_min()) << " : " << std::numeric_limits<Scalar>::denorm_min() << '\n';

		std::cerr.flush();

		return EXIT_SUCCESS;  // signal successful completion for ctest
	}

	Scalar bf = Scalar(atof(argv[1]));
	ShowRepresentations<Scalar>(std::cout, bf);

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
