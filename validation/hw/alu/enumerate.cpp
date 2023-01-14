// enumerate.cpp: test of encoding enumeration
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/common/enumerate_encodings.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	constexpr unsigned nbits = 5;
	constexpr unsigned es = 2;
	using Real = cfloat<nbits, es, std::uint8_t, false, false, false>;

	EnumerateValidEncodingsViaIncrement<Real>(std::cout);

	EnumerateValidEncodingsViaDecrement<cfloat<9, 2, std::uint8_t, false, false, false>>(std::cout, 1.0, -1.0);

//	GenerateBinaryOpTestVectors<cfloat<5, 3, std::uint8_t, false, false, false> >(std::cout, "add");
//	GenerateBinaryOpTestVectors<cfloat<5, 3, std::uint8_t, true, true, false> >(std::cout, "add");

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
