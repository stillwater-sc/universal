// half.cpp: components of a float: cli to show the sign/scale/fraction components of a half precision float 
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#include <universal/number/cfloat/cfloat.hpp>

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using Scalar = half;

	if (argc != 2) {
		std::cerr << "half : components of an IEEE-754 half-precision floating-point: 16 bits with 5 exponent bits\n";
		std::cerr << "Show the sign/scale/fraction components of a half-precision IEEE-754 floating-point.\n";
		std::cerr << "Usage: half value\n";
		std::cerr << "Example: half 0.03124999\n";
		ShowRepresentations<Scalar>(std::cerr, 0.03124999f);

		std::cerr << "\nNumber Traits of half-precision IEEE-754 floating-point\n";
		numberTraits<Scalar>(std::cout);

		std::cerr << "smallest normal number\n";
		std::cerr << to_binary(std::numeric_limits<Scalar>::min()) << " : " << std::numeric_limits<Scalar>::min() << '\n';
		std::cerr << "smallest denormalized number\n";
		std::cerr << to_binary(std::numeric_limits<Scalar>::denorm_min()) << " : " << std::numeric_limits<Scalar>::denorm_min() << '\n';

		std::cerr.flush();
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}

	float f = float(atof(argv[1]));
	ShowRepresentations<Scalar>(std::cout, f);

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
